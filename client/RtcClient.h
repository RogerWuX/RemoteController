#pragma once
#include <QObject>

#undef emit
#include "api/peer_connection_interface.h"
#define emit

class RtcClient;

class CreateSessionDescriptionObserver: public webrtc::CreateSessionDescriptionObserver
{
 public:
    CreateSessionDescriptionObserver(RtcClient &_kRtcClient);
    void OnSuccess(webrtc::SessionDescriptionInterface* desc) override;
    void OnFailure(webrtc::RTCError _kError) override;
protected:
    RtcClient &m_kRtcClient;
};

class SetLocalSessionDescriptionObserver : public webrtc::SetSessionDescriptionObserver
{
public:
    SetLocalSessionDescriptionObserver(RtcClient &_kRtcClient);
    void OnSuccess();
    void OnFailure(webrtc::RTCError _kError);
protected:
    RtcClient &m_kRtcClient;
};

class SetRemoteSessionDescriptionObserver : public webrtc::SetSessionDescriptionObserver
{
public:
    SetRemoteSessionDescriptionObserver(RtcClient &_kRtcClient);
    void OnSuccess();
    void OnFailure(webrtc::RTCError _kError);
protected:
    RtcClient &m_kRtcClient;
};
class PeerConnectionObserver : public webrtc::PeerConnectionObserver
{
 public:
    PeerConnectionObserver(RtcClient &_kRtcClient);
    void OnConnectionChange(
          webrtc::PeerConnectionInterface::PeerConnectionState new_state) override;
    void OnSignalingChange(
          webrtc::PeerConnectionInterface::SignalingState new_state) override;
    void OnAddTrack(
      rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
      const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>&
            streams) override;
    void OnRemoveTrack(
      rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override;
    void OnDataChannel(
      rtc::scoped_refptr<webrtc::DataChannelInterface> channel) override;
    void OnRenegotiationNeeded() override;
    void OnIceConnectionChange(
      webrtc::PeerConnectionInterface::IceConnectionState new_state) override;
    void OnIceGatheringChange(
      webrtc::PeerConnectionInterface::IceGatheringState new_state) override;
    void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;
    void OnIceConnectionReceivingChange(bool receiving) override;

protected:
    RtcClient &m_kRtcClient;
};

class DataChannelObserver : public webrtc::DataChannelObserver
{
public:
    DataChannelObserver(RtcClient &_kRtcClient);
    void OnStateChange();
    void OnMessage(const webrtc::DataBuffer& buffer);
protected:
    RtcClient &m_kRtcClient;
};

class RtcClient : public QObject
{
    Q_OBJECT
public:
    RtcClient(QObject *parent=nullptr);
    ~RtcClient();    
    bool InitPeerConncetion(const std::string &_kStunServerAddress);
signals:
    //observer pass
    void RtcPeerConnectionStateChange(webrtc::PeerConnectionInterface::PeerConnectionState _nState);
    void RtcSetLocalSessionSuccess();
    void RtcSetLocalSessionFailed();
    void RtcSetRemoteSessionSuccess();
    void RtcSetRemoteSessionFailed();
    void RtcLocalSdpCreated(std::string _kStrSdpType, std::string _kStrSdp);
    void RtcIceCandidateReceived(std::string _kSdpMid, qint32 _nSdpMlineIndex, std::string _kSdp);
    void RtcOnAddTrack(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> _pkTrack);
    void RtcOnRemoveTrack(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> _pkTrack);
    void RtcOnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> _pkDataChannel);
    void RtcDataChannelStateChange();
    void RtcDataChannelOnMessage(rtc::CopyOnWriteBuffer _kBuffer);
    void RtcFrameReceived(QImage _kArgbImg);    

    void SendSignalMsg(unsigned short _nMsgType, google::protobuf::Message *_kData);
    void ErrorHappen();
    void PeerConnected();
    void PeerDisconnected();
protected slots:
    void OnRtcPeerConnectionStateChange(webrtc::PeerConnectionInterface::PeerConnectionState _nState);
    void OnRtcSetLocalSessionSuccess();
    void OnRtcSetLocalSessionFailed();
    void OnRtcSetRemoteSessionSuccess();
    void OnRtcSetRemoteSessionFailed();
    void OnRtcLocalSdpCreated(std::string _kStrSdpType, std::string _kStrSdp);
    void OnRtcIceCandidateReceived(std::string _kSdpMid, qint32 _nSdpMlineIndex, std::string _kSdp);    
    void OnRtcDataChannelStateChange();
    void OnRtcDataChannelOnMessage(rtc::CopyOnWriteBuffer _kBuffer);
    virtual void OnRtcOnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> _pkDataChannel);
    virtual void OnRtcOnAddTrack(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> _pkTrack);
    virtual void OnRtcOnRemoveTrack(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> _pkTrack);
public slots:
    void OnRemoteSdpReceived(const std::string &_kStrSdpType, const std::string &_kStrSdp);
    void OnRemoteIceCandidateReceived(const std::string &_kSdpMid, int32_t _nSdpMlineIndex, const std::string &_kSdp);    
    void SendDataChannelMsg(unsigned short _nMsgType, std::string _kData);
    void SendDataChannelMsg(unsigned short _nMsgType, google::protobuf::Message *_kData);
protected:
    bool SetRemoteSdp(const std::string &_kStrSdpType, const std::string &_kStrSdp);
    virtual void OnDataChannelMessage(unsigned short _nMsgType, const std::string &_kData);
    std::unique_ptr<rtc::Thread>                                m_pkSignalingThread;
    rtc::scoped_refptr<webrtc::PeerConnectionInterface>         m_pkPeerConnection;
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>  m_pkPeerConnectionFactory;
    rtc::scoped_refptr<webrtc::DataChannelInterface>            m_pkDataChannel;
    //Observer
    DataChannelObserver                                     m_kDataChannelObserver;
    PeerConnectionObserver                                  m_kPeerConnectionObserver;    
    rtc::scoped_refptr<SetLocalSessionDescriptionObserver>  m_pkSetLocalSessionDescriptionObserver;
    rtc::scoped_refptr<SetRemoteSessionDescriptionObserver> m_pkSetRemoteSessionDescriptionObserver;
    rtc::scoped_refptr<CreateSessionDescriptionObserver>    m_pkCreateSessionDescriptionObserver;

    std::vector<std::unique_ptr<webrtc::IceCandidateInterface> > m_kIceCandidateBuffer;
};

