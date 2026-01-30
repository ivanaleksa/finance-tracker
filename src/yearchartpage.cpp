#include "yearchartpage.h"
#include "barchart.h"
#include "database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

YearChartPage::YearChartPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void YearChartPage::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 30, 40, 30);
    mainLayout->setSpacing(20);
    
    // title
    QLabel *titleLabel = new QLabel("Статистика по месяцам", this);
    titleLabel->setObjectName("pageTitle");
    mainLayout->addWidget(titleLabel);
    
    // filters
    QWidget *filterWidget = new QWidget(this);
    filterWidget->setObjectName("filterPanel");
    QHBoxLayout *filterLayout = new QHBoxLayout(filterWidget);
    filterLayout->setContentsMargins(20, 15, 20, 15);
    filterLayout->setSpacing(15);
    
    QLabel *typeLabel = new QLabel("Тип:", this);
    filterLayout->addWidget(typeLabel);
    
    m_typeCombo = new QComboBox(this);
    m_typeCombo->addItem("Расходы", static_cast<int>(Transaction::Type::Expense));
    m_typeCombo->addItem("Доходы", static_cast<int>(Transaction::Type::Income));
    m_typeCombo->addItem("Сбережения", static_cast<int>(Transaction::Type::Savings));
    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &YearChartPage::onFilterChanged);
    filterLayout->addWidget(m_typeCombo);
    
    filterLayout->addSpacing(20);
    
    QLabel *yearLabel = new QLabel("Год:", this);
    filterLayout->addWidget(yearLabel);
    
    m_yearSpin = new QSpinBox(this);
    m_yearSpin->setRange(2000, 2100);
    m_yearSpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_yearSpin->setValue(QDate::currentDate().year());
    connect(m_yearSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &YearChartPage::onFilterChanged);
    filterLayout->addWidget(m_yearSpin);
    
    filterLayout->addStretch();
    mainLayout->addWidget(filterWidget);
    
    // charts
    m_chart = new BarChart(this);
    m_chart->setObjectName("chartWidget");
    mainLayout->addWidget(m_chart, 1);
}

void YearChartPage::refreshChart()
{
    updateChart();
}

void YearChartPage::onFilterChanged()
{
    updateChart();
}

void YearChartPage::updateChart()
{
    int year = m_yearSpin->value();
    Transaction::Type type = static_cast<Transaction::Type>(m_typeCombo->currentData().toInt());

    QMap<int, double> monthlyTotals;

    if (type == Transaction::Type::Savings) {
        monthlyTotals = Database::instance().getMonthlySavings(year);
    } else {
        monthlyTotals = Database::instance().getMonthlyTotals(year, type);
    }

    QString typeStr;
    QColor barColor;
    switch (type) {
    case Transaction::Type::Income:
        typeStr = "Доходы";
        barColor = QColor("#27ae60");
        break;
    case Transaction::Type::Savings:
        typeStr = "Сбережения";
        barColor = QColor("#8e44ad");
        break;
    default:
        typeStr = "Расходы";
        barColor = QColor("#e74c3c");
        break;
    }

    m_chart->setTitle(QString("%1 за %2 год").arg(typeStr, QString::number(year)));
    m_chart->setBarColor(barColor);
    m_chart->setData(monthlyTotals);
}
