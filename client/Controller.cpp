#include "Controller.h"

#include <X11/Xlib.h>

Controller::Controller(QObject *parent)
    :IControllState(parent)
{
    m_pkRtcClient = new ControllerClient();
    m_pkControllerWnd = new ControllerWnd();
}

void Controller::Init(MatchClient *_pkSignallingClient, const std::string &_kStunServerAddress)
{
    QObject::connect(m_pkRtcClient, &RtcClient::SendSignalMsg, _pkSignallingClient, &MatchClient::SendMsg);

    QObject::connect(_pkSignallingClient, &MatchClient::RemoteSdpReceived, m_pkRtcClient, &RtcClient::OnRemoteSdpReceived);
    QObject::connect(_pkSignallingClient, &MatchClient::RemoteIceCandidateReceived, m_pkRtcClient, &RtcClient::OnRemoteIceCandidateReceived);

    QObject::connect(m_pkRtcClient, &RtcClient::PeerConnected, this, &Controller::OnPeerConnected);
    QObject::connect(m_pkRtcClient, &RtcClient::PeerDisconnected, this, &Controller::OnPeerDisconnected);

    m_pkRtcClient->InitPeerConncetion(_kStunServerAddress);
}

void Controller::OnPeerConnected()
{
    QObject::connect(m_pkRtcClient, &ControllerClient::FrameReceived, m_pkControllerWnd, &ControllerWnd::OnFrameUpdate);

    QObject::connect(m_pkControllerWnd, &ControllerWnd::MouseMoved,         this, &Controller::OnMouseMoved);
    QObject::connect(m_pkControllerWnd, &ControllerWnd::MousePressed,       this, &Controller::OnMousePressed);
    QObject::connect(m_pkControllerWnd, &ControllerWnd::MouseReleased,      this, &Controller::OnMouseReleased);
    QObject::connect(m_pkControllerWnd, &ControllerWnd::MouseDoubleClicked, this, &Controller::OnMouseDoubleClicked);
    QObject::connect(m_pkControllerWnd, &ControllerWnd::MouseWheel,         this, &Controller::OnMouseWheel);

    QObject::connect(m_pkControllerWnd, &ControllerWnd::KeyPressed,     this, &Controller::OnKeyPressed);
    QObject::connect(m_pkControllerWnd, &ControllerWnd::KeyReleased,    this, &Controller::OnKeyReleased);

    emit Ready();
    m_pkControllerWnd->show();
}

void Controller::OnPeerDisconnected()
{
    emit Finish();
}

void Controller::OnMouseMoved(int _nX, int _nY)
{
    float fWidthRatio = _nX * 1.0f / m_pkControllerWnd->width();
    float fHeightRatio = _nY * 1.0f / m_pkControllerWnd->height();

    protocol::p2p::MouseMoveEvent kMsg;
    kMsg.set_step_x(fWidthRatio);
    kMsg.set_step_y(fHeightRatio);

    m_pkRtcClient->SendDataChannelMsg(protocol::p2p::eMT_MouseMove, &kMsg);
}

void Controller::OnMousePressed(Qt::MouseButton _eButton)
{
    int nSysBtn = QtButtonToSysButton(_eButton);
    if(nSysBtn == 0)
        return;
    protocol::p2p::MouseButtonStateEvent kMsg;
    kMsg.set_button(nSysBtn);

    m_pkRtcClient->SendDataChannelMsg(protocol::p2p::eMT_MousePress, &kMsg);
}

void Controller::OnMouseReleased(Qt::MouseButton _eButton)
{
    int nSysBtn = QtButtonToSysButton(_eButton);
    if(nSysBtn == 0)
        return;
    protocol::p2p::MouseButtonStateEvent kMsg;
    kMsg.set_button(nSysBtn);

    m_pkRtcClient->SendDataChannelMsg(protocol::p2p::eMT_MouseRelease, &kMsg);
}

void Controller::OnMouseDoubleClicked(Qt::MouseButton _eButton)
{
    int nSysBtn = QtButtonToSysButton(_eButton);
    if(nSysBtn == 0)
        return;
    protocol::p2p::MouseButtonStateEvent kMsg;
    kMsg.set_button(nSysBtn);

    m_pkRtcClient->SendDataChannelMsg(protocol::p2p::eMT_MouseDoubleClick, &kMsg);
}

void Controller::OnMouseWheel(QPoint _kAngleDelta)
{
    protocol::p2p::MouseButtonStateEvent kMsg;
    kMsg.set_button(_kAngleDelta.y() > 0 ? Button4 : Button5);

    m_pkRtcClient->SendDataChannelMsg(protocol::p2p::eMT_MouseDoubleClick, &kMsg);
}

void Controller::OnKeyPressed(int _nKey)
{
    protocol::p2p::KeyboardEvent kMsg;
    kMsg.set_key(_nKey);

    m_pkRtcClient->SendDataChannelMsg(protocol::p2p::eMT_KeyboardPress, &kMsg);
}

void Controller::OnKeyReleased(int _nKey)
{
    protocol::p2p::KeyboardEvent kMsg;
    kMsg.set_key(_nKey);

    m_pkRtcClient->SendDataChannelMsg(protocol::p2p::eMT_KeyboardRelease, &kMsg);
}

int Controller::QtButtonToSysButton(Qt::MouseButton _eButton)
{
    switch (_eButton)
    {
    case Qt::LeftButton:
        return Button1;
    case Qt::MiddleButton:
        return Button2;
    case Qt::RightButton:
        return Button3;
    }
    return 0;
}
