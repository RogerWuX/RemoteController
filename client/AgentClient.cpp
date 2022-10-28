#include "AgentClient.h"

#include <cassert>

#include "third_party/libyuv/include/libyuv.h"
#include "api/video/i420_buffer.h"

#include <QPixmap>

#include "../protocol/signalling.pb.h"
#include "Logger.h"


void VideoTrackSource::SendFrame(QPixmap &_kPixmap)
{
    int64_t nMicrosecond = rtc::TimeMicros();
    int nOutWidth,nOutHeight;
    int nCropWidth,nCropHeight,nCropX,nCropY;

    if(!AdaptFrame(_kPixmap.width(),_kPixmap.height(),nMicrosecond,
                   &nOutWidth,&nOutHeight,
                   &nCropWidth,&nCropHeight,&nCropX,&nCropY)){
        return;
    }

    QImage kImage = _kPixmap.toImage();
    kImage = kImage.scaled(nOutWidth,nOutHeight);

    rtc::scoped_refptr<webrtc::I420Buffer> kI420Buffer =
        webrtc::I420Buffer::Create(kImage.width(), kImage.height());
    libyuv::ABGRToI420((const uint8_t*)kImage.constBits(),kImage.bytesPerLine(),
                       kI420Buffer->MutableDataY(),kI420Buffer->StrideY(),
                       kI420Buffer->MutableDataU(),kI420Buffer->StrideU(),
                       kI420Buffer->MutableDataV(),kI420Buffer->StrideV(),
                       kImage.width(),kImage.height());

    webrtc::VideoFrame kFrame = webrtc::VideoFrame::Builder()
                                .set_video_frame_buffer(kI420Buffer)
                                .set_timestamp_rtp(0)
                                .set_ntp_time_ms(0)
                                .set_timestamp_us(nMicrosecond)
                                .set_rotation(webrtc::kVideoRotation_0)
                                .build();
    OnFrame(kFrame);
}

AgentClient::AgentClient(QObject *parent)
    :RtcClient(parent)
{
    m_pkVideoSource = rtc::make_ref_counted<VideoTrackSource>();
}

AgentClient::~AgentClient()
{
}

void AgentClient::StartEstablishConnection(const std::string &_kStunServerName)
{
    if(!InitPeerConncetion(_kStunServerName) ||
       !InitDataChannel() ||
       !InitVideoTrack()){
        emit ErrorHappen();
        return;
    }

    m_pkPeerConnection->SetLocalDescription(m_pkSetLocalSessionDescriptionObserver.get());
}

bool AgentClient::InitVideoTrack()
{
    rtc::scoped_refptr<webrtc::VideoTrackInterface> pkVideoTrack=
        m_pkPeerConnectionFactory->CreateVideoTrack("VideoLabel", m_pkVideoSource.get());
    if(!pkVideoTrack)
        return false;

    auto kResultOrError = m_pkPeerConnection->AddTrack(pkVideoTrack, {"StreamId"});
    if (!kResultOrError.ok()){
        RTC_LOGGER->warn("Add video track failed. Message:{}", kResultOrError.error().message());
        return false;
    }

    return true;
}

bool AgentClient::InitDataChannel()
{
    webrtc::DataChannelInit kDataChannelConfig;
    kDataChannelConfig.ordered = true;

    auto kErrorOrDataChannel = m_pkPeerConnection->CreateDataChannelOrError("DataChannelLabel", &kDataChannelConfig);
    if(!kErrorOrDataChannel.ok()){
        RTC_LOGGER->warn("Create data channel failed. Message:{}", kErrorOrDataChannel.error().message());
        return false;
    }
    m_pkDataChannel = std::move(kErrorOrDataChannel.value());
    m_pkDataChannel->RegisterObserver(&m_kDataChannelObserver);

    return true;
}

void AgentClient::SendFrame(QPixmap _kPixmap)
{
    assert(m_pkVideoSource);

    m_pkVideoSource->SendFrame(_kPixmap);
    return;
}

void AgentClient::OnDataChannelMessage(unsigned short _nMsgType, const std::string &_kData)
{    
    switch(_nMsgType)
    {
    case protocol::p2p::eMT_MouseMove:
    {
        protocol::p2p::MouseMoveEvent kMsg;
        kMsg.ParseFromString(_kData);
        emit RemoteMouseMove(kMsg.step_x(), kMsg.step_y());
        break;
    }
    case protocol::p2p::eMT_MousePress:
    {
        protocol::p2p::MouseButtonStateEvent kMsg;
        kMsg.ParseFromString(_kData);
        emit RemoteMousePress(kMsg.button());
        break;
    }
    case protocol::p2p::eMT_MouseRelease:
    {
        protocol::p2p::MouseButtonStateEvent kMsg;
        kMsg.ParseFromString(_kData);
        emit RemoteMouseRelease(kMsg.button());
        break;
    }
    case protocol::p2p::eMT_MouseDoubleClick:
    {
        protocol::p2p::MouseButtonStateEvent kMsg;
        kMsg.ParseFromString(_kData);
        emit RemoteMouseDoubleClick(kMsg.button());
        break;
    }
    case protocol::p2p::eMT_MouseWheel:
    {
        protocol::p2p::MouseButtonStateEvent kMsg;
        kMsg.ParseFromString(_kData);
        emit RemoteMouseWheel(kMsg.button());
        break;
    }
    case protocol::p2p::eMT_KeyboardPress:
    {
        protocol::p2p::KeyboardEvent kMsg;
        kMsg.ParseFromString(_kData);
        emit RemoteKeyboardPress(kMsg.key());
        break;
    }
    case protocol::p2p::eMT_KeyboardRelease:
    {
        protocol::p2p::KeyboardEvent kMsg;
        kMsg.ParseFromString(_kData);
        emit RemoteKeyboardRelease(kMsg.key());
        break;
    }
    default:
        break;
    }

}
