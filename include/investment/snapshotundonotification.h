#ifndef SNAPSHOTUNDONOTIFICATION_H
#define SNAPSHOTUNDONOTIFICATION_H

#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QPropertyAnimation>
#include "investment/snapshot.h"

class SnapshotUndoNotification : public QFrame
{
    Q_OBJECT

public:
    explicit SnapshotUndoNotification(QWidget *parent = nullptr);

    void showNotification(const Snapshot& snapshot);
    void hideNotification();

signals:
    void undoRequested(const Snapshot& snapshot);

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

    Snapshot m_lastSnapshot;
};

#endif // SNAPSHOTUNDONOTIFICATION_H
