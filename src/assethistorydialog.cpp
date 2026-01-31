#include "assethistorydialog.h"
#include "database.h"
#include "assetoperation.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>

AssetHistoryDialog::AssetHistoryDialog(int assetId, QWidget *parent)
    : QDialog(parent)
    , m_assetId(assetId)
{
    PortfolioAsset asset = Database::instance().getPortfolioAsset(assetId);
    m_assetName = asset.name();

    setupUi();
    loadOperations();

    connect(&Database::instance(), &Database::portfolioDataChanged,
            this, &AssetHistoryDialog::refreshOperations);
}

void AssetHistoryDialog::setupUi()
{
    setWindowTitle(QString("История операций: %1").arg(m_assetName));
    setMinimumSize(600, 400);
    resize(650, 450);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // Title
    QLabel *titleLabel = new QLabel(QString("История операций по активу \"%1\"").arg(m_assetName), this);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #2c3e50;");
    mainLayout->addWidget(titleLabel);

    // Operations table
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
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    QPushButton *closeBtn = new QPushButton("Закрыть", this);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton { padding: 8px 24px; border-radius: 6px; "
        "background-color: #3498db; color: white; font-weight: bold; }"
        "QPushButton:hover { background-color: #2980b9; }"
    );
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addWidget(closeBtn);

    mainLayout->addLayout(buttonLayout);
}

void AssetHistoryDialog::refreshOperations()
{
    loadOperations();
}

void AssetHistoryDialog::loadOperations()
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

        QTableWidgetItem *priceItem = new QTableWidgetItem(QString::number(op.price(), 'f', 6));
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
        connect(deleteBtn, &QPushButton::clicked, this, &AssetHistoryDialog::onDeleteOperationClicked);
        m_operationsTable->setCellWidget(i, 6, deleteBtn);
    }
}

void AssetHistoryDialog::onDeleteOperationClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    int operationId = btn->property("operationId").toInt();

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Подтверждение");
    msgBox.setText("Вы уверены, что хотите удалить эту операцию?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setStyleSheet(
        "QMessageBox { background-color: white; }"
        "QMessageBox QLabel { color: #2c3e50; }"
        "QPushButton { background-color: #3498db; color: white; border: none; "
        "padding: 8px 16px; border-radius: 4px; min-width: 70px; }"
        "QPushButton:hover { background-color: #2980b9; }"
    );

    if (msgBox.exec() == QMessageBox::Yes) {
        if (!Database::instance().deleteAssetOperation(operationId)) {
            QMessageBox::critical(this, "Ошибка", "Не удалось удалить операцию");
        }
    }
}
