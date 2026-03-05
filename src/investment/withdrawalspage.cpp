#include "investment/withdrawalspage.h"
#include "investment/withdrawalundonotification.h"
#include "database.h"
#include "widgets/uiutils.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QLabel>

WithdrawalsPage::WithdrawalsPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();

    connect(&Database::instance(), &Database::investmentDataChanged,
            this, &WithdrawalsPage::refreshData);
}

void WithdrawalsPage::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 30, 40, 30);
    mainLayout->setSpacing(20);

    // Title
    QLabel *titleLabel = new QLabel("Выводы из портфеля", this);
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
    m_addBtn = new QPushButton("Вывести", this);
    m_addBtn->setObjectName("primaryButton");
    m_addBtn->setCursor(Qt::PointingHandCursor);
    connect(m_addBtn, &QPushButton::clicked, this, &WithdrawalsPage::onAddClicked);
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
    m_undoNotification = new WithdrawalUndoNotification(this);
    connect(m_undoNotification, &WithdrawalUndoNotification::undoRequested,
            this, &WithdrawalsPage::onUndoClicked);
}

void WithdrawalsPage::refreshData()
{
    loadWithdrawals();
}

void WithdrawalsPage::loadWithdrawals()
{
    m_table->setRowCount(0);

    QList<Withdrawal> withdrawals = Database::instance().getWithdrawals();

    m_table->setRowCount(withdrawals.size());

    for (int i = 0; i < withdrawals.size(); ++i) {
        const Withdrawal& w = withdrawals[i];

        QTableWidgetItem *dateItem = new QTableWidgetItem(w.date().toString("dd.MM.yyyy"));
        dateItem->setData(Qt::UserRole, w.id());
        m_table->setItem(i, 0, dateItem);

        QString amountStr = QString("%1 ₽").arg(w.amount(), 0, 'f', 2);
        QTableWidgetItem *amountItem = new QTableWidgetItem(amountStr);
        amountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        amountItem->setForeground(QColor("#27ae60")); // green for income
        m_table->setItem(i, 1, amountItem);

        m_table->setItem(i, 2, new QTableWidgetItem(w.comment()));

        QPushButton *deleteBtn = new QPushButton("🗑", this);
        deleteBtn->setToolTip("Удалить вывод");
        deleteBtn->setObjectName("deleteButton");
        deleteBtn->setCursor(Qt::PointingHandCursor);
        deleteBtn->setProperty("withdrawalId", w.id());
        connect(deleteBtn, &QPushButton::clicked, this, &WithdrawalsPage::onDeleteClicked);
        m_table->setCellWidget(i, 3, deleteBtn);
    }
}

void WithdrawalsPage::onAddClicked()
{
    double amount = m_amountSpin->value();
    if (amount <= 0) {
        QMessageBox::warning(this, "Ошибка", "Укажите сумму вывода");
        return;
    }

    Withdrawal withdrawal;
    withdrawal.setDate(m_dateEdit->date());
    withdrawal.setAmount(amount);
    withdrawal.setComment(m_commentEdit->text().trimmed());

    if (Database::instance().addWithdrawal(withdrawal)) {
        m_undoNotification->showNotification(withdrawal);
        clearForm();
        loadWithdrawals();
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось добавить вывод");
    }
}

void WithdrawalsPage::onDeleteClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    int withdrawalId = btn->property("withdrawalId").toInt();

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Подтверждение",
        "Вы уверены, что хотите удалить этот вывод?\n"
        "Связанная транзакция дохода также будет удалена.",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (Database::instance().deleteWithdrawal(withdrawalId)) {
            loadWithdrawals();
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось удалить вывод");
        }
    }
}

void WithdrawalsPage::onUndoClicked(const Withdrawal& withdrawal)
{
    if (Database::instance().deleteWithdrawal(withdrawal.id())) {
        loadWithdrawals();
    }
}

void WithdrawalsPage::clearForm()
{
    m_dateEdit->setDate(QDate::currentDate());
    m_amountSpin->setValue(0);
    m_commentEdit->clear();
}
