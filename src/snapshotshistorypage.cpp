#include "snapshotshistorypage.h"
#include "snapshotitemwidget.h"
#include "database.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>

SnapshotsHistoryPage::SnapshotsHistoryPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    loadSnapshots();

    connect(&Database::instance(), &Database::investmentDataChanged,
            this, &SnapshotsHistoryPage::refreshData);
}

void SnapshotsHistoryPage::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 30, 40, 30);
    mainLayout->setSpacing(20);

    // Title
    QLabel *titleLabel = new QLabel("История снимков", this);
    titleLabel->setObjectName("pageTitle");
    mainLayout->addWidget(titleLabel);

    // Scroll area
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setStyleSheet("QScrollArea { background: transparent; }");

    m_container = new QWidget();
    m_container->setStyleSheet("background: transparent;");
    m_containerLayout = new QVBoxLayout(m_container);
    m_containerLayout->setContentsMargins(0, 0, 0, 0);
    m_containerLayout->setSpacing(15);
    m_containerLayout->addStretch();

    m_scrollArea->setWidget(m_container);
    mainLayout->addWidget(m_scrollArea, 1);
}

void SnapshotsHistoryPage::refreshData()
{
    loadSnapshots();
}

void SnapshotsHistoryPage::clearSnapshots()
{
    for (SnapshotItemWidget *item : m_snapshotItems) {
        item->deleteLater();
    }
    m_snapshotItems.clear();

    // Remove all widgets from layout except the stretch
    while (m_containerLayout->count() > 1) {
        QLayoutItem *item = m_containerLayout->takeAt(0);
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
}

void SnapshotsHistoryPage::loadSnapshots()
{
    clearSnapshots();

    QList<Snapshot> snapshots = Database::instance().getAllSnapshots();

    // Sort by date descending
    std::sort(snapshots.begin(), snapshots.end(), [](const Snapshot& a, const Snapshot& b) {
        return a.date() > b.date();
    });

    if (snapshots.isEmpty()) {
        QLabel *emptyLabel = new QLabel("Снимки портфеля отсутствуют", this);
        emptyLabel->setStyleSheet("font-size: 14px; color: #7f8c8d; padding: 40px;");
        emptyLabel->setAlignment(Qt::AlignCenter);
        m_containerLayout->insertWidget(0, emptyLabel);
        return;
    }

    for (const Snapshot& snapshot : snapshots) {
        SnapshotItemWidget *item = new SnapshotItemWidget(snapshot, m_container);
        connect(item, &SnapshotItemWidget::deleteRequested,
                this, &SnapshotsHistoryPage::onSnapshotDeleteRequested);

        m_containerLayout->insertWidget(m_containerLayout->count() - 1, item);
        m_snapshotItems.append(item);
    }
}

void SnapshotsHistoryPage::onSnapshotDeleteRequested(int snapshotId)
{
    if (Database::instance().deleteSnapshot(snapshotId)) {
        loadSnapshots();
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось удалить снимок");
    }
}
