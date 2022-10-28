#include "Agent.h"

#include <QApplication>
#include <QScreen>
#include <QtGui/5.12.8/QtGui/qpa/qplatformnativeinterface.h>

#include "Logger.h"
#include "XControl.h"

#define TARGET_FPS 30
const int TARGET_CAPTURE_INTERVAL_MSEC = 1000/TARGET_FPS;

Agent::Agent(QObject *parent)
    :IControllState(parent),
     m_pkCaptureTimer(nullptr)
{
    QPlatformNativeInterface *kPlatformNativeInterface = QApplication::platformNativeInterface();
    QScreen *pkPrimaryScreen = QApplication::primaryScreen();
    Display *pkDisplay = (Display*)kPlatformNativeInterface->nativeResourceForScreen("display", pkPrimaryScreen);
    m_kControl = new XControl(pkDisplay);

    m_pkRtcClient = new AgentClient();
}

Agent::~Agent()
{
    delete m_kControl;
}

void Agent::Init(MatchClient *_pkSignallingClient, const std::string &_kStunServerAddress)
{
    QObject::connect(m_pkRtcClient, &RtcClient::SendSignalMsg, _pkSignallingClient, &MatchClient::SendMsg);

    QObject::connect(_pkSignallingClient, &MatchClient::RemoteSdpReceived, m_pkRtcClient, &RtcClient::OnRemoteSdpReceived);
    QObject::connect(_pkSignallingClient, &MatchClient::RemoteIceCandidateReceived, m_pkRtcClient, &RtcClient::OnRemoteIceCandidateReceived);

    QObject::connect(m_pkRtcClient, &RtcClient::PeerConnected, this, &Agent::OnPeerConnected);
    QObject::connect(m_pkRtcClient, &RtcClient::PeerDisconnected, this, &Agent::OnPeerDisconnected);

    m_pkRtcClient->StartEstablishConnection(_kStunServerAddress);
}

void Agent::OnPeerConnected()
{
    m_pkCaptureTimer = new QTimer();
    QObject::connect(m_pkCaptureTimer,&QTimer::timeout,this,&Agent::CaptureFrame);
    m_pkCaptureTimer->start(TARGET_CAPTURE_INTERVAL_MSEC);

    QObject::connect(this, &Agent::FrameCaptured, m_pkRtcClient, &AgentClient::SendFrame);
    QObject::connect(m_pkRtcClient, &AgentClient::RemoteMouseMove,          this, &Agent::OnRemoteMouseMove);
    QObject::connect(m_pkRtcClient, &AgentClient::RemoteMousePress,         this, &Agent::OnRemoteMousePress);
    QObject::connect(m_pkRtcClient, &AgentClient::RemoteMouseRelease,       this, &Agent::OnRemoteMouseRelease);
    QObject::connect(m_pkRtcClient, &AgentClient::RemoteMouseDoubleClick,   this, &Agent::OnRemoteMouseDoubleClick);
    QObject::connect(m_pkRtcClient, &AgentClient::RemoteMouseWheel,         this, &Agent::OnRemoteMouseWheel);
    QObject::connect(m_pkRtcClient, &AgentClient::RemoteKeyboardPress,      this, &Agent::OnRemoteKeyboardPress);
    QObject::connect(m_pkRtcClient, &AgentClient::RemoteKeyboardRelease,    this, &Agent::OnRemoteKeyboardRelease);
    emit Ready();
}

void Agent::OnPeerDisconnected()
{
    emit Finish();
}

void Agent::OnConnectionErrorHappen()
{
    emit Error();
}

void Agent::CaptureFrame()
{
    QScreen *pkScreen = QGuiApplication::primaryScreen();
    if(!pkScreen)
        return;
    QPixmap kPixmap = pkScreen->grabWindow(0);
    emit FrameCaptured(kPixmap);
}

void Agent::OnRemoteMouseMove(float _nStepX, float _nStepY)
{
    QScreen *screen = QGuiApplication::primaryScreen();
    if(!screen)
        return;
    QSize kSize = screen->size();
    int nPosX = kSize.width() * _nStepX;
    int nPosY = kSize.height() * _nStepY;
    m_kControl->MoveCursor(nPosX, nPosY);
}

void Agent::OnRemoteMousePress(int nButton)
{
    m_kControl->TriggerMousePress(nButton);
}

void Agent::OnRemoteMouseRelease(int nButton)
{
    m_kControl->TriggerMouseRelease(nButton);
}

void Agent::OnRemoteMouseDoubleClick(int nButton)
{
    m_kControl->TriggerClick(nButton);
}

void Agent::OnRemoteMouseWheel(int nButton)
{
    m_kControl->TriggerClick(nButton);
}

void Agent::OnRemoteKeyboardPress(int _nKey)
{
    m_kControl->TriggerKeyPress(_nKey);
}

void Agent::OnRemoteKeyboardRelease(int _nKey)
{
    m_kControl->TriggerKeyRelease(_nKey);
}
