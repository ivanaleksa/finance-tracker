#ifndef ASSETHISTORYDIALOG_H
#define ASSETHISTORYDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include "portfolioasset.h"

class AssetHistoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AssetHistoryDialog(int assetId, QWidget *parent = nullptr);

private slots:
    void onDeleteOperationClicked();
    void refreshOperations();

private:
    void setupUi();
    void loadOperations();

    int m_assetId;
    QString m_assetName;
    QTableWidget *m_operationsTable;
};

#endif // ASSETHISTORYDIALOG_H
