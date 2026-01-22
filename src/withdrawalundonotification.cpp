#include "withdrawalundonotification.h"
#include <QHBoxLayout>

WithdrawalUndoNotification::WithdrawalUndoNotification(QWidget *parent)
    : QFrame(parent)
{
    setupUi();
    hide();

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &WithdrawalUndoNotification::onTimeout);
}

void WithdrawalUndoNotification::setupUi()
{
    setObjectName("undoNotification");
    setFixedHeight(60);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(15, 10, 15, 10);
    layout->setSpacing(15);

    m_messageLabel = new QLabel(this);
    m_messageLabel->setObjectName("undoMessage");
    m_messageLabel->setWordWrap(true);
    layout->addWidget(m_messageLabel, 1);

    m_undoBtn = new QPushButton("Отменить", this);
    m_undoBtn->setObjectName("undoButton");
    m_undoBtn->setCursor(Qt::PointingHandCursor);
    connect(m_undoBtn, &QPushButton::clicked, this, &WithdrawalUndoNotification::onUndoClicked);
    layout->addWidget(m_undoBtn);

    m_closeBtn = new QPushButton("✕", this);
    m_closeBtn->setObjectName("closeButton");
    m_closeBtn->setFixedSize(24, 24);
    m_closeBtn->setCursor(Qt::PointingHandCursor);
    connect(m_closeBtn, &QPushButton::clicked, this, &WithdrawalUndoNotification::hideNotification);
    layout->addWidget(m_closeBtn);

    m_animation = new QPropertyAnimation(this, "geometry", this);
    m_animation->setDuration(300);
    m_animation->setEasingCurve(QEasingCurve::OutCubic);
}

void WithdrawalUndoNotification::showNotification(const Withdrawal& withdrawal)
{
    m_lastWithdrawal = withdrawal;

    QString message = QString("Вывод: %1 ₽").arg(withdrawal.amount(), 0, 'f', 2);

    if (!withdrawal.comment().isEmpty()) {
        message += QString(" — %1").arg(withdrawal.comment());
    }

    m_messageLabel->setText(message);

    animateShow();

    m_timer->start(7000);
}

void WithdrawalUndoNotification::hideNotification()
{
    m_timer->stop();
    animateHide();
}

void WithdrawalUndoNotification::onUndoClicked()
{
    m_timer->stop();
    emit undoRequested(m_lastWithdrawal);
    animateHide();
}

void WithdrawalUndoNotification::onTimeout()
{
    animateHide();
}

void WithdrawalUndoNotification::animateShow()
{
    QWidget *parent = parentWidget();
    if (!parent) return;

    int width = parent->width() - 40;
    setFixedWidth(qMin(width, 500));

    int startY = parent->height();
    int endY = parent->height() - height() - 20;
    int x = parent->width() - this->width() - 20;

    setGeometry(x, startY, this->width(), height());
    show();
    raise();

    m_animation->setStartValue(QRect(x, startY, this->width(), height()));
    m_animation->setEndValue(QRect(x, endY, this->width(), height()));
    m_animation->start();
}

void WithdrawalUndoNotification::animateHide()
{
    QWidget *parent = parentWidget();
    if (!parent) {
        hide();
        return;
    }

    int x = geometry().x();
    int startY = geometry().y();
    int endY = parent->height();

    m_animation->setStartValue(QRect(x, startY, width(), height()));
    m_animation->setEndValue(QRect(x, endY, width(), height()));

    connect(m_animation, &QPropertyAnimation::finished, this, [this]() {
        hide();
        disconnect(m_animation, &QPropertyAnimation::finished, nullptr, nullptr);
    });

    m_animation->start();
}
