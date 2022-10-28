#include "Application.h"

#include <cassert>

#include <QDir>

#include "Controller.h"
#include "Agent.h"

Setting::Setting()
{
    QString kIniFilePath = QApplication::applicationDirPath() + QDir::separator() + "setting.ini";
    m_pkSetting = new QSettings(kIniFilePath, QSettings::IniFormat);
}

Setting::~Setting()
{
    delete m_pkSetting;
}

IControllState::IControllState(QObject *parent)
    :QObject(parent)
{
}

IControllState::~IControllState()
{
}

Application::Application(int &argc, char **argv)
    :QApplication(argc,argv),
     m_kSetting(),
     m_pkControllState(nullptr)
{
    InitMetaType();    

    m_pkMatchWnd = new MatchWnd();
    m_pkSignalingClient = new MatchClient();

    QObject::connect(m_pkSignalingClient, &MatchClient::UpdateSessionId, m_pkMatchWnd, &MatchWnd::UpdateSessionId);

    QObject::connect(m_pkSignalingClient, &MatchClient::ErrorHappen,                this, &Application::OnMatchClientError);
    QObject::connect(m_pkSignalingClient, &MatchClient::RemoteRequestConnection,    this, &Application::OnRemoteRequestConnection);
    QObject::connect(m_pkSignalingClient, &MatchClient::ReplyConnection,            this, &Application::OnReplyConnection);

    QObject::connect(m_pkMatchWnd,        &MatchWnd::ConnectToSession,  this, &Application::ConnectToSession);

    m_pkSignalingClient->ConnectToServer(m_kSetting.GetSignallingServerAddress().toStdString(), m_kSetting.GetSignallingServerPort());
    m_pkMatchWnd->show();
}

void Application::InitMetaType()
{
    qRegisterMetaType<std::string>("std::string");
    qRegisterMetaType<webrtc::PeerConnectionInterface::PeerConnectionState>("webrtc::PeerConnectionInterface::PeerConnectionState");
    qRegisterMetaType<rtc::scoped_refptr<webrtc::DataChannelInterface>>("rtc::scoped_refptr<webrtc::DataChannelInterface>");
    qRegisterMetaType<rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>>("rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>");
    qRegisterMetaType<std::shared_ptr<uint8_t[]>>("std::shared_ptr<uint8_t[]>");
    qRegisterMetaType<rtc::CopyOnWriteBuffer>("rtc::CopyOnWriteBuffer");
}

void Application::ConnectToSession(const std::string &_kSessionId)
{
    //Avoid multiple request before server reply
    m_pkMatchWnd->SetConfirmBtnState(false);

    protocol::signalling::SessionId kMsg;
    kMsg.set_session_id(_kSessionId);
    std::string kData;
    kMsg.SerializeToString(&kData);
    m_pkSignalingClient->SendMsg(protocol::signalling::eMT_RequestConnection, kData);
}

void Application::OnMatchClientError()
{
    quit();
}

void Application::OnReplyConnection(bool _bSuccess)
{
    if(!_bSuccess){
        m_pkMatchWnd->SetConfirmBtnState(true);
        return;
    }

    assert(!m_pkControllState);
    Controller *pkController = new Controller();
    QObject::connect(pkController, &IControllState::Ready,  this,   &Application::OnControlStateReady);
    QObject::connect(pkController, &IControllState::Error,  this,   &Application::OnControlStateError);
    QObject::connect(pkController, &IControllState::Finish, this,   &Application::OnControlStateFinish);
    pkController->Init(m_pkSignalingClient, m_kSetting.GetStunServerName().toStdString());
    m_pkControllState = pkController;
}

void Application::OnRemoteRequestConnection(const std::string &_kSessionId)
{
    assert(!m_pkControllState);

    Agent *pkAgent = new Agent();
    QObject::connect(pkAgent, &IControllState::Ready,   this, &Application::OnControlStateReady);
    QObject::connect(pkAgent, &IControllState::Error,   this, &Application::OnControlStateError);
    QObject::connect(pkAgent, &IControllState::Finish,  this, &Application::OnControlStateFinish);
    pkAgent->Init(m_pkSignalingClient, m_kSetting.GetStunServerName().toStdString());
    m_pkControllState = pkAgent;
}


void Application::OnControlStateReady()
{
    m_pkMatchWnd->hide();
    m_pkSignalingClient->DisconnectFromServer();
}

void Application::OnControlStateError()
{
    quit();
}

void Application::OnControlStateFinish()
{
    quit();
}



