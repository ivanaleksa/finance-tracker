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
    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &YearChartPage::onFilterChanged);
    filterLayout->addWidget(m_typeCombo);
    
    filterLayout->addSpacing(20);
    
    QLabel *yearLabel = new QLabel("Год:", this);
    filterLayout->addWidget(yearLabel);
    
    m_yearSpin = new QSpinBox(this);
    m_yearSpin->setRange(2000, 2100);
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
    
    QMap<int, double> monthlyTotals = Database::instance().getMonthlyTotals(year, type);
    
    QString typeStr = type == Transaction::Type::Income ? "Доходы" : "Расходы";
    m_chart->setTitle(QString("%1 за %2 год").arg(typeStr, QString::number(year)));
    m_chart->setBarColor(type == Transaction::Type::Income ? QColor("#27ae60") : QColor("#e74c3c"));
    m_chart->setData(monthlyTotals);
}
