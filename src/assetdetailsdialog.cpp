#include "assetdetailsdialog.h"
#include "addoperationdialog.h"
#include "database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>

AssetDetailsDialog::AssetDetailsDialog(int assetId, QWidget *parent)
    : QDialog(parent)
    , m_assetId(assetId)
{
    setupUi();
    loadAssetData();
    loadOperations();

    connect(&Database::instance(), &Database::portfolioDataChanged,
            this, &AssetDetailsDialog::refreshData);
}

void AssetDetailsDialog::setupUi()
{
    setWindowTitle("Детали актива");
    setMinimumSize(600, 500);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // Asset info group
    QGroupBox *infoGroup = new QGroupBox("Информация", this);
    QFormLayout *infoLayout = new QFormLayout(infoGroup);
    infoLayout->setSpacing(8);

    m_nameLabel = new QLabel(this);
    m_nameLabel->setStyleSheet("font-weight: bold; font-size: 16px;");
    infoLayout->addRow("Название:", m_nameLabel);

    m_categoryLabel = new QLabel(this);
    infoLayout->addRow("Категория:", m_categoryLabel);

    m_countryLabel = new QLabel(this);
    infoLayout->addRow("Страна:", m_countryLabel);

    m_currencyLabel = new QLabel(this);
    infoLayout->addRow("Валюта:", m_currencyLabel);

    mainLayout->addWidget(infoGroup);

    // Price update section
    QHBoxLayout *priceLayout = new QHBoxLayout();
    priceLayout->addWidget(new QLabel("Текущая цена:", this));

    m_priceSpin = new QDoubleSpinBox(this);
    m_priceSpin->setDecimals(2);
    m_priceSpin->setRange(0.0, 999999999.99);
    m_priceSpin->setMinimumWidth(150);
    priceLayout->addWidget(m_priceSpin);

    m_updatePriceBtn = new QPushButton("Обновить", this);
    m_updatePriceBtn->setCursor(Qt::PointingHandCursor);
    connect(m_updatePriceBtn, &QPushButton::clicked, this, &AssetDetailsDialog::onUpdatePriceClicked);
    priceLayout->addWidget(m_updatePriceBtn);

    priceLayout->addStretch();
    mainLayout->addLayout(priceLayout);

    // Summary group
    QGroupBox *summaryGroup = new QGroupBox("Сводка", this);
    QFormLayout *summaryLayout = new QFormLayout(summaryGroup);
    summaryLayout->setSpacing(8);

    m_quantityLabel = new QLabel(this);
    summaryLayout->addRow("Количество:", m_quantityLabel);

    m_avgPriceLabel = new QLabel(this);
    summaryLayout->addRow("Средняя цена:", m_avgPriceLabel);

    m_currentValueLabel = new QLabel(this);
    summaryLayout->addRow("Текущая стоимость:", m_currentValueLabel);

    m_profitLabel = new QLabel(this);
    summaryLayout->addRow("Прибыль:", m_profitLabel);

    m_yieldLabel = new QLabel(this);
    m_yieldLabel->setStyleSheet("font-weight: bold;");
    summaryLayout->addRow("Доходность:", m_yieldLabel);

    mainLayout->addWidget(summaryGroup);

    // Action buttons
    QHBoxLayout *actionLayout = new QHBoxLayout();

    m_addBuyBtn = new QPushButton("Добавить покупку", this);
    m_addBuyBtn->setObjectName("primaryButton");
    m_addBuyBtn->setCursor(Qt::PointingHandCursor);
    connect(m_addBuyBtn, &QPushButton::clicked, this, &AssetDetailsDialog::onAddBuyClicked);
    actionLayout->addWidget(m_addBuyBtn);

    m_addSellBtn = new QPushButton("Добавить продажу", this);
    m_addSellBtn->setCursor(Qt::PointingHandCursor);
    connect(m_addSellBtn, &QPushButton::clicked, this, &AssetDetailsDialog::onAddSellClicked);
    actionLayout->addWidget(m_addSellBtn);

    actionLayout->addStretch();

    m_deleteAssetBtn = new QPushButton("Удалить актив", this);
    m_deleteAssetBtn->setObjectName("dangerButton");
    m_deleteAssetBtn->setCursor(Qt::PointingHandCursor);
    connect(m_deleteAssetBtn, &QPushButton::clicked, this, &AssetDetailsDialog::onDeleteAssetClicked);
    actionLayout->addWidget(m_deleteAssetBtn);

    mainLayout->addLayout(actionLayout);

    // Operations table
    QLabel *historyLabel = new QLabel("История операций:", this);
    historyLabel->setStyleSheet("font-weight: bold;");
    mainLayout->addWidget(historyLabel);

    m_operationsTable = new QTableWidget(this);
    m_operationsTable->setObjectName("transactionTable");
    m_operationsTable->setColumnCount(7);
    m_operationsTable->setHorizontalHeaderLabels({"Дата", "Тип", "Кол-во", "Цена", "Комиссия", "Комментарий", ""});
    m_operationsTable->horizontalHeader()->setStretchLastSection(false);
    m_operationsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_operationsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_operationsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_operationsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_operationsTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_operationsTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
    m_operationsTable->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Fixed);
    m_operationsTable->setColumnWidth(6, 50);
    m_operationsTable->verticalHeader()->setVisible(false);
    m_operationsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_operationsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_operationsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_operationsTable->setAlternatingRowColors(true);
    m_operationsTable->verticalHeader()->setDefaultSectionSize(35);

    mainLayout->addWidget(m_operationsTable, 1);

    // Close button
    QHBoxLayout *closeLayout = new QHBoxLayout();
    closeLayout->addStretch();
    QPushButton *closeBtn = new QPushButton("Закрыть", this);
    closeBtn->setCursor(Qt::PointingHandCursor);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    closeLayout->addWidget(closeBtn);
    mainLayout->addLayout(closeLayout);
}

void AssetDetailsDialog::refreshData()
{
    loadAssetData();
    loadOperations();
}

void AssetDetailsDialog::loadAssetData()
{
    m_asset = Database::instance().getPortfolioAsset(m_assetId);

    m_nameLabel->setText(m_asset.name());
    m_categoryLabel->setText(m_asset.categoryName().isEmpty() ? "—" : m_asset.categoryName());
    m_countryLabel->setText(m_asset.countryName().isEmpty() ? "—" : m_asset.countryName());
    m_currencyLabel->setText(m_asset.currencyCode());

    m_priceSpin->setValue(m_asset.currentPrice());

    m_quantityLabel->setText(QString::number(m_asset.totalQuantity(), 'f', 6));
    m_avgPriceLabel->setText(QString::number(m_asset.averageBuyPrice(), 'f', 2));
    m_currentValueLabel->setText(QString::number(m_asset.currentValue(), 'f', 2));

    double profit = m_asset.profit();
    QString profitColor = profit >= 0 ? "#27ae60" : "#e74c3c";
    m_profitLabel->setText(QString("<span style='color: %1;'>%2%3</span>")
                          .arg(profitColor)
                          .arg(profit >= 0 ? "+" : "")
                          .arg(profit, 0, 'f', 2));

    double yield = m_asset.yieldPercent();
    QString yieldColor = yield >= 0 ? "#27ae60" : "#e74c3c";
    m_yieldLabel->setText(QString("<span style='color: %1;'>%2%3%</span>")
                         .arg(yieldColor)
                         .arg(yield >= 0 ? "+" : "")
                         .arg(yield, 0, 'f', 2));

    // Enable/disable sell button based on quantity
    m_addSellBtn->setEnabled(m_asset.totalQuantity() > 0);
}

void AssetDetailsDialog::loadOperations()
{
    m_operationsTable->setRowCount(0);

    QList<AssetOperation> operations = Database::instance().getAssetOperations(m_assetId);
    m_operationsTable->setRowCount(operations.size());

    for (int i = 0; i < operations.size(); ++i) {
        const AssetOperation& op = operations[i];

        QTableWidgetItem *dateItem = new QTableWidgetItem(op.date().toString("dd.MM.yyyy"));
        dateItem->setData(Qt::UserRole, op.id());
        m_operationsTable->setItem(i, 0, dateItem);

        QTableWidgetItem *typeItem = new QTableWidgetItem(AssetOperation::typeToDisplayString(op.type()));
        if (op.type() == AssetOperation::Type::Buy) {
            typeItem->setForeground(QColor("#27ae60"));
        } else {
            typeItem->setForeground(QColor("#e74c3c"));
        }
        m_operationsTable->setItem(i, 1, typeItem);

        QTableWidgetItem *qtyItem = new QTableWidgetItem(QString::number(op.quantity(), 'f', 6));
        qtyItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_operationsTable->setItem(i, 2, qtyItem);

        QTableWidgetItem *priceItem = new QTableWidgetItem(QString::number(op.price(), 'f', 2));
        priceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_operationsTable->setItem(i, 3, priceItem);

        QTableWidgetItem *commItem = new QTableWidgetItem(QString::number(op.commission(), 'f', 2));
        commItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_operationsTable->setItem(i, 4, commItem);

        m_operationsTable->setItem(i, 5, new QTableWidgetItem(op.comment()));

        QPushButton *deleteBtn = new QPushButton("X", this);
        deleteBtn->setToolTip("Удалить операцию");
        deleteBtn->setObjectName("deleteButton");
        deleteBtn->setCursor(Qt::PointingHandCursor);
        deleteBtn->setProperty("operationId", op.id());
        connect(deleteBtn, &QPushButton::clicked, this, &AssetDetailsDialog::onDeleteOperationClicked);
        m_operationsTable->setCellWidget(i, 6, deleteBtn);
    }
}

void AssetDetailsDialog::onUpdatePriceClicked()
{
    double newPrice = m_priceSpin->value();
    if (Database::instance().updatePortfolioAssetPrice(m_assetId, newPrice)) {
        loadAssetData();
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось обновить цену");
    }
}

void AssetDetailsDialog::onAddBuyClicked()
{
    AddOperationDialog dialog(m_assetId, AssetOperation::Type::Buy, 0, this);
    if (dialog.exec() == QDialog::Accepted) {
        AssetOperation op = dialog.getOperation();
        if (!Database::instance().addAssetOperation(op)) {
            QMessageBox::critical(this, "Ошибка", "Не удалось добавить покупку");
        }
    }
}

void AssetDetailsDialog::onAddSellClicked()
{
    double maxQty = m_asset.totalQuantity();
    AddOperationDialog dialog(m_assetId, AssetOperation::Type::Sell, maxQty, this);
    if (dialog.exec() == QDialog::Accepted) {
        AssetOperation op = dialog.getOperation();
        if (!Database::instance().addAssetOperation(op)) {
            QMessageBox::critical(this, "Ошибка", "Не удалось добавить продажу");
        }
    }
}

void AssetDetailsDialog::onDeleteAssetClicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Подтверждение",
        QString("Вы уверены, что хотите удалить актив \"%1\"?\n"
                "Все операции по этому активу также будут удалены.")
        .arg(m_asset.name()),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (Database::instance().deletePortfolioAsset(m_assetId)) {
            emit assetDeleted();
            accept();
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось удалить актив");
        }
    }
}

void AssetDetailsDialog::onDeleteOperationClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    int operationId = btn->property("operationId").toInt();

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Подтверждение",
        "Вы уверены, что хотите удалить эту операцию?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (!Database::instance().deleteAssetOperation(operationId)) {
            QMessageBox::critical(this, "Ошибка", "Не удалось удалить операцию");
        }
    }
}
