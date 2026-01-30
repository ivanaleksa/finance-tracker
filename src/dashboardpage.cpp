#include "dashboardpage.h"
#include "database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>

DashboardPage::DashboardPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void DashboardPage::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 30, 40, 30);
    mainLayout->setSpacing(30);
    
    // title
    QLabel *titleLabel = new QLabel("Обзор финансов", this);
    titleLabel->setObjectName("pageTitle");
    mainLayout->addWidget(titleLabel);
    
    // preiod selection
    QWidget *periodWidget = new QWidget(this);
    periodWidget->setObjectName("filterPanel");
    QHBoxLayout *periodLayout = new QHBoxLayout(periodWidget);
    periodLayout->setContentsMargins(20, 15, 20, 15);
    periodLayout->setSpacing(15);
    
    QLabel *periodLabel = new QLabel("Период:", this);
    periodLayout->addWidget(periodLabel);
    
    m_monthCombo = new QComboBox(this);
    QStringList months = {"Январь", "Февраль", "Март", "Апрель", "Май", "Июнь",
                          "Июль", "Август", "Сентябрь", "Октябрь", "Ноябрь", "Декабрь"};
    for (int i = 0; i < months.size(); ++i) {
        m_monthCombo->addItem(months[i], i + 1);
    }
    m_monthCombo->setCurrentIndex(QDate::currentDate().month() - 1);
    connect(m_monthCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DashboardPage::onPeriodChanged);
    periodLayout->addWidget(m_monthCombo);
    
    m_yearSpin = new QSpinBox(this);
    m_yearSpin->setRange(2000, 2100);
    m_yearSpin->setValue(QDate::currentDate().year());
    m_yearSpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    connect(m_yearSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &DashboardPage::onPeriodChanged);
    periodLayout->addWidget(m_yearSpin);
    
    periodLayout->addStretch();
    mainLayout->addWidget(periodWidget);
    
    // statistic
    QGridLayout *cardsLayout = new QGridLayout();
    cardsLayout->setSpacing(20);
    
    // income per month
    QWidget *incomeCard = createStatCard("Доходы за месяц", m_incomeValue, "#27ae60");
    cardsLayout->addWidget(incomeCard, 0, 0);
    
    // exceses per month
    QWidget *expenseCard = createStatCard("Расходы за месяц", m_expenseValue, "#e74c3c");
    cardsLayout->addWidget(expenseCard, 0, 1);
    
    // month balance
    QWidget *balanceCard = createStatCard("Баланс месяца", m_balanceValue, "#3498db");
    cardsLayout->addWidget(balanceCard, 0, 2);

    // savings
    QWidget *savingsCard = createStatCard("Сбережения за месяц", m_savingsValue, "#8e44ad");
    cardsLayout->addWidget(savingsCard, 1, 1);
    
    // residual
    QWidget *carryoverCard = createStatCard("Остаток с прошлых месяцев", m_carryoverValue, "#9b59b6");
    cardsLayout->addWidget(carryoverCard, 1, 0);
    
    // total balance
    QWidget *totalCard = createStatCard("Общий баланс", m_totalBalanceValue, "#f39c12");
    cardsLayout->addWidget(totalCard, 1, 2);
    
    mainLayout->addLayout(cardsLayout);
    mainLayout->addStretch();
}

QWidget* DashboardPage::createStatCard(const QString& title, QLabel*& valueLabel, const QString& color)
{
    QFrame *card = new QFrame(this);
    card->setObjectName("statCard");
    card->setStyleSheet(QString("QFrame#statCard { border-left: 4px solid %1; }").arg(color));
    
    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(25, 20, 25, 20);
    layout->setSpacing(10);
    
    QLabel *titleLabel = new QLabel(title, card);
    titleLabel->setObjectName("statTitle");
    layout->addWidget(titleLabel);
    
    valueLabel = new QLabel("0.00 ₽", card);
    valueLabel->setObjectName("statValue");
    valueLabel->setStyleSheet(QString("color: %1;").arg(color));
    layout->addWidget(valueLabel);
    
    return card;
}

void DashboardPage::refreshData()
{
    updateStatistics();
}

void DashboardPage::onPeriodChanged()
{
    updateStatistics();
}

void DashboardPage::updateStatistics()
{
    int year = m_yearSpin->value();
    int month = m_monthCombo->currentData().toInt();

    double income = Database::instance().getTotalByMonth(year, month, Transaction::Type::Income);
    double expense = Database::instance().getTotalByMonth(year, month, Transaction::Type::Expense);
    double savings = Database::instance().getSavingsByMonth(year, month);
    double balance = income - expense - savings;
    double carryover = Database::instance().getBalanceUpToMonth(year, month);
    double totalBalance = carryover + balance;

    m_incomeValue->setText(QString("%1 ₽").arg(income, 0, 'f', 2));
    m_expenseValue->setText(QString("%1 ₽").arg(expense, 0, 'f', 2));
    m_savingsValue->setText(QString("%1 ₽").arg(savings, 0, 'f', 2));

    m_balanceValue->setText(QString("%1%2 ₽")
                                .arg(balance >= 0 ? "+" : "")
                                .arg(balance, 0, 'f', 2));
    m_balanceValue->setStyleSheet(QString("color: %1; font-size: 28px; font-weight: bold;")
                                      .arg(balance >= 0 ? "#27ae60" : "#e74c3c"));

    m_carryoverValue->setText(QString("%1%2 ₽")
                                  .arg(carryover >= 0 ? "+" : "")
                                  .arg(carryover, 0, 'f', 2));
    m_carryoverValue->setStyleSheet(QString("color: %1; font-size: 28px; font-weight: bold;")
                                        .arg(carryover >= 0 ? "#9b59b6" : "#e74c3c"));

    m_totalBalanceValue->setText(QString("%1%2 ₽")
                                     .arg(totalBalance >= 0 ? "+" : "")
                                     .arg(totalBalance, 0, 'f', 2));
    m_totalBalanceValue->setStyleSheet(QString("color: %1; font-size: 32px; font-weight: bold;")
                                           .arg(totalBalance >= 0 ? "#f39c12" : "#e74c3c"));
}
