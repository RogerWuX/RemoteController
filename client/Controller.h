#pragma once
#include "Application.h"

#include "ControllerClient.h"
#include "ControllerWnd.h"

class Controller: public IControllState
{
    Q_OBJECT
public:
    Controller(QObject *parent = nullptr);
    void Init(MatchClient *_pkSignallingClient, const std::string &_kStunServerAddress);
public slots:
    void OnPeerConnected();
    void OnPeerDisconnected();

    void OnMouseMoved(int _nX, int _nY);
    void OnMousePressed(Qt::MouseButton _eButton);
    void OnMouseReleased(Qt::MouseButton _eButton);
    void OnMouseDoubleClicked(Qt::MouseButton _eButton);
    void OnMouseWheel(QPoint _kAngleDelta);
    void OnKeyPressed(int _nKey);
    void OnKeyReleased(int _nKey);
protected:
    int QtButtonToSysButton(Qt::MouseButton _eButton);
    ControllerClient   *m_pkRtcClient;
    ControllerWnd*      m_pkControllerWnd;
};

