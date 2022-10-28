#include "ControllerClient.h"

#include "third_party/libyuv/include/libyuv.h"
#include "api/video/i420_buffer.h"

#include <QImage>

#include "Logger.h"

RtcVideoSink::RtcVideoSink(ControllerClient &_kClient)
    :m_kClient(_kClient)
{
}

void RtcVideoSink::OnFrame(const webrtc::VideoFrame &_kFrame)
{
    rtc::scoped_refptr<webrtc::I420BufferInterface> pkBuffer(_kFrame.video_frame_buffer()->ToI420());

    QImage kRgbImg(pkBuffer->width(), pkBuffer->height(), QImage::Format::Format_RGB888);
    libyuv::I420ToRGB24(pkBuffer->DataY(),pkBuffer->StrideY(),
                       pkBuffer->DataU(),pkBuffer->StrideU(),
                       pkBuffer->DataV(),pkBuffer->StrideV(),
                       kRgbImg.bits(),3 * pkBuffer->width(),
                       pkBuffer->width(),pkBuffer->height());
    //shallow copy
    emit m_kClient.RtcFrameReceived(kRgbImg);
}

void RtcVideoSink::OnDiscardedFrame()
{
}

ControllerClient::ControllerClient(QObject *parent)
    :RtcClient(parent),
     m_kVideoSink(*this)
{
    QObject::connect(this, &ControllerClient::RtcFrameReceived, this, &ControllerClient::OnRtcFrameReceived, Qt::QueuedConnection);
}

ControllerClient::~ControllerClient()
{
}

void ControllerClient::OnRtcOnAddTrack(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> _pkTrack)
{
    if(_pkTrack->kind() == webrtc::MediaStreamTrackInterface::kVideoKind){
        auto *pkVideoTrack = static_cast<webrtc::VideoTrackInterface*>(_pkTrack.get());
        pkVideoTrack->AddOrUpdateSink(&m_kVideoSink, rtc::VideoSinkWants());
        RTC_LOGGER->info("Add video track.");
    }
}

void ControllerClient::OnRtcOnRemoveTrack(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> _pkTrack)
{
    if(_pkTrack->kind() == webrtc::MediaStreamTrackInterface::kVideoKind)
        RTC_LOGGER->info("Remove video track.");
}

void ControllerClient::OnRtcOnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> _pkDataChannel)
{
    assert(!m_pkDataChannel);

    m_pkDataChannel = _pkDataChannel;
    m_pkDataChannel->RegisterObserver(&m_kDataChannelObserver);
}

void ControllerClient::OnRtcFrameReceived(QImage _kArgbImg)
{
    emit FrameReceived(_kArgbImg);
}
