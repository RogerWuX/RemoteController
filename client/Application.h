#pragma once
#include <QApplication>
#include <QSettings>

#include "MatchClient.h"
#include "RtcClient.h"
#include "MatchWnd.h"

#define DEFAULT_STUN_SERVER_NAME "stun:stun.l.google.com:19302"
#define DEFAULT_SIGNALLING_SERVER_ADDRESS "127.0.0.1"
#define DEFAULT_SIGNALLING_SERVER_PORT "7777"

class Setting
{
public:
    Setting();
    ~Setting();
    inline QString GetStunServerName()
    {
        return m_pkSetting->value("StunServerAddress", DEFAULT_STUN_SERVER_NAME).toString();
    }
    inline QString GetSignallingServerAddress()
    {
        return m_pkSetting->value("SignallingServerAddress", DEFAULT_SIGNALLING_SERVER_ADDRESS).toString();
    }
    inline unsigned int GetSignallingServerPort()
    {
        return m_pkSetting->value("SignallingServerPort", DEFAULT_SIGNALLING_SERVER_PORT).toUInt();
    }

protected:
    QSettings *m_pkSetting;
};

class IControllState : public QObject
{
    Q_OBJECT
public:
    IControllState(QObject *parent);
    virtual ~IControllState();
signals:
    void Ready();
    void Error();
    void Finish();
};

class Application : public QApplication
{
    Q_OBJECT
public:
    Application(int &argc, char **argv);
public slots:
    void ConnectToSession(const std::string &_kSessionId);
    void OnMatchClientError();
    void OnReplyConnection(bool _bSuccess);
    void OnRemoteRequestConnection(const std::string &_kSessionId);
    void OnControlStateReady();
    void OnControlStateError();
    void OnControlStateFinish();
protected:
    void InitMetaType();
protected:
    Setting         m_kSetting;
    IControllState *m_pkControllState;
    MatchClient    *m_pkSignalingClient;
    MatchWnd       *m_pkMatchWnd;
};

