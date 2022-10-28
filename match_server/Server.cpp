#include "Server.h"
#include <iostream>
#include "../protocol/signalling.pb.h"

#define SERIAL_NUM_MAX 100
#define ALPHABET_NUM 26
#define SESSION_ALPHABET_LENGTH 99

IdAllocator::IdAllocator()
    :m_kRandomEngine(std::random_device{}()),
     m_kRandomDistribution(0, 2*ALPHABET_NUM - 1),
     m_kSerialNumAllocationVec(SERIAL_NUM_MAX, false)
{
    for(int i = 0; i < ALPHABET_NUM; ++i){
        m_kRandomCharVec.push_back('A'+i);
        m_kRandomCharVec.push_back('a'+i);
    }
}

std::string IdAllocator::AllocateId()
{
    std::string kId;
    unsigned short nSerialNumber = AllocateSerialNumber();
    if(nSerialNumber == 0)
        return "";
    char nBuffer[4];
    snprintf(nBuffer, sizeof(nBuffer), "%03d", nSerialNumber);
    kId += nBuffer;
    for(int i = 0; i < SESSION_ALPHABET_LENGTH; ++i)
        kId += m_kRandomCharVec[m_kRandomDistribution(m_kRandomEngine)];
    return std::move(kId);
}

void IdAllocator::ReleaseId(const std::string &_kId)
{
    std::string kSerialNumStr = _kId.substr(0, 3);
    unsigned short nSerialNum = std::stoi(kSerialNumStr);
    ReleaseSerialNumber(nSerialNum);
}

unsigned short IdAllocator::AllocateSerialNumber()
{
    auto kSerialNumIt = std::find(m_kSerialNumAllocationVec.begin(), m_kSerialNumAllocationVec.end(), false);
    if(kSerialNumIt == m_kSerialNumAllocationVec.end())
        return 0;

    *kSerialNumIt = true;
    return kSerialNumIt - m_kSerialNumAllocationVec.begin() + 1;
}

void IdAllocator::ReleaseSerialNumber(unsigned short _nNumber)
{
    if(_nNumber >= m_kSerialNumAllocationVec.size() || _nNumber == 0)
        return;
    m_kSerialNumAllocationVec[_nNumber - 1] = false;
}

Server::Server(asio::io_context &_kIoContext,const asio::ip::address& _kListenAddress, unsigned int _nPort)
    :m_acceptor(_kIoContext, asio::ip::tcp::endpoint(_kListenAddress,_nPort)),
     m_kSslContext(asio::ssl::context::sslv23)
{
    m_kSslContext.set_verify_mode(asio::ssl::verify_none);

    asio::ip::tcp::endpoint kLocalEndpoint = m_acceptor.local_endpoint();
    std::cout<<"listen on "<<kLocalEndpoint.address().to_string()<<":"<<kLocalEndpoint.port()<<std::endl;
}

Server::~Server()
{
}

void Server::SetCertInfo(const std::string &_kCertFilePath, const std::string &_kKeyFilePath)
{
    m_kSslContext.use_certificate_file(_kCertFilePath, asio::ssl::context::pem);
    m_kSslContext.use_private_key_file(_kKeyFilePath, asio::ssl::context::pem);
}

void Server::StartAccept()
{
    m_acceptor.async_accept(std::bind(&Server::OnAcceptDone, this,
                                      std::placeholders::_1,std::placeholders::_2));

}

void Server::OnAcceptDone(const std::error_code &_kErrorCode, asio::ip::tcp::socket _kSocket)
{
    if(_kErrorCode) {
        std::cerr<<"error: "<<_kErrorCode<<std::endl;
    }
    else{
        InitSession(std::move(_kSocket));
    }

    if(m_kIdToSessionMap.size() < SERIAL_NUM_MAX){
        StartAccept();
    }
}

void Server::OnSessionReady(Session &_kSession)
{
    protocol::signalling::SessionId kMsg;
    kMsg.set_session_id(_kSession.GetId());
    std::string kData;
    kMsg.SerializeToString(&kData);
    _kSession.SendMessage(protocol::signalling::eMT_UpdateSessionId,kData.c_str(), kData.size());
}

void Server::OnClientMsg(Session& _kSession, unsigned short _nMsgType, const char *_pnData, unsigned short _nSize)
{
    switch(_nMsgType)
    {
    case protocol::signalling::eMT_RequestConnection:{
        OnClientRequestConnection(_kSession, _pnData, _nSize);
        break;
    }
    case protocol::signalling::eMT_ExchangeSessionDescription:
    case protocol::signalling::eMT_ExchangeIceCandidate:
    {
        TransferMsgToPair(_kSession, _nMsgType, _pnData, _nSize);
        break;
    }
    default:
        break;
    }

}

void Server::OnSessionClose(Session &_kSession)
{
    auto pIdToPairSessionIt = m_kIdToPairSessionMap.find(_kSession.GetId());
    if(pIdToPairSessionIt != m_kIdToPairSessionMap.end()) {
        auto pPairSessionSharedPtr = pIdToPairSessionIt->second.lock();
        if(pPairSessionSharedPtr){
            m_kIdToPairSessionMap.erase(pPairSessionSharedPtr->GetId());
        }
        m_kIdToPairSessionMap.erase(pIdToPairSessionIt);
    }
    m_kSessionIdAllocator.ReleaseId(_kSession.GetId());

    m_kIdToSessionMap.erase(_kSession.GetId());

    if(m_kIdToSessionMap.size() == SERIAL_NUM_MAX - 1){
        StartAccept();
    }
}

void Server::InitSession(asio::ip::tcp::socket _kSocket)
{
    std::string kSessionID = m_kSessionIdAllocator.AllocateId();
    try{
        auto pSessionSharedPtr = std::make_shared<Session>(kSessionID, std::move(_kSocket), m_kSslContext, *this);
        m_kIdToSessionMap.insert(std::make_pair(pSessionSharedPtr->GetId(),pSessionSharedPtr));
        pSessionSharedPtr->Init();
    }
    catch (asio::system_error &e){
        m_kSessionIdAllocator.ReleaseId(kSessionID);
    }
}

void Server::OnClientRequestConnection(Session& _kSession, const char *_pnData, unsigned short size)
{
    auto kIdToSessionIt = m_kIdToSessionMap.find(_kSession.GetId());
    if(kIdToSessionIt == m_kIdToSessionMap.end())
        return;

    std::string kSessionIdData(_pnData, (size_t)size);
    protocol::signalling::SessionId kSessionIdMsg;
    kSessionIdMsg.ParseFromString(kSessionIdData);

    protocol::signalling::RequestReply kRequestReplyMsg;
    auto pkTargetSession = Match(kIdToSessionIt->second, kSessionIdMsg.session_id());
    if(pkTargetSession){
        kRequestReplyMsg.set_request_success(true);
        pkTargetSession->SendMessage(protocol::signalling::eMT_RemoteRequestConnection, "", 0);
    }
    else{
        kRequestReplyMsg.set_request_success(false);
    }
    std::string kReplyData;
    kRequestReplyMsg.SerializeToString(&kReplyData);
    _kSession.SendMessage(protocol::signalling::eMT_ReplyConnection, kReplyData.c_str(), kReplyData.size());
}

std::shared_ptr<Session> Server::Match(const std::shared_ptr<Session>& _pkInitSession, const std::string &_kTargetSessionId)
{
    if(_pkInitSession->GetId() == _kTargetSessionId)
        return nullptr;

    auto kInitIdToPairSessionIt = m_kIdToPairSessionMap.find(_pkInitSession->GetId());
    if(kInitIdToPairSessionIt != m_kIdToPairSessionMap.end())
        return nullptr;

    auto kTargetIdToSessionIt = m_kIdToSessionMap.find(_kTargetSessionId);
    if(kTargetIdToSessionIt == m_kIdToSessionMap.end())
        return nullptr;

    auto kTargetIdToPairSessionIt = m_kIdToPairSessionMap.find(kTargetIdToSessionIt->second->GetId());
    if(kTargetIdToPairSessionIt != m_kIdToPairSessionMap.end())
        return nullptr;

    m_kIdToPairSessionMap.emplace(_pkInitSession->GetId(),kTargetIdToSessionIt->second);
    m_kIdToPairSessionMap.emplace(kTargetIdToSessionIt->second->GetId(), _pkInitSession);

    return kTargetIdToSessionIt->second;
}

void Server::TransferMsgToPair(Session &_kSession, unsigned short _nMsgType, const char *_pkData, unsigned short _nSize)
{
    auto pkPairSessionIt = m_kIdToPairSessionMap.find(_kSession.GetId());
    if(pkPairSessionIt == m_kIdToPairSessionMap.end())
       return;

    auto pPairSessionSharedPtr = pkPairSessionIt->second.lock();
    if(!pPairSessionSharedPtr)
        return;
    pPairSessionSharedPtr->SendMessage(_nMsgType, _pkData, _nSize);
}


