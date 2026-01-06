#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QVBoxLayout>

class DashboardPage;
class AddTransactionPage;
class TransactionListPage;
class MonthChartPage;
class YearChartPage;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showDashboard();
    void showAddTransaction();
    void showTransactionList();
    void showMonthChart();
    void showYearChart();

private:
    void setupUi();
    void createNavigationPanel();
    void createPages();
    void setActiveButton(QPushButton* button);
    void loadStyleSheet();

    QWidget *m_centralWidget;
    QHBoxLayout *m_mainLayout;
    QWidget *m_navPanel;
    QVBoxLayout *m_navLayout;
    QStackedWidget *m_pageStack;

    QPushButton *m_btnDashboard;
    QPushButton *m_btnAddTransaction;
    QPushButton *m_btnTransactionList;
    QPushButton *m_btnMonthChart;
    QPushButton *m_btnYearChart;
    QPushButton *m_currentButton = nullptr;

    DashboardPage *m_dashboardPage;
    AddTransactionPage *m_addTransactionPage;
    TransactionListPage *m_transactionListPage;
    MonthChartPage *m_monthChartPage;
    YearChartPage *m_yearChartPage;
};

#endif // MAINWINDOW_H
