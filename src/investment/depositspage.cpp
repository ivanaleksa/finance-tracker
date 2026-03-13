#include "investment/depositspage.h"
#include "investment/depositundonotification.h"
#include "database.h"
#include "widgets/uiutils.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QLabel>

DepositsPage::DepositsPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();

    connect(&Database::instance(), &Database::investmentDataChanged,
            this, &DepositsPage::refreshData);
}

void DepositsPage::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 30, 40, 30);
    mainLayout->setSpacing(20);

    // Title
    QLabel *titleLabel = new QLabel("Пополнения портфеля", this);
    titleLabel->setObjectName("pageTitle");
    mainLayout->addWidget(titleLabel);

    // Add form panel
    QWidget *formWidget = new QWidget(this);
    formWidget->setObjectName("formGroup");
    QHBoxLayout *formLayout = new QHBoxLayout(formWidget);
    formLayout->setContentsMargins(20, 15, 20, 15);
    formLayout->setSpacing(20);

    // Date
    QLabel *dateLabel = new QLabel("Дата:", this);
    formLayout->addWidget(dateLabel);

    m_dateEdit = new QDateEdit(this);
    m_dateEdit->setCalendarPopup(true);
    m_dateEdit->setDisplayFormat("dd.MM.yyyy");
    m_dateEdit->setDate(QDate::currentDate());
    m_dateEdit->setMinimumWidth(130);
    formLayout->addWidget(m_dateEdit);

    formLayout->addSpacing(10);

    // Amount
    QLabel *amountLabel = new QLabel("Сумма:", this);
    formLayout->addWidget(amountLabel);

    m_amountSpin = new QDoubleSpinBox(this);
    m_amountSpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_amountSpin->setRange(0.01, 999999999.99);
    m_amountSpin->setDecimals(2);
    m_amountSpin->setSuffix(" ₽");
    m_amountSpin->setMinimumWidth(150);
    formLayout->addWidget(m_amountSpin);

    formLayout->addSpacing(10);

    // Comment
    QLabel *commentLabel = new QLabel("Комментарий:", this);
    formLayout->addWidget(commentLabel);

    m_commentEdit = new QLineEdit(this);
    m_commentEdit->setPlaceholderText("Необязательно");
    m_commentEdit->setMinimumWidth(100);
    formLayout->addWidget(m_commentEdit, 1);

    formLayout->addSpacing(10);

    // Add button
    m_addBtn = new QPushButton("Пополнить", this);
    m_addBtn->setObjectName("primaryButton");
    m_addBtn->setCursor(Qt::PointingHandCursor);
    connect(m_addBtn, &QPushButton::clicked, this, &DepositsPage::onAddClicked);
    formLayout->addWidget(m_addBtn);

    UiUtils::applyShadow(formWidget);
    mainLayout->addWidget(formWidget);

    // Table
    m_table = new QTableWidget(this);
    m_table->setObjectName("transactionTable");
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels({"Дата", "Сумма", "Комментарий", ""});
    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    m_table->setColumnWidth(3, 50);
    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->setDefaultSectionSize(40);

    mainLayout->addWidget(m_table, 1);

    // Undo notification
    m_undoNotification = new DepositUndoNotification(this);
    connect(m_undoNotification, &DepositUndoNotification::undoRequested,
            this, &DepositsPage::onUndoClicked);
}

void DepositsPage::refreshData()
{
    loadDeposits();
}

void DepositsPage::loadDeposits()
{
    m_table->setRowCount(0);

    QList<Deposit> deposits = Database::instance().getDeposits();

    m_table->setRowCount(deposits.size());

    for (int i = 0; i < deposits.size(); ++i) {
        const Deposit& d = deposits[i];

        QTableWidgetItem *dateItem = new QTableWidgetItem(d.date().toString("dd.MM.yyyy"));
        dateItem->setData(Qt::UserRole, d.id());
        m_table->setItem(i, 0, dateItem);

        QString amountStr = QString("%1 ₽").arg(d.amount(), 0, 'f', 2);
        QTableWidgetItem *amountItem = new QTableWidgetItem(amountStr);
        amountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        amountItem->setForeground(QColor("#3498db")); // blue for deposit
        m_table->setItem(i, 1, amountItem);

        m_table->setItem(i, 2, new QTableWidgetItem(d.comment()));

        QPushButton *deleteBtn = new QPushButton("", this);
        deleteBtn->setIcon(QIcon(":/icons/utility/trash.svg"));
        deleteBtn->setIconSize(QSize(20, 20));
        deleteBtn->setToolTip("Удалить пополнение");
        deleteBtn->setObjectName("deleteButton");
        deleteBtn->setCursor(Qt::PointingHandCursor);
        deleteBtn->setProperty("depositId", d.id());
        connect(deleteBtn, &QPushButton::clicked, this, &DepositsPage::onDeleteClicked);
        m_table->setCellWidget(i, 3, deleteBtn);
    }
}

void DepositsPage::onAddClicked()
{
    double amount = m_amountSpin->value();
    if (amount <= 0) {
        QMessageBox::warning(this, "Ошибка", "Укажите сумму пополнения");
        return;
    }

    Deposit deposit;
    deposit.setDate(m_dateEdit->date());
    deposit.setAmount(amount);
    deposit.setComment(m_commentEdit->text().trimmed());

    if (Database::instance().addDeposit(deposit)) {
        m_undoNotification->showNotification(deposit);
        clearForm();
        loadDeposits();
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось добавить пополнение");
    }
}

void DepositsPage::onDeleteClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    int depositId = btn->property("depositId").toInt();

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Подтверждение",
        "Вы уверены, что хотите удалить это пополнение?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (Database::instance().deleteDeposit(depositId)) {
            loadDeposits();
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось удалить пополнение");
        }
    }
}

void DepositsPage::onUndoClicked(const Deposit& deposit)
{
    if (Database::instance().deleteDeposit(deposit.id())) {
        loadDeposits();
    }
}

void DepositsPage::clearForm()
{
    m_dateEdit->setDate(QDate::currentDate());
    m_amountSpin->setValue(0);
    m_commentEdit->clear();
}
