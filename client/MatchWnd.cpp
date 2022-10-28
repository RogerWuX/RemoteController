#include "MatchWnd.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QClipboard>

MatchWnd::MatchWnd(QWidget *parent)
    : QDialog(parent)
{
    m_pkSessionId = new QLabel();
    m_pkSessionCopyBtn = new QPushButton("Copy");
    m_pkTargetSessionIdInput = new QLineEdit();
    m_pkConfirmBtn = new QPushButton("Confirm");

    QObject::connect(m_pkSessionCopyBtn, &QPushButton::clicked, this, &MatchWnd::CopySessionId);
    QObject::connect(m_pkConfirmBtn, &QPushButton::clicked, this, &MatchWnd::OnConfirm);

    QVBoxLayout *pkVboxLayout = new QVBoxLayout(this);
    QHBoxLayout *pkSessionIdHBoxLayout = new QHBoxLayout();
    pkSessionIdHBoxLayout->addWidget(m_pkSessionId);
    pkSessionIdHBoxLayout->addWidget(m_pkSessionCopyBtn, 0, Qt::AlignRight);
    pkVboxLayout->addLayout(pkSessionIdHBoxLayout);
    pkVboxLayout->addWidget(m_pkTargetSessionIdInput);
    pkVboxLayout->addWidget(m_pkConfirmBtn);
}

void MatchWnd::SetConfirmBtnState(bool _bEnable)
{
    m_pkConfirmBtn->setEnabled(_bEnable);
}

void MatchWnd::UpdateSessionId(const std::string &_kSessionId)
{
    m_pkSessionId->setText(_kSessionId.c_str());
}

void MatchWnd::OnConfirm()
{
    QString kSessionId = m_pkTargetSessionIdInput->text();
    emit ConnectToSession(kSessionId.toStdString());
}

void MatchWnd::CopySessionId()
{
    QClipboard *pkClipBoard = QApplication::clipboard();
    if(!pkClipBoard)
        return;
    QString kSessionId = m_pkSessionId->text();
    if(kSessionId.isEmpty())
        return;
    pkClipBoard->setText(kSessionId);
}

