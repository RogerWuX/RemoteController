#include "MatchClient.h"

#include <QApplication>
#include <QHostAddress>
#include <QList>

#include "../protocol/signalling.pb.h"
#include "Logger.h"

MatchClient::MatchClient(QObject *parent)
    :QObject(parent),
     m_kSocket(this)
{
#if ENCRYPT_MATCH
    m_kSocket.setPeerVerifyMode(QSslSocket::VerifyNone);
    QObject::connect(&m_kSocket, &QSslSocket::encrypted, this, &MatchClient::OnReady);
    QObject::connect(&m_kSocket, QOverload<const QList<QSslError> &>::of(&QSslSocket::sslErrors), this, &MatchClient::OnSslErrors);
#else
    QObject::connect(&m_kSocket, &QSslSocket::connected, this, &Client::OnReady);
#endif
    QObject::connect(&m_kSocket, &QTcpSocket::disconnected, this, &MatchClient::OnDisConnected);
    QObject::connect(&m_kSocket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error), this, &MatchClient::OnConnectionError);
}

MatchClient::~MatchClient()
{
}

void MatchClient::ConnectToServer(const std::string &_server_address, unsigned short port)
{
#if ENCRYPT_MATCH
    m_kSocket.connectToHostEncrypted(_server_address.c_str(), port);
#else
    m_kSocket.connectToHost(_server_address.c_str(), port);
#endif
}

void MatchClient::DisconnectFromServer()
{
    m_kSocket.disconnectFromHost();
}

void MatchClient::OnReady()
{
    MAIN_LOGGER->info("Connect to signalling server {}:{}", m_kSocket.peerAddress().toString().toStdString().c_str(), m_kSocket.peerPort());
    QObject::connect(&m_kSocket,&QTcpSocket::readyRead, this, &MatchClient::OnReadyRead);
}

void MatchClient::OnDisConnected()
{
    MAIN_LOGGER->info("Disconnect from signalling server {}:{}", m_kSocket.peerAddress().toString().toStdString().c_str(), m_kSocket.peerPort());
}

void MatchClient::OnConnectionError(QAbstractSocket::SocketError socketError)
{
    MAIN_LOGGER->warn("Signalling connection error. code:{}", socketError);
    emit ErrorHappen();
}

void MatchClient::OnSslErrors(const QList<QSslError> &errors)
{
    for(const QSslError &error:errors){
        MAIN_LOGGER->warn("Signalling SSL error. Error:{}", error.errorString().toStdString().c_str());
    }
    emit ErrorHappen();
}


/*
 *| Message Size | Message Type |         Data        |
 *       2              2            MessageSize - 2
 */
void MatchClient::OnReadyRead()
{    
    static unsigned short msgSize = 0;
    m_kDataBuffer += m_kSocket.readAll();
    while(m_kDataBuffer.length() >= msgSize) {
        if(msgSize == 0){
            if(m_kDataBuffer.length() < 2)
                break;
            msgSize=static_cast<unsigned char>(m_kDataBuffer[0])+ (static_cast<unsigned char>(m_kDataBuffer[1])<<8);
            m_kDataBuffer = m_kDataBuffer.mid(2);
        }
        else{
            QByteArray msg = m_kDataBuffer.left(msgSize);            
            m_kDataBuffer = m_kDataBuffer.mid(msgSize);
            msgSize = 0;
            ResolveMsg(msg);
        }
    }
}

void MatchClient::SendMsg(unsigned short _nMsgType, const std::string &data)
{
    unsigned short nMsgSize = 2 + data.length();
    m_kSocket.write((const char*)&nMsgSize,2);
    m_kSocket.write((const char*)&_nMsgType,2);
    m_kSocket.write(data.c_str(),data.length());
}

void MatchClient::ResolveMsg(QByteArray &_kMsgData)
{
    //little endian
    unsigned short nType = _kMsgData[0] + (static_cast<unsigned short>(_kMsgData[1])<<8);
    std::string kData = _kMsgData.mid(2).toStdString();
    switch(nType)
    {
    case protocol::signalling::eMT_UpdateSessionId:{
        protocol::signalling::SessionId msg;
        msg.ParseFromString(kData);
        emit UpdateSessionId(msg.session_id());
        break;
    }
    case protocol::signalling::eMT_RemoteRequestConnection:{
        protocol::signalling::SessionId msg;
        msg.ParseFromString(kData);
        emit RemoteRequestConnection(msg.session_id());
        break;
    }
    case protocol::signalling::eMT_ReplyConnection:{
        protocol::signalling::RequestReply msg;
        msg.ParseFromString(kData);
        emit ReplyConnection(msg.request_success());
        break;
    }
    case protocol::signalling::eMT_ExchangeSessionDescription:{
        protocol::signalling::SessionDescription msg;
        msg.ParseFromString(kData);
        emit RemoteSdpReceived(msg.sdp_type(),msg.sdp());
        break;
    }
    case protocol::signalling::eMT_ExchangeIceCandidate:{
        protocol::signalling::IceCandidate msg;
        msg.ParseFromString(kData);
        emit RemoteIceCandidateReceived(msg.sdp_mid_name(),msg.sdp_mline_index(),msg.sdp());
        break;
    }
    }
}


