#ifndef UNDONOTIFICATION_H
#define UNDONOTIFICATION_H

#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QPropertyAnimation>
#include "reporting/transaction.h"

class UndoNotification : public QFrame
{
    Q_OBJECT

public:
    explicit UndoNotification(QWidget *parent = nullptr);
    
    void showNotification(const Transaction& transaction, const QString& categoryName);
    void hideNotification();

signals:
    void undoRequested(const Transaction& transaction);

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
    
    Transaction m_lastTransaction;
};

#endif // UNDONOTIFICATION_H
