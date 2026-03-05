#include "reporting/monthchartpage.h"
#include "widgets/piechart.h"
#include "database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

MonthChartPage::MonthChartPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    refreshCategories();

    connect(&Database::instance(), &Database::categoryAdded, this, &MonthChartPage::refreshCategories);
    connect(&Database::instance(), &Database::categoryDeleted, this, &MonthChartPage::refreshCategories);
    connect(&Database::instance(), &Database::dataChanged, this, &MonthChartPage::refreshCategories);
}

void MonthChartPage::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 30, 40, 30);
    mainLayout->setSpacing(20);

    // Заголовок
    QLabel *titleLabel = new QLabel("Расходы по категориям", this);
    titleLabel->setObjectName("pageTitle");
    mainLayout->addWidget(titleLabel);

    // Фильтры
    QWidget *filterWidget = new QWidget(this);
    filterWidget->setObjectName("filterPanel");
    QHBoxLayout *filterLayout = new QHBoxLayout(filterWidget);
    filterLayout->setContentsMargins(20, 15, 20, 15);
    filterLayout->setSpacing(15);

    // Тип периода
    QLabel *periodLabel = new QLabel("Период:", this);
    filterLayout->addWidget(periodLabel);

    m_periodTypeCombo = new QComboBox(this);
    m_periodTypeCombo->addItem("За месяц", 0);
    m_periodTypeCombo->addItem("За год", 1);
    connect(m_periodTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MonthChartPage::onPeriodTypeChanged);
    filterLayout->addWidget(m_periodTypeCombo);

    filterLayout->addSpacing(20);

    // Месяц
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

    // Год
    QLabel *yearLabel = new QLabel("Год:", this);
    filterLayout->addWidget(yearLabel);

    m_yearSpin = new QSpinBox(this);
    m_yearSpin->setRange(2000, 2100);
    m_yearSpin->setValue(QDate::currentDate().year());
    m_yearSpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    connect(m_yearSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MonthChartPage::onFilterChanged);
    filterLayout->addWidget(m_yearSpin);

    filterLayout->addSpacing(20);

    // Категория
    m_categoryLabel = new QLabel("Категория:", this);
    filterLayout->addWidget(m_categoryLabel);

    m_categoryCombo = new QComboBox(this);
    connect(m_categoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MonthChartPage::onCategoryChanged);
    filterLayout->addWidget(m_categoryCombo);

    filterLayout->addStretch();
    mainLayout->addWidget(filterWidget);

    // График
    m_chart = new PieChart(this);
    m_chart->setObjectName("chartWidget");
    mainLayout->addWidget(m_chart, 1);
}

void MonthChartPage::refreshCategories()
{
    int currentCategoryId = m_categoryCombo->currentData().toInt();

    m_categoryCombo->clear();
    m_categoryCombo->addItem("Все категории", -1);

    QList<Category> categories = Database::instance().getCategories();
    for (const Category& cat : categories) {
        m_categoryCombo->addItem(cat.name(), cat.id());
    }

    // Восстанавливаем выбор если категория ещё существует
    int index = m_categoryCombo->findData(currentCategoryId);
    if (index >= 0) {
        m_categoryCombo->setCurrentIndex(index);
    } else {
        m_categoryCombo->setCurrentIndex(0);
    }
}

void MonthChartPage::refreshChart()
{
    refreshCategories();
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

void MonthChartPage::onCategoryChanged()
{
    updateChart();
}

void MonthChartPage::updateChart()
{
    int year = m_yearSpin->value();
    bool isMonthly = (m_periodTypeCombo->currentData().toInt() == 0);
    int selectedCategoryId = m_categoryCombo->currentData().toInt();

    QMap<QString, double> totals;
    QString title;
    QString periodStr;

    // Формируем строку периода
    if (isMonthly) {
        QString monthName = m_monthCombo->currentText();
        periodStr = QString("%1 %2").arg(monthName).arg(year);
    } else {
        periodStr = QString("%1 год").arg(year);
    }

    if (selectedCategoryId < 0) {
        // Показываем все категории
        if (isMonthly) {
            int month = m_monthCombo->currentData().toInt();
            totals = Database::instance().getCategoryTotalsByMonth(year, month, Transaction::Type::Expense);
        } else {
            for (int month = 1; month <= 12; ++month) {
                QMap<QString, double> monthTotals = Database::instance().getCategoryTotalsByMonth(year, month, Transaction::Type::Expense);
                for (auto it = monthTotals.begin(); it != monthTotals.end(); ++it) {
                    totals[it.key()] += it.value();
                }
            }
        }
        title = QString("Расходы за %1").arg(periodStr);
    } else {
        // Показываем подкатегории выбранной категории
        QString categoryName = m_categoryCombo->currentText();

        if (isMonthly) {
            int month = m_monthCombo->currentData().toInt();
            totals = Database::instance().getSubcategoryTotalsByMonth(year, month, selectedCategoryId);
        } else {
            for (int month = 1; month <= 12; ++month) {
                QMap<QString, double> monthTotals = Database::instance().getSubcategoryTotalsByMonth(year, month, selectedCategoryId);
                for (auto it = monthTotals.begin(); it != monthTotals.end(); ++it) {
                    totals[it.key()] += it.value();
                }
            }
        }

        if (totals.isEmpty()) {
            // Проверяем, есть ли вообще подкатегории у этой категории
            QList<Category> subcats = Database::instance().getSubcategories(selectedCategoryId);
            if (subcats.isEmpty()) {
                m_chart->setTitle(QString("%1 — %2").arg(categoryName, periodStr));
                m_chart->setNoDataMessage("У этой категории нет подкатегорий");
                m_chart->clear();
                return;
            }
        }

        title = QString("%1 — %2").arg(categoryName, periodStr);
    }

    m_chart->setTitle(title);
    m_chart->setNoDataMessage("Нет данных за выбранный период");
    m_chart->setData(totals);
}
