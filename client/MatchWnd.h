#pragma once
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class MatchWnd : public QDialog
{
    Q_OBJECT
public:
    explicit MatchWnd(QWidget *parent = nullptr);
    void SetConfirmBtnState(bool _bEnable);
signals:
    void ConnectToSession(const std::string &_kSessionId);
public slots:
    void UpdateSessionId(const std::string &_kSessionId);
    void OnConfirm();
    void CopySessionId();
protected:
    QLabel *m_pkSessionId;
    QPushButton *m_pkSessionCopyBtn;
    QLineEdit *m_pkTargetSessionIdInput;
    QPushButton *m_pkConfirmBtn;
};

