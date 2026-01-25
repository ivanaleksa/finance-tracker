#ifndef PORTFOLIOPAGE_H
#define PORTFOLIOPAGE_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QMap>
#include <QScrollArea>
#include <QVBoxLayout>
#include "assetcard.h"

class PortfolioPage : public QWidget
{
    Q_OBJECT

public:
    explicit PortfolioPage(QWidget *parent = nullptr);

    void setCurrencyRates(const QMap<int, double>& rates);
    QMap<int, double> getCurrencyRates() const { return m_currencyRates; }

signals:
    void currenciesPageRequested();

public slots:
    void refreshData();

private slots:
    void onAddAssetClicked();
    void onCreateSnapshotClicked();
    void onCurrenciesClicked();
    void onAssetCardClicked(int assetId);
    void onAssetBuyRequested(int assetId);
    void onAssetSellRequested(int assetId);
    void onAssetDeleteRequested(int assetId);
    void onAssetPriceChanged(int assetId, double newPrice);

private:
    void setupUi();
    void loadCurrencyRates();
    void loadAssets();
    void clearCards();

    // Cards container
    QScrollArea *m_scrollArea;
    QWidget *m_cardsContainer;
    QVBoxLayout *m_cardsLayout;
    QList<AssetCard*> m_assetCards;

    // Action buttons
    QPushButton *m_addAssetBtn;
    QPushButton *m_createSnapshotBtn;
    QPushButton *m_currenciesBtn;

    // Currency rates cache
    QMap<int, double> m_currencyRates;
};

#endif // PORTFOLIOPAGE_H
