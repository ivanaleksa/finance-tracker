#ifndef SNAPSHOTITEMWIDGET_H
#define SNAPSHOTITEMWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QFrame>
#include "investment/snapshot.h"

class SnapshotItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SnapshotItemWidget(const Snapshot& snapshot, QWidget *parent = nullptr);

    int snapshotId() const { return m_snapshot.id(); }

signals:
    void deleteRequested(int snapshotId);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void toggleExpanded();
    void onDeleteClicked();

private:
    void setupUi();
    void updateExpandedState();
    double calculateTotalValue() const;

    Snapshot m_snapshot;
    bool m_expanded = false;
    QString m_fullDescription;

    // Header (always visible)
    QWidget *m_headerWidget;
    QLabel *m_dateLabel;
    QLabel *m_descriptionLabel;
    QLabel *m_fullDescriptionLabel;  // Shown when expanded
    QLabel *m_totalLabel;
    QPushButton *m_expandBtn;
    QPushButton *m_deleteBtn;

    // Expandable content
    QWidget *m_contentWidget;
    QTableWidget *m_positionsTable;
    QTableWidget *m_currenciesTable;
    QLabel *m_summaryLabel;
};

#endif // SNAPSHOTITEMWIDGET_H
