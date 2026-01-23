#ifndef ASSETDETAILSDIALOG_H
#define ASSETDETAILSDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QTableWidget>
#include "portfolioasset.h"

class AssetDetailsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AssetDetailsDialog(int assetId, QWidget *parent = nullptr);

signals:
    void assetDeleted();

private slots:
    void onUpdatePriceClicked();
    void onAddBuyClicked();
    void onAddSellClicked();
    void onDeleteAssetClicked();
    void onDeleteOperationClicked();
    void refreshData();

private:
    void setupUi();
    void loadAssetData();
    void loadOperations();

    int m_assetId;
    PortfolioAsset m_asset;

    // Info labels
    QLabel *m_nameLabel;
    QLabel *m_categoryLabel;
    QLabel *m_countryLabel;
    QLabel *m_currencyLabel;

    // Price update
    QDoubleSpinBox *m_priceSpin;
    QPushButton *m_updatePriceBtn;

    // Summary labels
    QLabel *m_quantityLabel;
    QLabel *m_avgPriceLabel;
    QLabel *m_currentValueLabel;
    QLabel *m_profitLabel;
    QLabel *m_yieldLabel;

    // Action buttons
    QPushButton *m_addBuyBtn;
    QPushButton *m_addSellBtn;
    QPushButton *m_deleteAssetBtn;

    // Operations table
    QTableWidget *m_operationsTable;
};

#endif // ASSETDETAILSDIALOG_H
