#pragma once
#include <QByteArray>
#include <QSslSocket>

class MatchClient :public QObject
{
    Q_OBJECT
public:
    MatchClient(QObject *parent=nullptr);
    virtual ~MatchClient();
    void OnReadyRead();    
    void ResolveMsg(QByteArray &_kMsgData);
    void ConnectToServer(const std::string &_server_address, unsigned short port);
    void DisconnectFromServer();
signals:    
    void UpdateSessionId(const std::string &_kSessionId);
    void RemoteRequestConnection(const std::string &_kSessionId);
    void ReplyConnection(bool _bSuccess);
    void RemoteSdpReceived(const std::string &_kSdpType, const std::string &_kSdp);
    void RemoteIceCandidateReceived(const std::string&, int32_t, const std::string&);
    void ErrorHappen();
public slots:
    void OnReady();
    void OnDisConnected();
    void OnConnectionError(QAbstractSocket::SocketError socketError);
    void OnSslErrors(const QList<QSslError> &errors);
    void SendMsg(unsigned short _nMsgType, google::protobuf::Message *_kMsg);
protected:
    QSslSocket m_kSocket;
    QByteArray m_kDataBuffer;
};

