#include "portfoliopage.h"
#include "addassetdialog.h"
#include "assetdetailsdialog.h"
#include "createsnapshotdialog.h"
#include "database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QGroupBox>
#include <QDoubleSpinBox>

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

    // Currency rates section
    QGroupBox *currencyGroup = new QGroupBox("Курсы валют", this);
    QVBoxLayout *currencyLayout = new QVBoxLayout(currencyGroup);

    m_currencyTable = new QTableWidget(this);
    m_currencyTable->setColumnCount(3);
    m_currencyTable->setHorizontalHeaderLabels({"Валюта", "Название", "Курс к RUB"});
    m_currencyTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_currencyTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_currencyTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_currencyTable->verticalHeader()->setVisible(false);
    m_currencyTable->setSelectionMode(QAbstractItemView::NoSelection);
    m_currencyTable->setAlternatingRowColors(true);
    m_currencyTable->setMaximumHeight(120);

    currencyLayout->addWidget(m_currencyTable);
    mainLayout->addWidget(currencyGroup);

    // Action buttons
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
    m_currencyTable->setRowCount(0);
    m_currencyRates.clear();

    // Get latest snapshot rates as default
    Snapshot latestSnapshot = Database::instance().getLatestSnapshot();
    QMap<int, double> snapshotRates = latestSnapshot.currencyRates();

    QList<Currency> currencies = Database::instance().getCurrencies();
    m_currencyTable->setRowCount(currencies.size());

    for (int i = 0; i < currencies.size(); ++i) {
        const Currency& curr = currencies[i];

        QTableWidgetItem *codeItem = new QTableWidgetItem(curr.code());
        codeItem->setData(Qt::UserRole, curr.id());
        codeItem->setFlags(codeItem->flags() & ~Qt::ItemIsEditable);
        m_currencyTable->setItem(i, 0, codeItem);

        QTableWidgetItem *nameItem = new QTableWidgetItem(curr.name());
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        m_currencyTable->setItem(i, 1, nameItem);

        QDoubleSpinBox *rateSpin = new QDoubleSpinBox(this);
        rateSpin->setDecimals(4);
        rateSpin->setRange(0.0001, 999999.9999);

        // Use snapshot rate if available, otherwise default
        double rate = snapshotRates.value(curr.id(), curr.code() == "RUB" ? 1.0 : 100.0);
        rateSpin->setValue(rate);
        rateSpin->setProperty("currencyId", curr.id());

        m_currencyRates[curr.id()] = rate;

        connect(rateSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &PortfolioPage::onCurrencyRateChanged);

        m_currencyTable->setCellWidget(i, 2, rateSpin);
    }
}

void PortfolioPage::onCurrencyRateChanged()
{
    // Update currency rates cache
    for (int i = 0; i < m_currencyTable->rowCount(); ++i) {
        QTableWidgetItem *codeItem = m_currencyTable->item(i, 0);
        QDoubleSpinBox *rateSpin = qobject_cast<QDoubleSpinBox*>(m_currencyTable->cellWidget(i, 2));
        if (codeItem && rateSpin) {
            int currencyId = codeItem->data(Qt::UserRole).toInt();
            m_currencyRates[currencyId] = rateSpin->value();
        }
    }

    // Reload assets with new rates
    loadAssets();
    updateTotals();
}

void PortfolioPage::loadAssets()
{
    m_assetsTable->setRowCount(0);

    QList<PortfolioAsset> assets = Database::instance().getActivePortfolioAssets();
    m_assetsTable->setRowCount(assets.size());

    for (int i = 0; i < assets.size(); ++i) {
        PortfolioAsset asset = assets[i];

        // Set currency rate from our cache
        double rate = m_currencyRates.value(asset.currencyId(), 1.0);
        asset.setCurrencyRate(rate);

        // Skip assets with zero quantity
        if (asset.totalQuantity() <= 0) {
            continue;
        }

        QTableWidgetItem *nameItem = new QTableWidgetItem(asset.name());
        nameItem->setData(Qt::UserRole, asset.id());
        m_assetsTable->setItem(i, 0, nameItem);

        QString categoryText = asset.categoryName().isEmpty() ? "—" : asset.categoryName();
        m_assetsTable->setItem(i, 1, new QTableWidgetItem(categoryText));

        QTableWidgetItem *qtyItem = new QTableWidgetItem(QString::number(asset.totalQuantity(), 'f', 4));
        qtyItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_assetsTable->setItem(i, 2, qtyItem);

        QString avgPriceStr = QString("%1 %2").arg(asset.averageBuyPrice(), 0, 'f', 2).arg(asset.currencyCode());
        QTableWidgetItem *avgPriceItem = new QTableWidgetItem(avgPriceStr);
        avgPriceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_assetsTable->setItem(i, 3, avgPriceItem);

        QString currPriceStr = QString("%1 %2").arg(asset.currentPrice(), 0, 'f', 2).arg(asset.currencyCode());
        QTableWidgetItem *currPriceItem = new QTableWidgetItem(currPriceStr);
        currPriceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_assetsTable->setItem(i, 4, currPriceItem);

        QString valueStr = QString("%1 ₽").arg(asset.currentValueInRub(), 0, 'f', 2);
        QTableWidgetItem *valueItem = new QTableWidgetItem(valueStr);
        valueItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_assetsTable->setItem(i, 5, valueItem);

        double yield = asset.yieldPercent();
        QString yieldStr = QString("%1%2%").arg(yield >= 0 ? "+" : "").arg(yield, 0, 'f', 2);
        QTableWidgetItem *yieldItem = new QTableWidgetItem(yieldStr);
        yieldItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        yieldItem->setForeground(yield >= 0 ? QColor("#27ae60") : QColor("#e74c3c"));
        m_assetsTable->setItem(i, 6, yieldItem);

        double profit = asset.profitInRub();
        QString profitStr = QString("%1%2 ₽").arg(profit >= 0 ? "+" : "").arg(profit, 0, 'f', 2);
        QTableWidgetItem *profitItem = new QTableWidgetItem(profitStr);
        profitItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        profitItem->setForeground(profit >= 0 ? QColor("#27ae60") : QColor("#e74c3c"));
        m_assetsTable->setItem(i, 7, profitItem);
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

void PortfolioPage::onAssetDoubleClicked(int row, int column)
{
    Q_UNUSED(column);

    QTableWidgetItem *item = m_assetsTable->item(row, 0);
    if (!item) return;

    int assetId = item->data(Qt::UserRole).toInt();
    AssetDetailsDialog dialog(assetId, this);
    dialog.exec();
}
