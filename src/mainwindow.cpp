#include "mainwindow.h"
#include "dashboardpage.h"
#include "addtransactionpage.h"
#include "transactionlistpage.h"
#include "monthchartpage.h"
#include "yearchartpage.h"
#include "database.h"
#include "config.h"
#include <QFile>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
    loadStyleSheet();

    showDashboard();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUi()
{
    setWindowTitle("Учёт финансов");
    setMinimumSize(1200, 700);
    resize(1200, 700);
    
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    m_mainLayout = new QHBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    createNavigationPanel();
    createPages();
    
    m_mainLayout->addWidget(m_navPanel);
    m_mainLayout->addWidget(m_pageStack, 1);
}

void MainWindow::createNavigationPanel()
{
    m_navPanel = new QWidget(this);
    m_navPanel->setObjectName("navPanel");
    m_navPanel->setFixedWidth(220);
    
    m_navLayout = new QVBoxLayout(m_navPanel);
    m_navLayout->setContentsMargins(10, 20, 10, 20);
    m_navLayout->setSpacing(5);
    
    // Logo and title
    QLabel *titleLabel = new QLabel("AlexFincance", m_navPanel);
    titleLabel->setObjectName("navTitle");
    titleLabel->setAlignment(Qt::AlignCenter);
    m_navLayout->addWidget(titleLabel);
    m_navLayout->addSpacing(30);
    
    // Navigation menu
    m_btnDashboard = new QPushButton("📊 Обзор", m_navPanel);
    m_btnDashboard->setObjectName("navButton");
    m_btnDashboard->setCursor(Qt::PointingHandCursor);
    connect(m_btnDashboard, &QPushButton::clicked, this, &MainWindow::showDashboard);
    m_navLayout->addWidget(m_btnDashboard);
    
    m_btnAddTransaction = new QPushButton("➕ Добавить", m_navPanel);
    m_btnAddTransaction->setObjectName("navButton");
    m_btnAddTransaction->setCursor(Qt::PointingHandCursor);
    connect(m_btnAddTransaction, &QPushButton::clicked, this, &MainWindow::showAddTransaction);
    m_navLayout->addWidget(m_btnAddTransaction);
    
    m_btnTransactionList = new QPushButton("📋 История", m_navPanel);
    m_btnTransactionList->setObjectName("navButton");
    m_btnTransactionList->setCursor(Qt::PointingHandCursor);
    connect(m_btnTransactionList, &QPushButton::clicked, this, &MainWindow::showTransactionList);
    m_navLayout->addWidget(m_btnTransactionList);
    
    m_btnMonthChart = new QPushButton("🥧 По категориям", m_navPanel);
    m_btnMonthChart->setObjectName("navButton");
    m_btnMonthChart->setCursor(Qt::PointingHandCursor);
    connect(m_btnMonthChart, &QPushButton::clicked, this, &MainWindow::showMonthChart);
    m_navLayout->addWidget(m_btnMonthChart);
    
    m_btnYearChart = new QPushButton("📈 По месяцам", m_navPanel);
    m_btnYearChart->setObjectName("navButton");
    m_btnYearChart->setCursor(Qt::PointingHandCursor);
    connect(m_btnYearChart, &QPushButton::clicked, this, &MainWindow::showYearChart);
    m_navLayout->addWidget(m_btnYearChart);
    
    m_navLayout->addStretch();

    QLabel *versionLabel = new QLabel(QString("v%1").arg(APP_VERSION), m_navPanel);
    versionLabel->setObjectName("versionLabel");
    versionLabel->setAlignment(Qt::AlignCenter);
    m_navLayout->addWidget(versionLabel);
}

void MainWindow::createPages()
{
    m_pageStack = new QStackedWidget(this);
    m_pageStack->setObjectName("pageStack");
    
    m_dashboardPage = new DashboardPage(this);
    m_addTransactionPage = new AddTransactionPage(this);
    m_transactionListPage = new TransactionListPage(this);
    m_monthChartPage = new MonthChartPage(this);
    m_yearChartPage = new YearChartPage(this);
    
    m_pageStack->addWidget(m_dashboardPage);
    m_pageStack->addWidget(m_addTransactionPage);
    m_pageStack->addWidget(m_transactionListPage);
    m_pageStack->addWidget(m_monthChartPage);
    m_pageStack->addWidget(m_yearChartPage);
    
    // db linking
    connect(&Database::instance(), &Database::dataChanged, m_dashboardPage, &DashboardPage::refreshData);
    connect(&Database::instance(), &Database::dataChanged, m_transactionListPage, &TransactionListPage::refreshData);
    connect(&Database::instance(), &Database::dataChanged, m_monthChartPage, &MonthChartPage::refreshChart);
    connect(&Database::instance(), &Database::dataChanged, m_yearChartPage, &YearChartPage::refreshChart);
}

void MainWindow::setActiveButton(QPushButton* button)
{
    if (m_currentButton) {
        m_currentButton->setProperty("active", false);
        m_currentButton->style()->unpolish(m_currentButton);
        m_currentButton->style()->polish(m_currentButton);
    }
    
    m_currentButton = button;
    if (m_currentButton) {
        m_currentButton->setProperty("active", true);
        m_currentButton->style()->unpolish(m_currentButton);
        m_currentButton->style()->polish(m_currentButton);
    }
}

void MainWindow::showDashboard()
{
    m_pageStack->setCurrentWidget(m_dashboardPage);
    setActiveButton(m_btnDashboard);
    m_dashboardPage->refreshData();
}

void MainWindow::showAddTransaction()
{
    m_pageStack->setCurrentWidget(m_addTransactionPage);
    setActiveButton(m_btnAddTransaction);
}

void MainWindow::showTransactionList()
{
    m_pageStack->setCurrentWidget(m_transactionListPage);
    setActiveButton(m_btnTransactionList);
    m_transactionListPage->refreshData();
}

void MainWindow::showMonthChart()
{
    m_pageStack->setCurrentWidget(m_monthChartPage);
    setActiveButton(m_btnMonthChart);
    m_monthChartPage->refreshChart();
}

void MainWindow::showYearChart()
{
    m_pageStack->setCurrentWidget(m_yearChartPage);
    setActiveButton(m_btnYearChart);
    m_yearChartPage->refreshChart();
}

void MainWindow::loadStyleSheet()
{
    QFile styleFile(":/style.qss");
    if (!styleFile.exists()) {
        styleFile.setFileName("style.qss");
    }
    
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QString style = styleFile.readAll();
        qApp->setStyleSheet(style);
        styleFile.close();
    }
}
