#ifndef PORTFOLIOPAGE_H
#define PORTFOLIOPAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QMap>

class PortfolioPage : public QWidget
{
    Q_OBJECT

public:
    explicit PortfolioPage(QWidget *parent = nullptr);

public slots:
    void refreshData();

private slots:
    void onAddAssetClicked();
    void onCreateSnapshotClicked();
    void onAssetDoubleClicked(int row, int column);
    void onCurrencyRateChanged();

private:
    void setupUi();
    void loadCurrencyRates();
    void loadAssets();
    void updateTotals();

    // Currency rates table
    QTableWidget *m_currencyTable;

    // Assets table
    QTableWidget *m_assetsTable;

    // Action buttons
    QPushButton *m_addAssetBtn;
    QPushButton *m_createSnapshotBtn;

    // Totals labels
    QLabel *m_totalValueLabel;
    QLabel *m_totalProfitLabel;
    QLabel *m_totalYieldLabel;

    // Currency rates cache
    QMap<int, double> m_currencyRates;
};

#endif // PORTFOLIOPAGE_H
