#ifndef DEPOSITUNDONOTIFICATION_H
#define DEPOSITUNDONOTIFICATION_H

#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QPropertyAnimation>
#include "deposit.h"

class DepositUndoNotification : public QFrame
{
    Q_OBJECT

public:
    explicit DepositUndoNotification(QWidget *parent = nullptr);

    void showNotification(const Deposit& deposit);
    void hideNotification();

signals:
    void undoRequested(const Deposit& deposit);

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

    Deposit m_lastDeposit;
};

#endif // DEPOSITUNDONOTIFICATION_H
