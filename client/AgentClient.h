#pragma once
#include "media/base/adapted_video_track_source.h"
#include "RtcClient.h"

class AgentClient;
class VideoTrackSource : public rtc::AdaptedVideoTrackSource
{
public:
    webrtc::MediaSourceInterface::SourceState state() const override{
        return kLive;
    }
    bool remote() const override{
        return false;
    }
    bool is_screencast() const override{
        return false;
    }
    absl::optional<bool> needs_denoising() const override{
        return false;
    }
    void SendFrame(QPixmap &_kPixmap);

};

class AgentClient : public RtcClient
{
    Q_OBJECT
public:
    AgentClient(QObject *parent=nullptr);
    ~AgentClient();
    void StartEstablishConnection(const std::string &_kStunServerName);
signals:
    //Datachannel
    void RemoteMouseMove(float _nStepX, float _nStepY);
    void RemoteMousePress(int _nButton);
    void RemoteMouseRelease(int _nButton);
    void RemoteMouseDoubleClick(int _nButton);
    void RemoteMouseWheel(int _nButton);
    void RemoteKeyboardPress(int _nKey);
    void RemoteKeyboardRelease(int _nKey);
protected:
    void OnDataChannelMessage(unsigned short _nMsgType, const std::string &_kData) override;
public slots:
    void SendFrame(QPixmap _kPixmap);
protected:
    bool InitDataChannel();
    bool InitVideoTrack();
    rtc::scoped_refptr<VideoTrackSource> m_pkVideoSource;

};
