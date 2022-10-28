#pragma once
#include <asio.hpp>
#include <asio/ssl.hpp>
#include <system_error>

#define READ_BUFFER_SIZE 4096
class Server;
class Session : public std::enable_shared_from_this<Session>
{
public:
    Session(const std::string &_kId, asio::ip::tcp::socket _kSocket, asio::ssl::context &_kSslContext, Server &_kServer);
    ~Session();
    inline asio::ssl::stream<asio::ip::tcp::socket>::lowest_layer_type& GetSocket(){ return m_kSslSteam.lowest_layer(); }
    inline const std::string &GetId(){ return m_kId; }
    void Init();
    void StartRead();
    void OnHandshakeDone(std::error_code ec);
    void OnHeaderReadDone(std::error_code ec, size_t bytes);
    void OnBodyReadDone(std::error_code ec, size_t bytes);
    void SendMessage(unsigned short _nMsgType, const char *_pkData, unsigned short _nSize);
    void OnWrite(const std::error_code &err_code);
protected:
    void OnHandshakeError(std::error_code &ec);
    void OnReadError(std::error_code &ec);
    void OnWriteError(std::error_code &ec, unsigned short _nMsgType);
    const std::string m_kId;
    asio::ssl::stream<asio::ip::tcp::socket> m_kSslSteam;
    asio::ip::address m_kRemoteAddress;
    asio::ip::port_type m_nRemotePort;
    Server& m_kServer;
    char m_anReadBuffer[READ_BUFFER_SIZE];

};
