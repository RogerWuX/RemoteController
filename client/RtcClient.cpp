#include "RtcClient.h"

#include <QPixmap>
#include <QImage>

#include "api/create_peerconnection_factory.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"

#include "Logger.h"

CreateSessionDescriptionObserver::CreateSessionDescriptionObserver(RtcClient &_kRtcClient)
    :m_kRtcClient(_kRtcClient)
{
}

void CreateSessionDescriptionObserver::OnSuccess(webrtc::SessionDescriptionInterface *desc)
{
    std::string kStrSdpType = webrtc::SdpTypeToString(desc->GetType());
    std::string kStrSdp;
    desc->ToString(&kStrSdp);

    emit m_kRtcClient.RtcLocalSdpCreated(kStrSdpType,kStrSdp);
}

void CreateSessionDescriptionObserver::OnFailure(webrtc::RTCError _kError)
{
    RTC_LOGGER->warn("Create Session Failed! Error message:{}", _kError.message());
}

SetLocalSessionDescriptionObserver::SetLocalSessionDescriptionObserver(RtcClient &_kRtcClient)
    :m_kRtcClient(_kRtcClient)
{
}

void SetLocalSessionDescriptionObserver::OnSuccess()
{
    emit m_kRtcClient.RtcSetLocalSessionSuccess();
}

void SetLocalSessionDescriptionObserver::OnFailure(webrtc::RTCError _kError)
{
    RTC_LOGGER->warn("Set local session failed! Error message:{}", _kError.message());
    emit m_kRtcClient.RtcSetLocalSessionFailed();
}

SetRemoteSessionDescriptionObserver::SetRemoteSessionDescriptionObserver(RtcClient &_kRtcClient)
    :m_kRtcClient(_kRtcClient)
{
}

void SetRemoteSessionDescriptionObserver::OnSuccess()
{
    emit m_kRtcClient.RtcSetRemoteSessionSuccess();
}

void SetRemoteSessionDescriptionObserver::OnFailure(webrtc::RTCError _kError)
{
    RTC_LOGGER->warn("Set remote session failed! Error message:{}", _kError.message());
    emit m_kRtcClient.RtcSetRemoteSessionFailed();
}

PeerConnectionObserver::PeerConnectionObserver(RtcClient &_kRtcClient)
    :m_kRtcClient(_kRtcClient)
{
}

void PeerConnectionObserver::OnConnectionChange(webrtc::PeerConnectionInterface::PeerConnectionState new_state)
{
    emit m_kRtcClient.RtcPeerConnectionStateChange(new_state);
}

void PeerConnectionObserver::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state)
{

}

void PeerConnectionObserver::OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
                                        const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface> > &streams)
{
    emit m_kRtcClient.RtcOnAddTrack(receiver->track());
}

void PeerConnectionObserver::OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver)
{
    emit m_kRtcClient.RtcOnRemoveTrack(receiver->track());
}

void PeerConnectionObserver::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel)
{
    emit m_kRtcClient.RtcOnDataChannel(channel);
}

void PeerConnectionObserver::OnRenegotiationNeeded()
{

}

void PeerConnectionObserver::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state)
{

}

void PeerConnectionObserver::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state)
{

}

void PeerConnectionObserver::OnIceCandidate(const webrtc::IceCandidateInterface *_pkCandidate)
{
    std::string kSdpMidName = _pkCandidate->sdp_mid();
    int nSdpMlineIndex = _pkCandidate->sdp_mline_index();
    std::string kSdp;
    _pkCandidate->ToString(&kSdp);
    emit m_kRtcClient.RtcIceCandidateReceived(kSdpMidName, nSdpMlineIndex, kSdp);
}

void PeerConnectionObserver::OnIceConnectionReceivingChange(bool receiving)
{

}

DataChannelObserver::DataChannelObserver(RtcClient &_kRtcClient)
    :m_kRtcClient(_kRtcClient)
{
}

void DataChannelObserver::OnStateChange()
{
    emit m_kRtcClient.RtcDataChannelStateChange();
}

void DataChannelObserver::OnMessage(const webrtc::DataBuffer &buffer)
{
    emit m_kRtcClient.RtcDataChannelOnMessage(buffer.data);
}

RtcClient::RtcClient(QObject *parent)
    :QObject(parent),
     m_kPeerConnectionObserver(*this),
     m_kDataChannelObserver(*this)
{
    m_pkSetLocalSessionDescriptionObserver = rtc::make_ref_counted<SetLocalSessionDescriptionObserver>(*this);
    m_pkSetRemoteSessionDescriptionObserver = rtc::make_ref_counted<SetRemoteSessionDescriptionObserver>(*this);
    m_pkCreateSessionDescriptionObserver = rtc::make_ref_counted<CreateSessionDescriptionObserver>(*this);

    //Pass data from webrtc signalling thread
    QObject::connect(this, &RtcClient::RtcPeerConnectionStateChange, this, &RtcClient::OnRtcPeerConnectionStateChange, Qt::QueuedConnection);
    QObject::connect(this, &RtcClient::RtcSetLocalSessionSuccess,    this, &RtcClient::OnRtcSetLocalSessionSuccess,    Qt::QueuedConnection);
    QObject::connect(this, &RtcClient::RtcSetRemoteSessionSuccess,   this, &RtcClient::OnRtcSetRemoteSessionSuccess,   Qt::QueuedConnection);
    QObject::connect(this, &RtcClient::RtcSetLocalSessionFailed,     this, &RtcClient::OnRtcSetLocalSessionFailed,     Qt::QueuedConnection);
    QObject::connect(this, &RtcClient::RtcSetRemoteSessionFailed,    this, &RtcClient::OnRtcSetRemoteSessionFailed,    Qt::QueuedConnection);
    QObject::connect(this, &RtcClient::RtcLocalSdpCreated,           this, &RtcClient::OnRtcLocalSdpCreated,           Qt::QueuedConnection);
    QObject::connect(this, &RtcClient::RtcIceCandidateReceived,      this, &RtcClient::OnRtcIceCandidateReceived,      Qt::QueuedConnection);
    QObject::connect(this, &RtcClient::RtcOnAddTrack,                this, &RtcClient::OnRtcOnAddTrack,                Qt::QueuedConnection);
    QObject::connect(this, &RtcClient::RtcOnRemoveTrack,             this, &RtcClient::OnRtcOnRemoveTrack,             Qt::QueuedConnection);
    QObject::connect(this, &RtcClient::RtcOnDataChannel,             this, &RtcClient::OnRtcOnDataChannel,             Qt::QueuedConnection);    
    QObject::connect(this, &RtcClient::RtcDataChannelStateChange,    this, &RtcClient::OnRtcDataChannelStateChange,    Qt::QueuedConnection);
    QObject::connect(this, &RtcClient::RtcDataChannelOnMessage,      this, &RtcClient::OnRtcDataChannelOnMessage,      Qt::QueuedConnection);

}

RtcClient::~RtcClient()
{
}

bool RtcClient::InitPeerConncetion(const std::string &_kStunServerAddress)
{
    assert(!m_pkSignalingThread);
    assert(!m_pkPeerConnectionFactory);
    assert(!m_pkPeerConnection);

    m_pkSignalingThread = rtc::Thread::Create();
    m_pkSignalingThread->Start();

    m_pkPeerConnectionFactory =
        webrtc::CreatePeerConnectionFactory(nullptr,
                                            nullptr,
                                            m_pkSignalingThread.get(),
                                            nullptr,
                                            webrtc::CreateBuiltinAudioEncoderFactory(),
                                            webrtc::CreateBuiltinAudioDecoderFactory(),
                                            webrtc::CreateBuiltinVideoEncoderFactory(),
                                            webrtc::CreateBuiltinVideoDecoderFactory(),
                                            nullptr,
                                            nullptr );
    if(!m_pkPeerConnectionFactory) {
        RTC_LOGGER->warn("Initialize peer connection factory failed!.");
        return false;
    }

    webrtc::PeerConnectionInterface::IceServer kIceServer;
    kIceServer.uri = _kStunServerAddress;

    webrtc::PeerConnectionInterface::RTCConfiguration kRtcConfig;
    kRtcConfig.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
    kRtcConfig.servers.push_back(kIceServer);
    webrtc::PeerConnectionDependencies kPeerConnDep(&m_kPeerConnectionObserver);

    auto kErrorOrPeerConnection =
         m_pkPeerConnectionFactory->CreatePeerConnectionOrError(kRtcConfig, std::move(kPeerConnDep));
    if (!(kErrorOrPeerConnection.ok())) {
        RTC_LOGGER->warn("Initialize peer connection failed!. Message:{}", kErrorOrPeerConnection.error().message());
        return false;
    }
    m_pkPeerConnection = std::move(kErrorOrPeerConnection.value());

    return true;
}

void RtcClient::OnRemoteSdpReceived(const std::string &_kStrSdpType, const std::string &_kStrSdp)
{
    if(!SetRemoteSdp(_kStrSdpType, _kStrSdp)){
        RTC_LOGGER->warn("Set remote sdp failed. Type: {} Sdp:{}", _kStrSdpType.c_str(), _kStrSdp.c_str());
        emit ErrorHappen();
        return;
    }

    for(auto &pDscp :m_kIceCandidateBuffer){
        if(!m_pkPeerConnection->AddIceCandidate(pDscp.get())){
            RTC_LOGGER->warn("Add ice candidate failed.");
            emit ErrorHappen();
            return;
        }
    }
    m_kIceCandidateBuffer.clear();
}

bool RtcClient::SetRemoteSdp(const std::string &_kStrSdpType, const std::string &_kStrSdp)
{
    absl::optional<webrtc::SdpType> kTypeOptional = webrtc::SdpTypeFromString(_kStrSdpType);
    if(!kTypeOptional){
        RTC_LOGGER->warn("Invalid sdp type. Type:{}", _kStrSdpType);
        return false;
    }
    webrtc::SdpType kSdpType = *kTypeOptional;
    webrtc::SdpParseError kSdpParseError;
    std::unique_ptr<webrtc::SessionDescriptionInterface> pkSessionDescription =
        webrtc::CreateSessionDescription(kSdpType, _kStrSdp, &kSdpParseError);
    if(!pkSessionDescription){
        RTC_LOGGER->warn("Create remote description failed. Line:{} Description:{}",kSdpParseError.line, kSdpParseError.description);
        return false;
    }

    m_pkPeerConnection->SetRemoteDescription(m_pkSetRemoteSessionDescriptionObserver.get(), pkSessionDescription.release());

    if(kSdpType == webrtc::SdpType::kOffer){
        m_pkPeerConnection->SetLocalDescription(m_pkSetLocalSessionDescriptionObserver.get());
    }

    return true;
}

void RtcClient::OnDataChannelMessage(unsigned short _nMsgType, const std::string &_kData)
{
}

void RtcClient::SendDataChannelMsg(unsigned short _nMsgType, std::string _kData)
{
    char nMsgTypeByte[2];
    nMsgTypeByte[0] = _nMsgType;
    nMsgTypeByte[1] = _nMsgType>>8;
    const webrtc::DataBuffer pkDataBuffer(std::string(nMsgTypeByte, 2) + _kData);
    if(!m_pkDataChannel->Send(pkDataBuffer)) {
        RTC_LOGGER->warn("Send data channel message failed! Type:{}", _nMsgType);
    }
}

void RtcClient::SendDataChannelMsg(unsigned short _nMsgType, google::protobuf::Message *_kData)
{
    size_t nMessageSize = _kData->ByteSizeLong();
    rtc::CopyOnWriteBuffer kBuffer(nMessageSize + 2);
    uint8_t *pBuffer = kBuffer.MutableData();
    *pBuffer = (uint8_t)_nMsgType;
    *(++pBuffer) = (uint8_t)(_nMsgType>>8);
    _kData->SerializeToArray(++pBuffer, nMessageSize);

    const webrtc::DataBuffer pkDataBuffer(kBuffer, true);
    if(!m_pkDataChannel->Send(pkDataBuffer)) {
        RTC_LOGGER->warn("Send data channel message failed! Type:{}", _nMsgType);
    }
}

void RtcClient::OnRtcPeerConnectionStateChange(webrtc::PeerConnectionInterface::PeerConnectionState _nState)
{
    switch(_nState)
    {
    case webrtc::PeerConnectionInterface::PeerConnectionState::kConnected:
    {
        emit PeerConnected();
        break;
    }
    case webrtc::PeerConnectionInterface::PeerConnectionState::kDisconnected:
    {
        emit PeerDisconnected();
        break;
    }
    case webrtc::PeerConnectionInterface::PeerConnectionState::kFailed:
    {
        emit ErrorHappen();
        break;
    }
    default:
        break;
    }
}

void RtcClient::OnRtcSetLocalSessionSuccess()
{
    const webrtc::SessionDescriptionInterface *pLocalDesc = m_pkPeerConnection->local_description();
    std::string kStrSdpType = webrtc::SdpTypeToString(pLocalDesc->GetType());
    std::string kStrSdp;
    pLocalDesc->ToString(&kStrSdp);

    RTC_LOGGER->info("Set local session success. Type:{}, Sdp:{}", kStrSdpType.c_str(), kStrSdp.c_str());

    protocol::signalling::SessionDescription kMsg;
    kMsg.set_sdp_type(kStrSdpType);
    kMsg.set_sdp(kStrSdp);
    emit SendSignalMsg(protocol::signalling::eMT_ExchangeSessionDescription, &kMsg);
}

void RtcClient::OnRtcSetLocalSessionFailed()
{
    emit ErrorHappen();
}

void RtcClient::OnRtcSetRemoteSessionSuccess()
{
    const webrtc::SessionDescriptionInterface *pRemoteDesc = m_pkPeerConnection->remote_description();
    std::string kStrSdpType = webrtc::SdpTypeToString(pRemoteDesc->GetType());
    std::string kStrSdp;
    pRemoteDesc->ToString(&kStrSdp);

    RTC_LOGGER->info("Set remote session success. Type:{}, Sdp:{}", kStrSdpType.c_str(), kStrSdp.c_str());
}

void RtcClient::OnRtcSetRemoteSessionFailed()
{
    emit ErrorHappen();
}

void RtcClient::OnRtcLocalSdpCreated(std::string _kStrSdpType, std::string _kStrSdp)
{    
}

void RtcClient::OnRtcIceCandidateReceived(std::string _kSdpMid, qint32 _nSdpMlineIndex, std::string _kSdp)
{
    RTC_LOGGER->info("Receive ice candidate. SdpMid:{} SdpMlineIndex:{} Sdp:{}", _kSdpMid.c_str(), _nSdpMlineIndex, _kSdp.c_str());

    protocol::signalling::IceCandidate kMsg;
    kMsg.set_sdp_mid_name(_kSdpMid);
    kMsg.set_sdp_mline_index(_nSdpMlineIndex);
    kMsg.set_sdp(_kSdp);
    emit SendSignalMsg(protocol::signalling::eMT_ExchangeIceCandidate, &kMsg);
}

void RtcClient::OnRtcOnAddTrack(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> _pkTrack)
{
}

void RtcClient::OnRtcOnRemoveTrack(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> _pkTrack)
{
}

void RtcClient::OnRtcOnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> _pkDataChannel)
{   
}

void RtcClient::OnRtcDataChannelStateChange()
{
    if(m_pkDataChannel->state() == webrtc::DataChannelInterface::DataState::kOpen) {
        RTC_LOGGER->info("Data channel open.");
    }
}

/*
 *| Message Type |         Data        |
 *       2            MessageSize - 2
 */
void RtcClient::OnRtcDataChannelOnMessage(rtc::CopyOnWriteBuffer _kBuffer)
{
    if(_kBuffer.size() < 2)
        return;

    const uint8_t *data = _kBuffer.data();
    //little endian
    unsigned short nMsgType = *data + (*(data + 1) << 8);
    std::string kData((const char*)(data + 2), _kBuffer.size() - 2);
    OnDataChannelMessage(nMsgType, kData);
}

void RtcClient::OnRemoteIceCandidateReceived(const std::string &_kSdpMid, int32_t _nSdpMlineIndex, const std::string &_kSdp)
{
    webrtc::SdpParseError kParseError;
    std::unique_ptr<webrtc::IceCandidateInterface> kCandidate (
        webrtc::CreateIceCandidate(_kSdpMid, _nSdpMlineIndex, _kSdp, &kParseError));
    if(!kCandidate){
        RTC_LOGGER->warn("Create ice candidate failed. Line:{} Description:{}", kParseError.line, kParseError.description);
        emit ErrorHappen();
        return;
    }

    // Remote ice candidate may arrive before remote description set
    if(m_pkPeerConnection->remote_description()){
        if(!m_pkPeerConnection->AddIceCandidate(kCandidate.get())){
            RTC_LOGGER->warn("Add ice candidate failed.");
            emit ErrorHappen();
            return;
        }
    }
    else{
        m_kIceCandidateBuffer.push_back(std::move(kCandidate));
    }

    return;
}
