#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

class DashboardPage;
class AddTransactionPage;
class TransactionListPage;
class MonthChartPage;
class YearChartPage;
class WithdrawalsPage;
class DepositsPage;
class PortfolioPage;
class CurrenciesPage;
class SnapshotsHistoryPage;
class InvestmentDashboardPage;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum AppMode {
        ReportingMode,
        InvestmentsMode
    };

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showDashboard();
    void showAddTransaction();
    void showTransactionList();
    void showMonthChart();
    void showYearChart();

    // Investments mode slots
    void showInvestmentDashboard();
    void showPortfolio();
    void showCurrencies();
    void showWithdrawals();
    void showDeposits();
    void showSnapshotsHistory();
    void onCurrencyRatesChanged();

    void switchToReportingMode();
    void switchToInvestmentsMode();

private:
    void setupUi();
    void createNavigationPanel();
    void createModeSwitcher();
    void createReportingButtons();
    void createInvestmentsButtons();
    void createPages();
    void setActiveButton(QPushButton* button);
    void setActiveMode(AppMode mode);
    void loadStyleSheet();

    QWidget *m_centralWidget;
    QHBoxLayout *m_mainLayout;
    QWidget *m_navPanel;
    QVBoxLayout *m_navLayout;
    QStackedWidget *m_pageStack;

    // Mode switcher
    QWidget *m_modeSwitcher;
    QPushButton *m_btnReportingMode;
    QPushButton *m_btnInvestmentsMode;
    AppMode m_currentMode = ReportingMode;

    // Navigation button containers
    QWidget *m_reportingButtonsContainer;
    QWidget *m_investmentsButtonsContainer;

    // Reporting mode buttons
    QPushButton *m_btnDashboard;
    QPushButton *m_btnAddTransaction;
    QPushButton *m_btnTransactionList;
    QPushButton *m_btnMonthChart;
    QPushButton *m_btnYearChart;
    QPushButton *m_currentButton = nullptr;

    // Investments mode buttons
    QPushButton *m_btnInvestmentDashboard;
    QPushButton *m_btnPortfolio;
    QPushButton *m_btnSnapshotsHistory;
    QPushButton *m_btnWithdrawals;
    QPushButton *m_btnDeposits;

    // Reporting pages
    DashboardPage *m_dashboardPage;
    AddTransactionPage *m_addTransactionPage;
    TransactionListPage *m_transactionListPage;
    MonthChartPage *m_monthChartPage;
    YearChartPage *m_yearChartPage;

    // Investments pages
    InvestmentDashboardPage *m_investmentDashboardPage;
    PortfolioPage *m_portfolioPage;
    CurrenciesPage *m_currenciesPage;
    SnapshotsHistoryPage *m_snapshotsHistoryPage;
    WithdrawalsPage *m_withdrawalsPage;
    DepositsPage *m_depositsPage;
};

#endif // MAINWINDOW_H
