#include "monthchartpage.h"
#include "piechart.h"
#include "database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

MonthChartPage::MonthChartPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void MonthChartPage::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 30, 40, 30);
    mainLayout->setSpacing(20);

    // title
    QLabel *titleLabel = new QLabel("Расходы по категориям", this);
    titleLabel->setObjectName("pageTitle");
    mainLayout->addWidget(titleLabel);

    // filters
    QWidget *filterWidget = new QWidget(this);
    filterWidget->setObjectName("filterPanel");
    QHBoxLayout *filterLayout = new QHBoxLayout(filterWidget);
    filterLayout->setContentsMargins(20, 15, 20, 15);
    filterLayout->setSpacing(15);

    // period type
    QLabel *periodLabel = new QLabel("Период:", this);
    filterLayout->addWidget(periodLabel);

    m_periodTypeCombo = new QComboBox(this);
    m_periodTypeCombo->addItem("За месяц", 0);
    m_periodTypeCombo->addItem("За год", 1);
    connect(m_periodTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MonthChartPage::onPeriodTypeChanged);
    filterLayout->addWidget(m_periodTypeCombo);

    filterLayout->addSpacing(20);

    // month
    m_monthLabel = new QLabel("Месяц:", this);
    filterLayout->addWidget(m_monthLabel);

    m_monthCombo = new QComboBox(this);
    QStringList months = {"Январь", "Февраль", "Март", "Апрель", "Май", "Июнь",
                          "Июль", "Август", "Сентябрь", "Октябрь", "Ноябрь", "Декабрь"};
    for (int i = 0; i < months.size(); ++i) {
        m_monthCombo->addItem(months[i], i + 1);
    }
    m_monthCombo->setCurrentIndex(QDate::currentDate().month() - 1);
    connect(m_monthCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MonthChartPage::onFilterChanged);
    filterLayout->addWidget(m_monthCombo);

    filterLayout->addSpacing(20);

    // year
    QLabel *yearLabel = new QLabel("Год:", this);
    filterLayout->addWidget(yearLabel);

    m_yearSpin = new QSpinBox(this);
    m_yearSpin->setRange(2000, 2100);
    m_yearSpin->setValue(QDate::currentDate().year());
    connect(m_yearSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MonthChartPage::onFilterChanged);
    filterLayout->addWidget(m_yearSpin);

    filterLayout->addStretch();
    mainLayout->addWidget(filterWidget);

    // chart
    m_chart = new PieChart(this);
    m_chart->setObjectName("chartWidget");
    mainLayout->addWidget(m_chart, 1);
}

void MonthChartPage::refreshChart()
{
    updateChart();
}

void MonthChartPage::onFilterChanged()
{
    updateChart();
}

void MonthChartPage::onPeriodTypeChanged()
{
    bool isMonthly = (m_periodTypeCombo->currentData().toInt() == 0);
    m_monthLabel->setVisible(isMonthly);
    m_monthCombo->setVisible(isMonthly);

    updateChart();
}

void MonthChartPage::updateChart()
{
    int year = m_yearSpin->value();
    bool isMonthly = (m_periodTypeCombo->currentData().toInt() == 0);

    QMap<QString, double> totals;
    QString title;

    if (isMonthly) {
        int month = m_monthCombo->currentData().toInt();
        totals = Database::instance().getCategoryTotalsByMonth(year, month, Transaction::Type::Expense);
        QString monthName = m_monthCombo->currentText();
        title = QString("Расходы за %1 %2").arg(monthName).arg(year);
    } else {
        for (int month = 1; month <= 12; ++month) {
            QMap<QString, double> monthTotals = Database::instance().getCategoryTotalsByMonth(year, month, Transaction::Type::Expense);
            for (auto it = monthTotals.begin(); it != monthTotals.end(); ++it) {
                totals[it.key()] += it.value();
            }
        }
        title = QString("Расходы за %1 год").arg(year);
    }

    m_chart->setTitle(title);
    m_chart->setData(totals);
}
