#ifndef WITHDRAWALUNDONOTIFICATION_H
#define WITHDRAWALUNDONOTIFICATION_H

#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QPropertyAnimation>
#include "withdrawal.h"

class WithdrawalUndoNotification : public QFrame
{
    Q_OBJECT

public:
    explicit WithdrawalUndoNotification(QWidget *parent = nullptr);

    void showNotification(const Withdrawal& withdrawal);
    void hideNotification();

signals:
    void undoRequested(const Withdrawal& withdrawal);

private slots:
    void onUndoClicked();
    void onTimeout();

private:
    void setupUi();
    void animateShow();
    void animateHide();

    QLabel *m_messageLabel;
    QPushButton *m_undoBtn;
    QPushButton *m_closeBtn;
    QTimer *m_timer;
    QPropertyAnimation *m_animation;

    Withdrawal m_lastWithdrawal;
};

#endif // WITHDRAWALUNDONOTIFICATION_H
