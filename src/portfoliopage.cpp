#include "portfoliopage.h"
#include "addassetdialog.h"
#include "assetdetailsdialog.h"
#include "createsnapshotdialog.h"
#include "database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>

PortfolioPage::PortfolioPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    loadCurrencyRates();
    loadAssets();

    connect(&Database::instance(), &Database::portfolioDataChanged,
            this, &PortfolioPage::refreshData);
    connect(&Database::instance(), &Database::investmentDataChanged,
            this, &PortfolioPage::refreshData);
}

void PortfolioPage::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 30, 40, 30);
    mainLayout->setSpacing(20);

    // Title
    QLabel *titleLabel = new QLabel("Живой портфель", this);
    titleLabel->setObjectName("pageTitle");
    mainLayout->addWidget(titleLabel);

    // Action buttons row
    QHBoxLayout *actionLayout = new QHBoxLayout();

    m_addAssetBtn = new QPushButton("Добавить актив", this);
    m_addAssetBtn->setObjectName("primaryButton");
    m_addAssetBtn->setCursor(Qt::PointingHandCursor);
    connect(m_addAssetBtn, &QPushButton::clicked, this, &PortfolioPage::onAddAssetClicked);
    actionLayout->addWidget(m_addAssetBtn);

    m_createSnapshotBtn = new QPushButton("Сделать снимок", this);
    m_createSnapshotBtn->setCursor(Qt::PointingHandCursor);
    connect(m_createSnapshotBtn, &QPushButton::clicked, this, &PortfolioPage::onCreateSnapshotClicked);
    actionLayout->addWidget(m_createSnapshotBtn);

    actionLayout->addStretch();

    // Currencies button
    m_currenciesBtn = new QPushButton("$ ₽", this);
    m_currenciesBtn->setObjectName("currenciesButton");
    m_currenciesBtn->setCursor(Qt::PointingHandCursor);
    m_currenciesBtn->setToolTip("Управление валютами");
    m_currenciesBtn->setStyleSheet(
        "QPushButton { "
        "   border: 2px solid #3498db; "
        "   border-radius: 8px; "
        "   background: transparent; "
        "   color: #3498db; "
        "   font-size: 16px; "
        "   font-weight: bold; "
        "   padding: 8px 16px; "
        "} "
        "QPushButton:hover { "
        "   background: #3498db; "
        "   color: white; "
        "}"
    );
    connect(m_currenciesBtn, &QPushButton::clicked, this, &PortfolioPage::onCurrenciesClicked);
    actionLayout->addWidget(m_currenciesBtn);

    mainLayout->addLayout(actionLayout);

    // Assets table
    m_assetsTable = new QTableWidget(this);
    m_assetsTable->setObjectName("transactionTable");
    m_assetsTable->setColumnCount(8);
    m_assetsTable->setHorizontalHeaderLabels({
        "Название", "Категория", "Кол-во", "Ср. цена", "Тек. цена", "Стоимость", "Доходность %", "Прибыль"
    });
    m_assetsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_assetsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_assetsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_assetsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_assetsTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_assetsTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    m_assetsTable->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
    m_assetsTable->horizontalHeader()->setSectionResizeMode(7, QHeaderView::ResizeToContents);
    m_assetsTable->verticalHeader()->setVisible(false);
    m_assetsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_assetsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_assetsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_assetsTable->setAlternatingRowColors(true);
    m_assetsTable->verticalHeader()->setDefaultSectionSize(40);

    connect(m_assetsTable, &QTableWidget::cellDoubleClicked,
            this, &PortfolioPage::onAssetDoubleClicked);

    mainLayout->addWidget(m_assetsTable, 1);

    // Totals section
    QWidget *totalsWidget = new QWidget(this);
    totalsWidget->setObjectName("formGroup");
    QHBoxLayout *totalsLayout = new QHBoxLayout(totalsWidget);
    totalsLayout->setContentsMargins(20, 15, 20, 15);
    totalsLayout->setSpacing(30);

    totalsLayout->addWidget(new QLabel("Итого:", this));

    m_totalValueLabel = new QLabel("0.00 ₽", this);
    m_totalValueLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    totalsLayout->addWidget(new QLabel("Стоимость:", this));
    totalsLayout->addWidget(m_totalValueLabel);

    m_totalProfitLabel = new QLabel("0.00 ₽", this);
    m_totalProfitLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    totalsLayout->addWidget(new QLabel("Прибыль:", this));
    totalsLayout->addWidget(m_totalProfitLabel);

    m_totalYieldLabel = new QLabel("0.00%", this);
    m_totalYieldLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    totalsLayout->addWidget(new QLabel("Доходность:", this));
    totalsLayout->addWidget(m_totalYieldLabel);

    totalsLayout->addStretch();
    mainLayout->addWidget(totalsWidget);
}

void PortfolioPage::refreshData()
{
    loadAssets();
    updateTotals();
}

void PortfolioPage::loadCurrencyRates()
{
    m_currencyRates.clear();

    // Get latest snapshot rates as default
    Snapshot latestSnapshot = Database::instance().getLatestSnapshot();
    m_currencyRates = latestSnapshot.currencyRates();

    // Ensure all currencies have a rate
    QList<Currency> currencies = Database::instance().getCurrencies();
    for (const Currency& curr : currencies) {
        if (curr.code() == "RUB") {
            m_currencyRates[curr.id()] = 1.0;
        } else if (!m_currencyRates.contains(curr.id())) {
            m_currencyRates[curr.id()] = 100.0;
        }
    }
}

void PortfolioPage::setCurrencyRates(const QMap<int, double>& rates)
{
    m_currencyRates = rates;
    loadAssets();
    updateTotals();
}

void PortfolioPage::loadAssets()
{
    m_assetsTable->setRowCount(0);

    QList<PortfolioAsset> assets = Database::instance().getActivePortfolioAssets();

    int visibleRow = 0;
    for (const PortfolioAsset& assetConst : assets) {
        PortfolioAsset asset = assetConst;

        // Skip assets with zero quantity
        if (asset.totalQuantity() <= 0) {
            continue;
        }

        // Set currency rate from our cache
        double rate = m_currencyRates.value(asset.currencyId(), 1.0);
        asset.setCurrencyRate(rate);

        m_assetsTable->setRowCount(visibleRow + 1);

        QTableWidgetItem *nameItem = new QTableWidgetItem(asset.name());
        nameItem->setData(Qt::UserRole, asset.id());
        m_assetsTable->setItem(visibleRow, 0, nameItem);

        QString categoryText = asset.categoryName().isEmpty() ? "—" : asset.categoryName();
        m_assetsTable->setItem(visibleRow, 1, new QTableWidgetItem(categoryText));

        QTableWidgetItem *qtyItem = new QTableWidgetItem(QString::number(asset.totalQuantity(), 'f', 4));
        qtyItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_assetsTable->setItem(visibleRow, 2, qtyItem);

        QString avgPriceStr = QString("%1 %2").arg(asset.averageBuyPrice(), 0, 'f', 2).arg(asset.currencyCode());
        QTableWidgetItem *avgPriceItem = new QTableWidgetItem(avgPriceStr);
        avgPriceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_assetsTable->setItem(visibleRow, 3, avgPriceItem);

        QString currPriceStr = QString("%1 %2").arg(asset.currentPrice(), 0, 'f', 2).arg(asset.currencyCode());
        QTableWidgetItem *currPriceItem = new QTableWidgetItem(currPriceStr);
        currPriceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_assetsTable->setItem(visibleRow, 4, currPriceItem);

        QString valueStr = QString("%1 ₽").arg(asset.currentValueInRub(), 0, 'f', 2);
        QTableWidgetItem *valueItem = new QTableWidgetItem(valueStr);
        valueItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_assetsTable->setItem(visibleRow, 5, valueItem);

        double yield = asset.yieldPercent();
        QString yieldStr = QString("%1%2%").arg(yield >= 0 ? "+" : "").arg(yield, 0, 'f', 2);
        QTableWidgetItem *yieldItem = new QTableWidgetItem(yieldStr);
        yieldItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        yieldItem->setForeground(yield >= 0 ? QColor("#27ae60") : QColor("#e74c3c"));
        m_assetsTable->setItem(visibleRow, 6, yieldItem);

        double profit = asset.profitInRub();
        QString profitStr = QString("%1%2 ₽").arg(profit >= 0 ? "+" : "").arg(profit, 0, 'f', 2);
        QTableWidgetItem *profitItem = new QTableWidgetItem(profitStr);
        profitItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        profitItem->setForeground(profit >= 0 ? QColor("#27ae60") : QColor("#e74c3c"));
        m_assetsTable->setItem(visibleRow, 7, profitItem);

        visibleRow++;
    }

    updateTotals();
}

void PortfolioPage::updateTotals()
{
    QList<PortfolioAsset> assets = Database::instance().getActivePortfolioAssets();

    double totalValue = 0.0;
    double totalInvested = 0.0;

    for (const PortfolioAsset& asset : assets) {
        if (asset.totalQuantity() <= 0) continue;

        double rate = m_currencyRates.value(asset.currencyId(), 1.0);
        totalValue += asset.currentValue() * rate;
        totalInvested += asset.totalInvested() * rate;
    }

    double totalProfit = totalValue - totalInvested;
    double totalYield = totalInvested > 0 ? (totalProfit / totalInvested) * 100.0 : 0.0;

    m_totalValueLabel->setText(QString("%1 ₽").arg(totalValue, 0, 'f', 2));

    QString profitColor = totalProfit >= 0 ? "#27ae60" : "#e74c3c";
    m_totalProfitLabel->setText(QString("<span style='color: %1;'>%2%3 ₽</span>")
                               .arg(profitColor)
                               .arg(totalProfit >= 0 ? "+" : "")
                               .arg(totalProfit, 0, 'f', 2));

    QString yieldColor = totalYield >= 0 ? "#27ae60" : "#e74c3c";
    m_totalYieldLabel->setText(QString("<span style='color: %1;'>%2%3%</span>")
                              .arg(yieldColor)
                              .arg(totalYield >= 0 ? "+" : "")
                              .arg(totalYield, 0, 'f', 2));
}

void PortfolioPage::onAddAssetClicked()
{
    AddAssetDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        PortfolioAsset asset = dialog.getAsset();
        AssetOperation operation = dialog.getFirstOperation();

        if (Database::instance().addPortfolioAsset(asset)) {
            operation.setAssetId(asset.id());
            if (!Database::instance().addAssetOperation(operation)) {
                QMessageBox::warning(this, "Предупреждение",
                    "Актив добавлен, но не удалось добавить первую операцию");
            }
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось добавить актив");
        }
    }
}

void PortfolioPage::onCreateSnapshotClicked()
{
    CreateSnapshotDialog dialog(m_currencyRates, this);
    dialog.exec();
}

void PortfolioPage::onCurrenciesClicked()
{
    emit currenciesPageRequested();
}

void PortfolioPage::onAssetDoubleClicked(int row, int column)
{
    Q_UNUSED(column);

    QTableWidgetItem *item = m_assetsTable->item(row, 0);
    if (!item) return;

    int assetId = item->data(Qt::UserRole).toInt();
    AssetDetailsDialog dialog(assetId, this);
    dialog.exec();
}
