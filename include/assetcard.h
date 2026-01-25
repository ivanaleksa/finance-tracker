#ifndef ASSETCARD_H
#define ASSETCARD_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QMenu>
#include "portfolioasset.h"

class AssetCard : public QWidget
{
    Q_OBJECT

public:
    explicit AssetCard(const PortfolioAsset& asset, QWidget *parent = nullptr);

    int assetId() const { return m_asset.id(); }
    void updateAsset(const PortfolioAsset& asset);

signals:
    void clicked();
    void buyRequested(int assetId);
    void sellRequested(int assetId);
    void deleteRequested(int assetId);
    void priceChanged(int assetId, double newPrice);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onMenuButtonClicked();
    void onBuyClicked();
    void onSellClicked();
    void onDeleteClicked();
    void onPriceLabelClicked();
    void onPriceEditFinished();

private:
    void setupUi();
    void updateDisplay();

    PortfolioAsset m_asset;

    // Labels
    QLabel *m_nameLabel;
    QLabel *m_quantityLabel;
    QLabel *m_avgPriceLabel;
    QLabel *m_currentPriceLabel;  // Clickable for editing
    QLineEdit *m_priceEdit;       // Hidden edit field
    QLabel *m_valueLabel;
    QLabel *m_profitPercentLabel;
    QLabel *m_profitRubLabel;
    QLabel *m_currencyLabel;
    QLabel *m_countryLabel;

    // Menu
    QPushButton *m_menuBtn;
    QMenu *m_menu;
};

#endif // ASSETCARD_H
