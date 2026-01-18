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

    switchToReportingMode();
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
    m_navLayout->addSpacing(15);

    // Mode switcher
    createModeSwitcher();
    m_navLayout->addWidget(m_modeSwitcher);
    m_navLayout->addSpacing(20);

    // Create button containers for both modes
    createReportingButtons();
    createInvestmentsButtons();

    m_navLayout->addWidget(m_reportingButtonsContainer);
    m_navLayout->addWidget(m_investmentsButtonsContainer);

    m_navLayout->addStretch();

    QLabel *versionLabel = new QLabel(QString("v%1").arg(APP_VERSION), m_navPanel);
    versionLabel->setObjectName("versionLabel");
    versionLabel->setAlignment(Qt::AlignCenter);
    m_navLayout->addWidget(versionLabel);
}

void MainWindow::createModeSwitcher()
{
    m_modeSwitcher = new QWidget(m_navPanel);
    m_modeSwitcher->setObjectName("modeSwitcher");

    QHBoxLayout *switcherLayout = new QHBoxLayout(m_modeSwitcher);
    switcherLayout->setContentsMargins(4, 4, 4, 4);
    switcherLayout->setSpacing(0);

    m_btnReportingMode = new QPushButton("Отчётность", m_modeSwitcher);
    m_btnReportingMode->setObjectName("modeButton");
    m_btnReportingMode->setCursor(Qt::PointingHandCursor);
    m_btnReportingMode->setCheckable(true);
    connect(m_btnReportingMode, &QPushButton::clicked, this, &MainWindow::switchToReportingMode);

    m_btnInvestmentsMode = new QPushButton("Инвестиции", m_modeSwitcher);
    m_btnInvestmentsMode->setObjectName("modeButton");
    m_btnInvestmentsMode->setCursor(Qt::PointingHandCursor);
    m_btnInvestmentsMode->setCheckable(true);
    connect(m_btnInvestmentsMode, &QPushButton::clicked, this, &MainWindow::switchToInvestmentsMode);

    switcherLayout->addWidget(m_btnReportingMode);
    switcherLayout->addWidget(m_btnInvestmentsMode);
}

void MainWindow::createReportingButtons()
{
    m_reportingButtonsContainer = new QWidget(m_navPanel);
    QVBoxLayout *layout = new QVBoxLayout(m_reportingButtonsContainer);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);

    m_btnDashboard = new QPushButton("📊 Обзор", m_reportingButtonsContainer);
    m_btnDashboard->setObjectName("navButton");
    m_btnDashboard->setCursor(Qt::PointingHandCursor);
    connect(m_btnDashboard, &QPushButton::clicked, this, &MainWindow::showDashboard);
    layout->addWidget(m_btnDashboard);

    m_btnAddTransaction = new QPushButton("➕ Добавить", m_reportingButtonsContainer);
    m_btnAddTransaction->setObjectName("navButton");
    m_btnAddTransaction->setCursor(Qt::PointingHandCursor);
    connect(m_btnAddTransaction, &QPushButton::clicked, this, &MainWindow::showAddTransaction);
    layout->addWidget(m_btnAddTransaction);

    m_btnTransactionList = new QPushButton("📋 История", m_reportingButtonsContainer);
    m_btnTransactionList->setObjectName("navButton");
    m_btnTransactionList->setCursor(Qt::PointingHandCursor);
    connect(m_btnTransactionList, &QPushButton::clicked, this, &MainWindow::showTransactionList);
    layout->addWidget(m_btnTransactionList);

    m_btnMonthChart = new QPushButton("🥧 По категориям", m_reportingButtonsContainer);
    m_btnMonthChart->setObjectName("navButton");
    m_btnMonthChart->setCursor(Qt::PointingHandCursor);
    connect(m_btnMonthChart, &QPushButton::clicked, this, &MainWindow::showMonthChart);
    layout->addWidget(m_btnMonthChart);

    m_btnYearChart = new QPushButton("📈 По месяцам", m_reportingButtonsContainer);
    m_btnYearChart->setObjectName("navButton");
    m_btnYearChart->setCursor(Qt::PointingHandCursor);
    connect(m_btnYearChart, &QPushButton::clicked, this, &MainWindow::showYearChart);
    layout->addWidget(m_btnYearChart);
}

void MainWindow::createInvestmentsButtons()
{
    m_investmentsButtonsContainer = new QWidget(m_navPanel);
    QVBoxLayout *layout = new QVBoxLayout(m_investmentsButtonsContainer);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);

    m_btnInvestmentsPlaceholder = new QPushButton("💼 Портфель", m_investmentsButtonsContainer);
    m_btnInvestmentsPlaceholder->setObjectName("navButton");
    m_btnInvestmentsPlaceholder->setCursor(Qt::PointingHandCursor);
    connect(m_btnInvestmentsPlaceholder, &QPushButton::clicked, this, &MainWindow::showInvestmentsPlaceholder);
    layout->addWidget(m_btnInvestmentsPlaceholder);

    // Hide by default
    m_investmentsButtonsContainer->hide();
}

void MainWindow::createPages()
{
    m_pageStack = new QStackedWidget(this);
    m_pageStack->setObjectName("pageStack");

    // Reporting pages
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

    // Investments placeholder page
    m_investmentsPlaceholderPage = new QWidget(this);
    m_investmentsPlaceholderPage->setObjectName("pageContent");
    QVBoxLayout *placeholderLayout = new QVBoxLayout(m_investmentsPlaceholderPage);
    placeholderLayout->setAlignment(Qt::AlignCenter);

    QLabel *placeholderLabel = new QLabel("Раздел в разработке", m_investmentsPlaceholderPage);
    placeholderLabel->setObjectName("placeholderText");
    placeholderLabel->setAlignment(Qt::AlignCenter);
    placeholderLayout->addWidget(placeholderLabel);

    m_pageStack->addWidget(m_investmentsPlaceholderPage);

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

void MainWindow::setActiveMode(AppMode mode)
{
    m_currentMode = mode;

    // Update mode buttons appearance
    m_btnReportingMode->setChecked(mode == ReportingMode);
    m_btnInvestmentsMode->setChecked(mode == InvestmentsMode);

    // Repolish to apply styles
    m_btnReportingMode->style()->unpolish(m_btnReportingMode);
    m_btnReportingMode->style()->polish(m_btnReportingMode);
    m_btnInvestmentsMode->style()->unpolish(m_btnInvestmentsMode);
    m_btnInvestmentsMode->style()->polish(m_btnInvestmentsMode);

    // Show/hide appropriate button containers
    m_reportingButtonsContainer->setVisible(mode == ReportingMode);
    m_investmentsButtonsContainer->setVisible(mode == InvestmentsMode);
}

void MainWindow::switchToReportingMode()
{
    setActiveMode(ReportingMode);
    showDashboard();
}

void MainWindow::switchToInvestmentsMode()
{
    setActiveMode(InvestmentsMode);
    showInvestmentsPlaceholder();
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

void MainWindow::showInvestmentsPlaceholder()
{
    m_pageStack->setCurrentWidget(m_investmentsPlaceholderPage);
    setActiveButton(m_btnInvestmentsPlaceholder);
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
