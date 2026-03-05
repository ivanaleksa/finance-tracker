#ifndef SNAPSHOTSHISTORYPAGE_H
#define SNAPSHOTSHISTORYPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QList>
#include "investment/snapshot.h"

class SnapshotItemWidget;

class SnapshotsHistoryPage : public QWidget
{
    Q_OBJECT

public:
    explicit SnapshotsHistoryPage(QWidget *parent = nullptr);

public slots:
    void refreshData();

private slots:
    void onSnapshotDeleteRequested(int snapshotId);

private:
    void setupUi();
    void loadSnapshots();
    void clearSnapshots();

    QScrollArea *m_scrollArea;
    QWidget *m_container;
    QVBoxLayout *m_containerLayout;
    QList<SnapshotItemWidget*> m_snapshotItems;
};

#endif // SNAPSHOTSHISTORYPAGE_H
