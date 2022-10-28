#pragma once
#include "Application.h"

#include <QObject>
#include <QTimer>

#include "MatchClient.h"
#include "AgentClient.h"

class XControl;

class Agent : public IControllState
{
    Q_OBJECT
public:
    Agent(QObject *parent = nullptr);
    ~Agent();
    void Init(MatchClient *_pkSignallingClient, const std::string &_kStunServerAddress);
signals:
    void FrameCaptured(QPixmap _kImg);
public slots:
    void CaptureFrame();
    void OnRemoteMouseMove(float _nStepX, float _nStepY);
    void OnRemoteMousePress(int nButton);
    void OnRemoteMouseRelease(int nButton);
    void OnRemoteMouseDoubleClick(int nButton);
    void OnRemoteMouseWheel(int nButton);
    void OnRemoteKeyboardPress(int _nKey);
    void OnRemoteKeyboardRelease(int _nKey);
    void OnPeerConnected();
    void OnPeerDisconnected();
    void OnConnectionErrorHappen();
protected:
    XControl    *m_kControl;
    AgentClient *m_pkRtcClient;
    QTimer      *m_pkCaptureTimer;
};
