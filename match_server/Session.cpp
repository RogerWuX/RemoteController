#include "Session.h"
#include <iostream>
#include "Server.h"

Session::Session(const std::string &_kId, asio::ip::tcp::socket _kSocket, asio::ssl::context &_kSslContext, Server &_kServer)
    :m_kId(_kId),
     m_kSslSteam(std::move(_kSocket), _kSslContext),
     m_kServer(_kServer)
{
    asio::ip::tcp::endpoint kRemoteEndpoint = m_kSslSteam.lowest_layer().remote_endpoint();
    m_kRemoteAddress = kRemoteEndpoint.address();
    m_nRemotePort = kRemoteEndpoint.port();
    std::cout<<"connect from "<<m_kRemoteAddress.to_string()<<":"<<m_nRemotePort<<std::endl;
}

Session::~Session()
{
    std::cout<<"disconnect from "<<m_kRemoteAddress.to_string()<<":"<<m_nRemotePort<<std::endl;
}

void Session::Init()
{
    m_kSslSteam.async_handshake(asio::ssl::stream_base::server,
                                std::bind(&Session::OnHandshakeDone, shared_from_this(),
                                          std::placeholders::_1));
}

void Session::StartRead()
{
    asio::async_read(m_kSslSteam, asio::buffer(m_anReadBuffer, 2),
                     std::bind(&Session::OnHeaderReadDone, shared_from_this(),
                               std::placeholders::_1, std::placeholders::_2));

}

void Session::OnHandshakeDone(std::error_code ec)
{
    if(ec){
        OnHandshakeError(ec);
        return;
    }

    m_kServer.OnSessionReady(*this);
    StartRead();
}

/*
 *| Message Size | Message Type |         Data        |
 *       2              2            MessageSize - 2
 */
void Session::OnHeaderReadDone(std::error_code ec, size_t bytes)
{
    if(ec){
        OnReadError(ec);
        return;
    }

    //little endian
    unsigned short nBodySize = *(unsigned short*)m_anReadBuffer;
    if(nBodySize < 2){
        std::cerr<<"read error: "<<"Invalid body size."<<std::endl;
        m_kServer.OnSessionClose(*this);
        return;
    }

    asio::async_read(m_kSslSteam, asio::buffer(m_anReadBuffer,nBodySize),
                     std::bind(&Session::OnBodyReadDone, shared_from_this(),
                               std::placeholders::_1, std::placeholders::_2));

}
void Session::OnBodyReadDone(std::error_code ec, size_t bytes)
{
    if(ec){
        OnReadError(ec);
        return;
    }
    //little endian
    unsigned short nMsgType = *(unsigned short*)m_anReadBuffer;

    m_kServer.OnClientMsg(*this, nMsgType, m_anReadBuffer + 2, bytes - 2);
    StartRead();
}

void Session::SendMessage(unsigned short _nMsgType, const char *_pkData, unsigned short _nSize)
{
    unsigned short nMsgLength = 2 + _nSize;
    std::vector<asio::const_buffer> kBufferVec;
    kBufferVec.push_back(asio::const_buffer(&nMsgLength, 2));
    kBufferVec.push_back(asio::const_buffer(&_nMsgType, 2));
    if(_nSize > 0)
        kBufferVec.push_back(asio::const_buffer(_pkData, _nSize));

    std::error_code ec;
    size_t nTransferBytes = asio::write(m_kSslSteam, kBufferVec, ec);
    if(ec){
        OnWriteError(ec, _nMsgType);
    }

}

void Session::OnHandshakeError(std::error_code &ec)
{
    std::cerr<<"handshake error: "<<ec<<" "<<ec.message()<<std::endl;
    if(ec == asio::error::eof ||
       ec == asio::error::connection_reset ||
       ec == asio::ssl::error::stream_truncated){
       m_kServer.OnSessionClose(*this);
    }
}

void Session::OnReadError(std::error_code &ec)
{
    std::cerr<<"read error: "<<ec.message()<<std::endl;
    if(ec == asio::error::eof ||
       ec == asio::error::connection_reset ||
       ec == asio::ssl::error::stream_truncated){
       m_kServer.OnSessionClose(*this);
    }
}

void Session::OnWriteError(std::error_code &ec, unsigned short _nMsgType)
{
    std::cerr<<"message type <<"<<_nMsgType<<" write error: "<<ec.message()<<std::endl;
    if(ec == asio::error::eof ||
       ec == asio::error::connection_reset ||
       ec == asio::ssl::error::stream_truncated){
       m_kServer.OnSessionClose(*this);
    }
}
