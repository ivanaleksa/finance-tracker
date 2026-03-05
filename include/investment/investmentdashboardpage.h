#ifndef INVESTMENTDASHBOARDPAGE_H
#define INVESTMENTDASHBOARDPAGE_H

#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QMap>
#include "widgets/barchart.h"
#include "widgets/piechart.h"

class InvestmentDashboardPage : public QWidget
{
    Q_OBJECT

public:
    explicit InvestmentDashboardPage(QWidget *parent = nullptr);

public slots:
    void refreshData();

private slots:
    void onCurrencyChanged();
    void onPeriodChanged();
    void onPieTypeChanged();

private:
    void setupUi();
    void loadCurrencies();
    void updateSummary();
    void updateBarChart();
    void updatePieChart();
    double convertToSelectedCurrency(double rubValue);

    // Summary section
    QLabel *m_totalValueLabel;
    QLabel *m_totalInvestedLabel;
    QLabel *m_totalProfitLabel;
    QLabel *m_profitPercentLabel;

    // Currency selector
    QComboBox *m_currencyCombo;
    int m_selectedCurrencyId = -1;  // -1 means RUB

    // Bar chart
    BarChart *m_barChart;
    QComboBox *m_periodCombo;

    // Pie chart
    PieChart *m_pieChart;
    QComboBox *m_pieTypeCombo;

    // Currency rates cache
    QMap<int, double> m_currencyRates;
};

#endif // INVESTMENTDASHBOARDPAGE_H
