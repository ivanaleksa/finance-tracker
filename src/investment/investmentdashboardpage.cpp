#include "investment/investmentdashboardpage.h"
#include "database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFrame>
#include <QScrollArea>

InvestmentDashboardPage::InvestmentDashboardPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    loadCurrencies();
    refreshData();

    connect(&Database::instance(), &Database::portfolioDataChanged,
            this, &InvestmentDashboardPage::refreshData);
    connect(&Database::instance(), &Database::investmentDataChanged,
            this, &InvestmentDashboardPage::refreshData);
}

void InvestmentDashboardPage::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 30, 40, 30);
    mainLayout->setSpacing(20);

    // Title row with currency selector
    QHBoxLayout *titleLayout = new QHBoxLayout();

    QLabel *titleLabel = new QLabel("Обзор инвестиций", this);
    titleLabel->setObjectName("pageTitle");
    titleLayout->addWidget(titleLabel);

    titleLayout->addStretch();

    QLabel *currLabel = new QLabel("Валюта:", this);
    currLabel->setStyleSheet("font-size: 13px; color: #7f8c8d;");
    titleLayout->addWidget(currLabel);

    m_currencyCombo = new QComboBox(this);
    m_currencyCombo->setMinimumWidth(120);
    connect(m_currencyCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &InvestmentDashboardPage::onCurrencyChanged);
    titleLayout->addWidget(m_currencyCombo);

    mainLayout->addLayout(titleLayout);

    // Summary cards
    QHBoxLayout *summaryLayout = new QHBoxLayout();
    summaryLayout->setSpacing(20);

    // Total value card
    QFrame *valueCard = new QFrame(this);
    valueCard->setObjectName("summaryCard");
    valueCard->setStyleSheet(
        "QFrame#summaryCard { background: white; border-radius: 10px; border: 1px solid #e0e0e0; }"
    );
    QVBoxLayout *valueLayout = new QVBoxLayout(valueCard);
    valueLayout->setContentsMargins(20, 15, 20, 15);
    QLabel *valueTitle = new QLabel("Стоимость портфеля", valueCard);
    valueTitle->setStyleSheet("font-size: 12px; color: #7f8c8d;");
    m_totalValueLabel = new QLabel("0", valueCard);
    m_totalValueLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50;");
    valueLayout->addWidget(valueTitle);
    valueLayout->addWidget(m_totalValueLabel);
    summaryLayout->addWidget(valueCard);

    // Invested card
    QFrame *investedCard = new QFrame(this);
    investedCard->setObjectName("summaryCard");
    investedCard->setStyleSheet(
        "QFrame#summaryCard { background: white; border-radius: 10px; border: 1px solid #e0e0e0; }"
    );
    QVBoxLayout *investedLayout = new QVBoxLayout(investedCard);
    investedLayout->setContentsMargins(20, 15, 20, 15);
    QLabel *investedTitle = new QLabel("Вложено", investedCard);
    investedTitle->setStyleSheet("font-size: 12px; color: #7f8c8d;");
    m_totalInvestedLabel = new QLabel("0", investedCard);
    m_totalInvestedLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #3498db;");
    investedLayout->addWidget(investedTitle);
    investedLayout->addWidget(m_totalInvestedLabel);
    summaryLayout->addWidget(investedCard);

    // Profit card
    QFrame *profitCard = new QFrame(this);
    profitCard->setObjectName("summaryCard");
    profitCard->setStyleSheet(
        "QFrame#summaryCard { background: white; border-radius: 10px; border: 1px solid #e0e0e0; }"
    );
    QVBoxLayout *profitLayout = new QVBoxLayout(profitCard);
    profitLayout->setContentsMargins(20, 15, 20, 15);
    QLabel *profitTitle = new QLabel("Прибыль", profitCard);
    profitTitle->setStyleSheet("font-size: 12px; color: #7f8c8d;");
    m_totalProfitLabel = new QLabel("0", profitCard);
    m_totalProfitLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #27ae60;");
    profitLayout->addWidget(profitTitle);
    profitLayout->addWidget(m_totalProfitLabel);
    summaryLayout->addWidget(profitCard);

    // Yield card
    QFrame *yieldCard = new QFrame(this);
    yieldCard->setObjectName("summaryCard");
    yieldCard->setStyleSheet(
        "QFrame#summaryCard { background: white; border-radius: 10px; border: 1px solid #e0e0e0; }"
    );
    QVBoxLayout *yieldLayout = new QVBoxLayout(yieldCard);
    yieldLayout->setContentsMargins(20, 15, 20, 15);
    QLabel *yieldTitle = new QLabel("Доходность", yieldCard);
    yieldTitle->setStyleSheet("font-size: 12px; color: #7f8c8d;");
    m_profitPercentLabel = new QLabel("0%", yieldCard);
    m_profitPercentLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #27ae60;");
    yieldLayout->addWidget(yieldTitle);
    yieldLayout->addWidget(m_profitPercentLabel);
    summaryLayout->addWidget(yieldCard);

    mainLayout->addLayout(summaryLayout);

    // Scroll area for charts
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet("QScrollArea { background: transparent; }");

    QWidget *chartsContainer = new QWidget();
    chartsContainer->setStyleSheet("background: transparent;");
    QVBoxLayout *chartsLayout = new QVBoxLayout(chartsContainer);
    chartsLayout->setContentsMargins(0, 0, 0, 0);
    chartsLayout->setSpacing(20);

    // Bar chart section
    QFrame *barChartFrame = new QFrame(chartsContainer);
    barChartFrame->setStyleSheet(
        "QFrame { background: white; border-radius: 10px; border: 1px solid #e0e0e0; }"
    );
    QVBoxLayout *barChartLayout = new QVBoxLayout(barChartFrame);
    barChartLayout->setContentsMargins(15, 15, 15, 15);

    QHBoxLayout *barHeaderLayout = new QHBoxLayout();
    QLabel *barTitle = new QLabel("История стоимости", barChartFrame);
    barTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #2c3e50;");
    barHeaderLayout->addWidget(barTitle);
    barHeaderLayout->addStretch();

    m_periodCombo = new QComboBox(barChartFrame);
    m_periodCombo->addItem("6 снимков", 6);
    m_periodCombo->addItem("12 снимков", 12);
    m_periodCombo->addItem("24 снимка", 24);
    m_periodCombo->addItem("52 снимка", 52);
    m_periodCombo->addItem("Все", -1);
    m_periodCombo->setCurrentIndex(1);  // Default 12
    connect(m_periodCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &InvestmentDashboardPage::onPeriodChanged);
    barHeaderLayout->addWidget(m_periodCombo);

    barChartLayout->addLayout(barHeaderLayout);

    m_barChart = new BarChart(barChartFrame);
    m_barChart->setMinimumHeight(500);
    barChartLayout->addWidget(m_barChart, 1);

    chartsLayout->addWidget(barChartFrame);

    // Pie chart section
    QFrame *pieChartFrame = new QFrame(chartsContainer);
    pieChartFrame->setStyleSheet(
        "QFrame { background: white; border-radius: 10px; border: 1px solid #e0e0e0; }"
    );
    QVBoxLayout *pieChartLayout = new QVBoxLayout(pieChartFrame);
    pieChartLayout->setContentsMargins(15, 15, 15, 15);

    QHBoxLayout *pieHeaderLayout = new QHBoxLayout();
    QLabel *pieTitle = new QLabel("Распределение", pieChartFrame);
    pieTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #2c3e50;");
    pieHeaderLayout->addWidget(pieTitle);
    pieHeaderLayout->addStretch();

    m_pieTypeCombo = new QComboBox(pieChartFrame);
    m_pieTypeCombo->addItem("По активам", 0);
    m_pieTypeCombo->addItem("По валютам", 1);
    m_pieTypeCombo->addItem("По типам", 2);
    connect(m_pieTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &InvestmentDashboardPage::onPieTypeChanged);
    pieHeaderLayout->addWidget(m_pieTypeCombo);

    pieChartLayout->addLayout(pieHeaderLayout);

    m_pieChart = new PieChart(pieChartFrame);
    m_pieChart->setMinimumHeight(500);
    pieChartLayout->addWidget(m_pieChart, 1);

    chartsLayout->addWidget(pieChartFrame);
    chartsLayout->addStretch();

    scrollArea->setWidget(chartsContainer);
    mainLayout->addWidget(scrollArea, 1);
}

void InvestmentDashboardPage::loadCurrencies()
{
    m_currencyCombo->clear();
    m_currencyRates.clear();

    QList<Currency> currencies = Database::instance().getCurrencies();

    for (const Currency& curr : currencies) {
        m_currencyRates[curr.id()] = curr.rate();
        m_currencyCombo->addItem(curr.code(), curr.id());

        if (curr.code() == "RUB") {
            m_currencyCombo->setCurrentIndex(m_currencyCombo->count() - 1);
            m_selectedCurrencyId = curr.id();
        }
    }
}

void InvestmentDashboardPage::refreshData()
{
    // Reload currency rates
    QList<Currency> currencies = Database::instance().getCurrencies();
    for (const Currency& curr : currencies) {
        m_currencyRates[curr.id()] = curr.rate();
    }

    updateSummary();
    updateBarChart();
    updatePieChart();
}

void InvestmentDashboardPage::onCurrencyChanged()
{
    m_selectedCurrencyId = m_currencyCombo->currentData().toInt();
    updateSummary();
    updateBarChart();
}

void InvestmentDashboardPage::onPeriodChanged()
{
    updateBarChart();
}

void InvestmentDashboardPage::onPieTypeChanged()
{
    updatePieChart();
}

double InvestmentDashboardPage::convertToSelectedCurrency(double rubValue)
{
    if (m_selectedCurrencyId <= 0) return rubValue;

    double rate = m_currencyRates.value(m_selectedCurrencyId, 1.0);
    if (rate <= 0) return rubValue;

    return rubValue / rate;
}

void InvestmentDashboardPage::updateSummary()
{
    QList<PortfolioAsset> assets = Database::instance().getActivePortfolioAssets();

    double totalValueRub = 0.0;

    for (const PortfolioAsset& asset : assets) {
        double rate = m_currencyRates.value(asset.currencyId(), 1.0);

        // Current value of all assets
        totalValueRub += asset.currentPrice() * asset.totalQuantity() * rate;
    }

    // Invested = Total deposits - Total withdrawals (external money flow)
    double totalInvestedRub = Database::instance().getTotalDeposits() - Database::instance().getTotalWithdrawals();

    // Profit = Current value - Net invested
    double profitRub = totalValueRub - totalInvestedRub;
    double profitPercent = totalInvestedRub > 0 ? (profitRub / totalInvestedRub) * 100.0 : 0.0;

    // Convert to selected currency
    double totalValue = convertToSelectedCurrency(totalValueRub);
    double totalInvested = convertToSelectedCurrency(totalInvestedRub);
    double profit = convertToSelectedCurrency(profitRub);

    // Get currency symbol
    QString currSymbol = m_currencyCombo->currentText();

    m_totalValueLabel->setText(QString("%1 %2").arg(totalValue, 0, 'f', 2).arg(currSymbol));
    m_totalInvestedLabel->setText(QString("%1 %2").arg(totalInvested, 0, 'f', 2).arg(currSymbol));

    QString profitSign = profit >= 0 ? "+" : "";
    QString profitColor = profit >= 0 ? "#27ae60" : "#e74c3c";
    m_totalProfitLabel->setText(QString("%1%2 %3").arg(profitSign).arg(profit, 0, 'f', 2).arg(currSymbol));
    m_totalProfitLabel->setStyleSheet(QString("font-size: 24px; font-weight: bold; color: %1;").arg(profitColor));

    QString yieldSign = profitPercent >= 0 ? "+" : "";
    QString yieldColor = profitPercent >= 0 ? "#27ae60" : "#e74c3c";
    m_profitPercentLabel->setText(QString("%1%2%").arg(yieldSign).arg(profitPercent, 0, 'f', 2));
    m_profitPercentLabel->setStyleSheet(QString("font-size: 24px; font-weight: bold; color: %1;").arg(yieldColor));
}

void InvestmentDashboardPage::updateBarChart()
{
    QList<Snapshot> snapshots = Database::instance().getAllSnapshots();

    // Sort by date ascending
    std::sort(snapshots.begin(), snapshots.end(), [](const Snapshot& a, const Snapshot& b) {
        return a.date() < b.date();
    });

    // Limit to selected period
    int limit = m_periodCombo->currentData().toInt();
    if (limit > 0 && snapshots.size() > limit) {
        snapshots = snapshots.mid(snapshots.size() - limit);
    }

    QStringList labels;
    QMap<int, double> data;

    int index = 0;
    for (const Snapshot& snapshot : snapshots) {
        labels << snapshot.date().toString("dd.MM");

        // Calculate total value for this snapshot
        double totalRub = 0.0;
        QList<SnapshotPosition> positions = snapshot.positions();
        QMap<int, double> rates = snapshot.currencyRates();

        for (const SnapshotPosition& pos : positions) {
            double rate = rates.value(pos.currencyId(), 1.0);
            totalRub += pos.price() * pos.quantity() * rate;
        }

        double value = convertToSelectedCurrency(totalRub);
        data[index++] = value;
    }

    m_barChart->setLabels(labels);
    m_barChart->setData(data);
    m_barChart->update();
}

void InvestmentDashboardPage::updatePieChart()
{
    QList<PortfolioAsset> assets = Database::instance().getActivePortfolioAssets();

    QMap<QString, double> data;
    int pieType = m_pieTypeCombo->currentData().toInt();

    if (pieType == 0) {
        // By assets
        for (const PortfolioAsset& asset : assets) {
            if (asset.totalQuantity() <= 0) continue;

            double rate = m_currencyRates.value(asset.currencyId(), 1.0);
            double valueRub = asset.currentPrice() * asset.totalQuantity() * rate;

            data[asset.name()] = valueRub;
        }
    } else if (pieType == 1) {
        // By currencies
        QMap<int, double> byCurrency;
        for (const PortfolioAsset& asset : assets) {
            if (asset.totalQuantity() <= 0) continue;

            double rate = m_currencyRates.value(asset.currencyId(), 1.0);
            double valueRub = asset.currentPrice() * asset.totalQuantity() * rate;

            byCurrency[asset.currencyId()] += valueRub;
        }

        for (auto it = byCurrency.begin(); it != byCurrency.end(); ++it) {
            Currency curr = Database::instance().getCurrency(it.key());
            data[curr.code()] = it.value();
        }
    } else {
        // By category (asset type)
        for (const PortfolioAsset& asset : assets) {
            if (asset.totalQuantity() <= 0) continue;

            double rate = m_currencyRates.value(asset.currencyId(), 1.0);
            double valueRub = asset.currentPrice() * asset.totalQuantity() * rate;

            QString categoryName = asset.categoryName();
            if (categoryName.isEmpty()) {
                categoryName = "Без категории";
            }
            data[categoryName] += valueRub;
        }
    }

    m_pieChart->setData(data);
    m_pieChart->update();
}
