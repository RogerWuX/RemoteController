#pragma once
#include "RtcClient.h"

#include <QObject>


class ControllerClient;
class RtcVideoSink : public rtc::VideoSinkInterface<webrtc::VideoFrame>
{
public:
    RtcVideoSink(ControllerClient &_kClient);
    void OnFrame(const webrtc::VideoFrame& _kFrame) override;
    void OnDiscardedFrame() override;
protected:
    ControllerClient &m_kClient;
};

class ControllerClient : public RtcClient
{
    Q_OBJECT
public:
    ControllerClient(QObject *parent=nullptr);
    ~ControllerClient();
signals:
    void FrameReceived(QImage _kImg);
public slots:
    void OnRtcOnAddTrack(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> _pkTrack);
    void OnRtcOnRemoveTrack(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> _pkTrack);
    void OnRtcOnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> _pkDataChannel);
    void OnRtcFrameReceived(QImage _kArgbImg);
protected:
    RtcVideoSink                      m_kVideoSink;
};
