#include "addtransactionpage.h"
#include "undonotification.h"
#include "database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QMessageBox>

AddTransactionPage::AddTransactionPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    refreshCategories();
    
    connect(&Database::instance(), &Database::categoryAdded, this, &AddTransactionPage::refreshCategories);
}

void AddTransactionPage::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 30, 40, 30);
    mainLayout->setSpacing(20);
    
    // title
    QLabel *titleLabel = new QLabel("Добавить запись", this);
    titleLabel->setObjectName("pageTitle");
    mainLayout->addWidget(titleLabel);
    
    // form
    QGroupBox *formGroup = new QGroupBox(this);
    formGroup->setObjectName("formGroup");
    QFormLayout *formLayout = new QFormLayout(formGroup);
    formLayout->setContentsMargins(30, 30, 30, 30);
    formLayout->setSpacing(20);
    formLayout->setLabelAlignment(Qt::AlignRight);
    
    // date
    m_dateEdit = new QDateEdit(QDate::currentDate(), this);
    m_dateEdit->setCalendarPopup(true);
    m_dateEdit->setDisplayFormat("dd.MM.yyyy");
    m_dateEdit->setMinimumWidth(200);
    formLayout->addRow("Дата:", m_dateEdit);
    
    // type
    m_typeCombo = new QComboBox(this);
    m_typeCombo->addItem("Расход", static_cast<int>(Transaction::Type::Expense));
    m_typeCombo->addItem("Доход", static_cast<int>(Transaction::Type::Income));
    m_typeCombo->setMinimumWidth(200);
    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AddTransactionPage::onTypeChanged);
    formLayout->addRow("Тип:", m_typeCombo);
    
    // description
    m_descriptionEdit = new QLineEdit(this);
    m_descriptionEdit->setPlaceholderText("Введите описание...");
    m_descriptionEdit->setMinimumWidth(300);
    formLayout->addRow("Описание:", m_descriptionEdit);
    
    // category with add button
    QWidget *categoryWidget = new QWidget(this);
    QHBoxLayout *categoryLayout = new QHBoxLayout(categoryWidget);
    categoryLayout->setContentsMargins(0, 0, 0, 0);
    categoryLayout->setSpacing(10);
    
    m_categoryCombo = new CategoryComboBox(this);
    m_categoryCombo->setMinimumWidth(250);
    categoryLayout->addWidget(m_categoryCombo);
    
    m_addCategoryBtn = new QPushButton("+", this);
    m_addCategoryBtn->setObjectName("addCategoryBtn");
    m_addCategoryBtn->setFixedSize(32, 32);
    m_addCategoryBtn->setCursor(Qt::PointingHandCursor);
    m_addCategoryBtn->setToolTip("Добавить категорию");
    connect(m_addCategoryBtn, &QPushButton::clicked, this, &AddTransactionPage::onAddCategoryClicked);
    categoryLayout->addWidget(m_addCategoryBtn);
    
    categoryLayout->addStretch();
    formLayout->addRow("Категория:", categoryWidget);
    
    // sum
    m_amountSpin = new QDoubleSpinBox(this);
    m_amountSpin->setRange(0.01, 999999999.99);
    m_amountSpin->setDecimals(2);
    m_amountSpin->setSuffix(" ₽");
    m_amountSpin->setMinimumWidth(200);
    m_amountSpin->setStyleSheet(
        "QDoubleSpinBox { border: 1px solid gray; padding-right: 0px; }"
        "QDoubleSpinBox::up-button { width: 0px; border: none; background: none; }"
        "QDoubleSpinBox::down-button { width: 0px; border: none; background: none; }"

    );  // remove arrows in the field
    formLayout->addRow("Сумма:", m_amountSpin);
    
    mainLayout->addWidget(formGroup);
    
    // add button
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_addBtn = new QPushButton("Добавить", this);
    m_addBtn->setObjectName("primaryButton");
    m_addBtn->setMinimumSize(150, 45);
    m_addBtn->setCursor(Qt::PointingHandCursor);
    connect(m_addBtn, &QPushButton::clicked, this, &AddTransactionPage::onAddClicked);
    buttonLayout->addWidget(m_addBtn);
    
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
    
    mainLayout->addStretch();
    
    // notifications
    m_undoNotification = new UndoNotification(this);
    connect(m_undoNotification, &UndoNotification::undoRequested,
            this, &AddTransactionPage::onUndoClicked);
}

void AddTransactionPage::onTypeChanged(int index)
{
    Transaction::Type type = static_cast<Transaction::Type>(m_typeCombo->currentData().toInt());
    
    bool isExpense = (type == Transaction::Type::Expense);
    m_categoryCombo->setEnabled(isExpense);
    m_addCategoryBtn->setEnabled(isExpense);
    
    if (!isExpense) {
        m_categoryCombo->clear();
    }
}

void AddTransactionPage::onAddClicked()
{
    Transaction transaction;
    transaction.setDate(m_dateEdit->date());
    transaction.setType(static_cast<Transaction::Type>(m_typeCombo->currentData().toInt()));
    transaction.setDescription(m_descriptionEdit->text().trimmed());
    transaction.setAmount(m_amountSpin->value());
    
    if (transaction.type() == Transaction::Type::Expense) {
        int categoryId = m_categoryCombo->currentCategoryId();
        if (categoryId < 0) {
            QMessageBox::warning(this, "Ошибка", "Выберите категорию для расхода");
            return;
        }
        transaction.setCategoryId(categoryId);
    }
    
    if (transaction.amount() <= 0) {
        QMessageBox::warning(this, "Ошибка", "Укажите сумму");
        return;
    }
    
    QString categoryName;
    if (transaction.categoryId() >= 0) {
        categoryName = Database::instance().getCategory(transaction.categoryId()).name();
    }
    
    if (Database::instance().addTransaction(transaction)) {
        m_undoNotification->showNotification(transaction, categoryName);
        clearForm();
        emit transactionAdded();
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось добавить запись");
    }
}

void AddTransactionPage::onAddCategoryClicked()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Новая категория");
    dialog.setFixedSize(350, 120);
    
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);
    
    QLineEdit *nameEdit = new QLineEdit(&dialog);
    nameEdit->setPlaceholderText("Название категории");
    layout->addWidget(nameEdit);
    
    QDialogButtonBox *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    buttons->button(QDialogButtonBox::Ok)->setText("Добавить");
    buttons->button(QDialogButtonBox::Cancel)->setText("Отмена");
    layout->addWidget(buttons);
    
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    
    if (dialog.exec() == QDialog::Accepted) {
        QString name = nameEdit->text().trimmed();
        if (name.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Введите название категории");
            return;
        }
        
        Category category;
        category.setName(name);
        
        if (Database::instance().addCategory(category)) {
            refreshCategories();
            m_categoryCombo->setCurrentCategoryId(category.id());
        } else {
            QMessageBox::warning(this, "Ошибка", 
                "Не удалось добавить категорию.\nВозможно, такая категория уже существует.");
        }
    }
}

void AddTransactionPage::onUndoClicked(const Transaction& transaction)
{
    Database::instance().deleteTransaction(transaction.id());
    fillFormWithTransaction(transaction);
}

void AddTransactionPage::refreshCategories()
{
    m_categoryCombo->loadCategories(false);
}

void AddTransactionPage::clearForm()
{
    m_dateEdit->setDate(QDate::currentDate());
    m_typeCombo->setCurrentIndex(0);
    m_descriptionEdit->clear();
    m_categoryCombo->clear();
    m_amountSpin->setValue(0);
}

void AddTransactionPage::fillFormWithTransaction(const Transaction& transaction)
{
    m_dateEdit->setDate(transaction.date());
    
    int typeIndex = m_typeCombo->findData(static_cast<int>(transaction.type()));
    if (typeIndex >= 0) {
        m_typeCombo->setCurrentIndex(typeIndex);
    }
    
    m_descriptionEdit->setText(transaction.description());
    
    if (transaction.categoryId() >= 0) {
        m_categoryCombo->setCurrentCategoryId(transaction.categoryId());
    }
    
    m_amountSpin->setValue(transaction.amount());
}
