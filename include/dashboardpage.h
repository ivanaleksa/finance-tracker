#ifndef DASHBOARDPAGE_H
#define DASHBOARDPAGE_H

#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>

class DashboardPage : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardPage(QWidget *parent = nullptr);

public slots:
    void refreshData();

private slots:
    void onPeriodChanged();

private:
    void setupUi();
    void updateStatistics();
    QWidget* createStatCard(const QString& title, QLabel*& valueLabel, const QString& color);

    QComboBox *m_monthCombo;
    QSpinBox *m_yearSpin;
    
    QLabel *m_incomeValue;
    QLabel *m_expenseValue;
    QLabel *m_balanceValue;
    QLabel *m_carryoverValue;
    QLabel *m_totalBalanceValue;
    QLabel *m_savingsValue;
};

#endif // DASHBOARDPAGE_H
