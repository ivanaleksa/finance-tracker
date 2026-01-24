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
    void onAssetDoubleClicked(int row, int column);

private:
    void setupUi();
    void loadCurrencyRates();
    void loadAssets();
    void updateTotals();

    // Assets table
    QTableWidget *m_assetsTable;

    // Action buttons
    QPushButton *m_addAssetBtn;
    QPushButton *m_createSnapshotBtn;
    QPushButton *m_currenciesBtn;

    // Totals labels
    QLabel *m_totalValueLabel;
    QLabel *m_totalProfitLabel;
    QLabel *m_totalYieldLabel;

    // Currency rates cache
    QMap<int, double> m_currencyRates;
};

#endif // PORTFOLIOPAGE_H
