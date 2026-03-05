#include "reporting/transactionlistpage.h"
#include "database.h"
#include "widgets/uiutils.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QLabel>

TransactionListPage::TransactionListPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    
    connect(&Database::instance(), &Database::categoryAdded, this, [this]() {
        m_categoryCombo->loadCategories(true);
    });
}

void TransactionListPage::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 30, 40, 30);
    mainLayout->setSpacing(20);
    
    // title
    QLabel *titleLabel = new QLabel("История операций", this);
    titleLabel->setObjectName("pageTitle");
    mainLayout->addWidget(titleLabel);
    
    // filter panel
    QWidget *filterWidget = new QWidget(this);
    filterWidget->setObjectName("filterPanel");
    QHBoxLayout *filterLayout = new QHBoxLayout(filterWidget);
    filterLayout->setContentsMargins(10, 10, 10, 10);
    filterLayout->setSpacing(10);
    
    // period
    QLabel *fromLabel = new QLabel("С:", this);
    filterLayout->addWidget(fromLabel);
    
    m_fromDateEdit = new QDateEdit(this);
    m_fromDateEdit->setCalendarPopup(true);
    m_fromDateEdit->setDisplayFormat("dd.MM.yyyy");
    m_fromDateEdit->setDate(QDate::currentDate().addMonths(-1));
    m_fromDateEdit->setSpecialValueText("—");
    connect(m_fromDateEdit, &QDateEdit::dateChanged, this, &TransactionListPage::onFilterChanged);
    filterLayout->addWidget(m_fromDateEdit);
    
    QLabel *toLabel = new QLabel("По:", this);
    filterLayout->addWidget(toLabel);
    
    m_toDateEdit = new QDateEdit(this);
    m_toDateEdit->setCalendarPopup(true);
    m_toDateEdit->setDisplayFormat("dd.MM.yyyy");
    m_toDateEdit->setDate(QDate::currentDate());
    m_toDateEdit->setSpecialValueText("—");
    connect(m_toDateEdit, &QDateEdit::dateChanged, this, &TransactionListPage::onFilterChanged);
    filterLayout->addWidget(m_toDateEdit);
    
    filterLayout->addSpacing(10);
    
    // type
    QLabel *typeLabel = new QLabel("Тип:", this);
    filterLayout->addWidget(typeLabel);
    
    m_typeCombo = new QComboBox(this);
    m_typeCombo->addItem("Все", static_cast<int>(Transaction::Type::All));
    m_typeCombo->addItem("Расходы", static_cast<int>(Transaction::Type::Expense));
    m_typeCombo->addItem("Доходы", static_cast<int>(Transaction::Type::Income));
    m_typeCombo->addItem("Сбережения", static_cast<int>(Transaction::Type::Savings));
    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TransactionListPage::onFilterChanged);
    filterLayout->addWidget(m_typeCombo);
    
    filterLayout->addSpacing(10);
    
    // category
    QLabel *catLabel = new QLabel("Категория:", this);
    filterLayout->addWidget(catLabel);
    
    m_categoryCombo = new CategoryComboBox(this);
    m_categoryCombo->setMinimumWidth(150);
    m_categoryCombo->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_categoryCombo->loadCategories(true);
    connect(m_categoryCombo, &CategoryComboBox::categorySelected, this, &TransactionListPage::onFilterChanged);
    connect(m_categoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TransactionListPage::onFilterChanged);
    filterLayout->addWidget(m_categoryCombo);

    filterLayout->addStretch();

    // reset button
    m_clearFiltersBtn = new QPushButton("Сбросить", this);
    m_clearFiltersBtn->setObjectName("secondaryButton");
    m_clearFiltersBtn->setCursor(Qt::PointingHandCursor);
    connect(m_clearFiltersBtn, &QPushButton::clicked, this, &TransactionListPage::onClearFiltersClicked);
    filterLayout->addWidget(m_clearFiltersBtn);
    
    UiUtils::applyShadow(filterWidget);
    mainLayout->addWidget(filterWidget);

    // table
    m_table = new QTableWidget(this);
    m_table->setObjectName("transactionTable");
    m_table->setColumnCount(6);
    m_table->setHorizontalHeaderLabels({"Дата", "Тип", "Описание", "Категория", "Сумма", ""});
    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Fixed);
    m_table->setColumnWidth(5, 50);
    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->setDefaultSectionSize(40);
    
    mainLayout->addWidget(m_table, 1);
}

void TransactionListPage::refreshData()
{
    loadTransactions();
}

void TransactionListPage::onFilterChanged()
{
    loadTransactions();
}

void TransactionListPage::loadTransactions()
{
    m_table->setRowCount(0);
    
    QDate fromDate = m_fromDateEdit->date();
    QDate toDate = m_toDateEdit->date();
    int categoryId = m_categoryCombo->currentCategoryId();
    Transaction::Type type = static_cast<Transaction::Type>(m_typeCombo->currentData().toInt());
    
    QList<Transaction> transactions = Database::instance().getTransactions(
        fromDate, toDate, categoryId, type);
    
    m_table->setRowCount(transactions.size());
    
    for (int i = 0; i < transactions.size(); ++i) {
        const Transaction& t = transactions[i];
        
        QTableWidgetItem *dateItem = new QTableWidgetItem(t.date().toString("dd.MM.yyyy"));
        dateItem->setData(Qt::UserRole, t.id());
        m_table->setItem(i, 0, dateItem);
        
        QString typeStr;
        QColor typeColor;
        switch (t.type()) {
        case Transaction::Type::Income:
            typeStr = "Доход";
            typeColor = QColor("#27ae60");
            break;
        case Transaction::Type::Savings:
            typeStr = "Сбережения";
            typeColor = QColor("#8e44ad");
            break;
        default:
            typeStr = "Расход";
            typeColor = QColor("#e74c3c");
            break;
        }
        QTableWidgetItem *typeItem = new QTableWidgetItem(typeStr);
        typeItem->setForeground(typeColor);
        m_table->setItem(i, 1, typeItem);
        
        m_table->setItem(i, 2, new QTableWidgetItem(t.description()));

        QString categoryName = "—";
        if (t.categoryId() >= 0) {
            Category cat = Database::instance().getCategory(t.categoryId());
            if (cat.isValid()) {
                categoryName = cat.name();
            }
        }
        m_table->setItem(i, 3, new QTableWidgetItem(categoryName));
        
        QString amountStr = QString("%1 ₽").arg(t.amount(), 0, 'f', 2);
        QTableWidgetItem *amountItem = new QTableWidgetItem(amountStr);
        amountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        amountItem->setForeground(typeColor);
        m_table->setItem(i, 4, amountItem);
        
        QPushButton *deleteBtn = new QPushButton("🗑", this);
        deleteBtn->setToolTip("Удалить запись");
        deleteBtn->setObjectName("deleteButton");
        deleteBtn->setCursor(Qt::PointingHandCursor);
        deleteBtn->setProperty("transactionId", t.id());
        if (Database::instance().getCategory(t.categoryId()).name() == "Доход с инвестиций") {
            deleteBtn->setEnabled(false);
            deleteBtn->setToolTip("Удалять доход с инвестиций можно на странице выводов");
        }
        connect(deleteBtn, &QPushButton::clicked, this, &TransactionListPage::onDeleteClicked);
        m_table->setCellWidget(i, 5, deleteBtn);
    }
}

void TransactionListPage::onDeleteClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    
    int transactionId = btn->property("transactionId").toInt();
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Подтверждение",
        "Вы уверены, что хотите удалить эту запись?",
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        if (Database::instance().deleteTransaction(transactionId)) {
            loadTransactions();
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось удалить запись");
        }
    }
}

void TransactionListPage::onClearFiltersClicked()
{
    m_fromDateEdit->setDate(QDate::currentDate().addMonths(-1));
    m_toDateEdit->setDate(QDate::currentDate());
    m_typeCombo->setCurrentIndex(0);
    m_categoryCombo->loadCategories(true);
    m_categoryCombo->setCurrentIndex(0);
    
    loadTransactions();
}
