#include "addtransactionpage.h"
#include "undonotification.h"
#include "categorymanagerdialog.h"
#include "database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>

AddTransactionPage::AddTransactionPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    refreshCategories();

    connect(&Database::instance(), &Database::categoryAdded, this, &AddTransactionPage::refreshCategories);
    connect(&Database::instance(), &Database::categoryDeleted, this, &AddTransactionPage::refreshCategories);
    connect(&Database::instance(), &Database::dataChanged, this, &AddTransactionPage::refreshCategories);
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

    // category and subcategory
    QWidget *categoryWidget = new QWidget(this);
    QHBoxLayout *categoryLayout = new QHBoxLayout(categoryWidget);
    categoryLayout->setContentsMargins(0, 0, 0, 0);
    categoryLayout->setSpacing(10);

    // category
    m_categoryCombo = new CategoryComboBox(this);
    m_categoryCombo->setMinimumWidth(180);
    connect(m_categoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AddTransactionPage::onCategoryChanged);
    connect(m_categoryCombo, &CategoryComboBox::categorySelected,
            this, &AddTransactionPage::onCategoryChanged);
    categoryLayout->addWidget(m_categoryCombo);

    m_categorySettingsBtn = new QPushButton("⚙", this);
    m_categorySettingsBtn->setObjectName("settingsBtn");
    m_categorySettingsBtn->setFixedSize(32, 32);
    m_categorySettingsBtn->setCursor(Qt::PointingHandCursor);
    m_categorySettingsBtn->setToolTip("Управление категориями");
    connect(m_categorySettingsBtn, &QPushButton::clicked, this, &AddTransactionPage::onCategorySettingsClicked);
    categoryLayout->addWidget(m_categorySettingsBtn);

    categoryLayout->addSpacing(20);

    // subcategory
    QLabel *subcatLabel = new QLabel("Подкатегория:", this);
    categoryLayout->addWidget(subcatLabel);

    m_subcategoryCombo = new CategoryComboBox(this);
    m_subcategoryCombo->setMinimumWidth(180);
    categoryLayout->addWidget(m_subcategoryCombo);

    m_subcategorySettingsBtn = new QPushButton("⚙", this);
    m_subcategorySettingsBtn->setObjectName("settingsBtn");
    m_subcategorySettingsBtn->setFixedSize(32, 32);
    m_subcategorySettingsBtn->setCursor(Qt::PointingHandCursor);
    m_subcategorySettingsBtn->setToolTip("Управление подкатегориями");
    connect(m_subcategorySettingsBtn, &QPushButton::clicked, this, &AddTransactionPage::onSubcategorySettingsClicked);
    categoryLayout->addWidget(m_subcategorySettingsBtn);

    m_subcategoryCombo->setEnabled(false);
    m_subcategorySettingsBtn->setEnabled(false);
    categoryLayout->addStretch();
    formLayout->addRow("Категория:", categoryWidget);

    // total
    m_amountSpin = new QDoubleSpinBox(this);
    m_amountSpin->setRange(0.01, 999999999.99);
    m_amountSpin->setDecimals(2);
    m_amountSpin->setSuffix(" ₽");
    m_amountSpin->setMinimumWidth(200);
    m_amountSpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
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

    // notification
    m_undoNotification = new UndoNotification(this);
    connect(m_undoNotification, &UndoNotification::undoRequested,
            this, &AddTransactionPage::onUndoClicked);
}

void AddTransactionPage::onTypeChanged(int index)
{
    Q_UNUSED(index);
    Transaction::Type type = static_cast<Transaction::Type>(m_typeCombo->currentData().toInt());

    bool isExpense = (type == Transaction::Type::Expense);
    m_categoryCombo->setEnabled(isExpense);
    m_categorySettingsBtn->setEnabled(isExpense);
    m_subcategoryCombo->setEnabled(isExpense);
    m_subcategorySettingsBtn->setEnabled(isExpense);
}

void AddTransactionPage::onCategoryChanged()
{
    int categoryId = m_categoryCombo->currentCategoryId();

    bool hasCategory = (categoryId >= 0);
    m_subcategoryCombo->setEnabled(hasCategory);
    m_subcategorySettingsBtn->setEnabled(hasCategory);

    refreshSubcategories();
}

void AddTransactionPage::onCategorySettingsClicked()
{
    CategoryManagerDialog dialog(-1, this);
    connect(&dialog, &CategoryManagerDialog::categoriesChanged, this, &AddTransactionPage::refreshCategories);
    dialog.exec();
}

void AddTransactionPage::onSubcategorySettingsClicked()
{
    int categoryId = m_categoryCombo->currentCategoryId();
    if (categoryId < 0) {
        QMessageBox::information(this, "Подкатегории", "Сначала выберите категорию");
        return;
    }

    CategoryManagerDialog dialog(categoryId, this);
    connect(&dialog, &CategoryManagerDialog::categoriesChanged, this, &AddTransactionPage::refreshSubcategories);
    dialog.exec();
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

        // Подкатегория опциональна
        int subcategoryId = m_subcategoryCombo->currentCategoryId();
        if (subcategoryId >= 0) {
            transaction.setSubcategoryId(subcategoryId);
        }
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

void AddTransactionPage::onUndoClicked(const Transaction& transaction)
{
    Database::instance().deleteTransaction(transaction.id());
    fillFormWithTransaction(transaction);
}

void AddTransactionPage::refreshCategories()
{
    m_categoryCombo->loadCategories(false);
    refreshSubcategories();
}

void AddTransactionPage::refreshSubcategories()
{
    int categoryId = m_categoryCombo->currentCategoryId();
    m_subcategoryCombo->loadSubcategories(categoryId);

    bool hasCategory = (categoryId >= 0);
    m_subcategoryCombo->setEnabled(hasCategory);
    m_subcategorySettingsBtn->setEnabled(hasCategory);
}

void AddTransactionPage::clearForm()
{
    m_dateEdit->setDate(QDate::currentDate());
    m_typeCombo->setCurrentIndex(0);
    m_descriptionEdit->clear();
    m_categoryCombo->clear();
    m_subcategoryCombo->clear();
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

    if (transaction.subcategoryId() >= 0) {
        m_subcategoryCombo->setCurrentCategoryId(transaction.subcategoryId());
    }

    m_amountSpin->setValue(transaction.amount());
}
