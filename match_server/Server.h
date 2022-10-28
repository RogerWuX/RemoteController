#pragma once
#include <functional>
#include <memory>
#include <system_error>
#include <vector>
#include <map>
#include <random>
#include <asio.hpp>
#include <asio/ssl.hpp>

#include "Session.h"

class IdAllocator
{
public:
    IdAllocator();
    std::string AllocateId();
    void ReleaseId(const std::string &_kId);
protected:
    unsigned short AllocateSerialNumber();
    void ReleaseSerialNumber(unsigned short _nNumber);
    std::default_random_engine m_kRandomEngine;
    std::uniform_int_distribution<> m_kRandomDistribution;
    std::vector<char> m_kRandomCharVec;
    std::vector<bool> m_kSerialNumAllocationVec;

};
class Server{

public:
    Server(asio::io_context &_kIoContext,const asio::ip::address& _kListenAddress, unsigned int _nPort);
    ~Server();
    void SetCertInfo(const std::string &_kCertFilePath, const std::string &_kKeyFilePath);
    void StartAccept();
    void OnAcceptDone(const std::error_code &_kErrorCode, asio::ip::tcp::socket _kSocket);
    void OnSessionReady(Session& _kSession);
    void OnClientMsg(Session& _kSession, unsigned short _nMsgType, const char *_pnData, unsigned short _nSize);
    void OnSessionClose(Session& _kSession);
protected:
    void InitSession(asio::ip::tcp::socket _kSocket);
    void OnClientRequestConnection(Session& _kSession, const char *_pnData, unsigned short size);
    std::shared_ptr<Session> Match(const std::shared_ptr<Session>& _pkInitSession, const std::string &_kTargetSessionId);
    void TransferMsgToPair(Session& _kSession, unsigned short _nMsgType, const char *_pkData, unsigned short _nSize);
protected:
    asio::ssl::context      m_kSslContext;
    asio::ip::tcp::acceptor m_acceptor;
    std::map<std::string, std::shared_ptr<Session>> m_kIdToSessionMap;
    std::map<std::string, std::weak_ptr<Session>> m_kIdToPairSessionMap;
    IdAllocator m_kSessionIdAllocator;
};
