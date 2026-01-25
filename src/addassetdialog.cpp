#include "addassetdialog.h"
#include "database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QLabel>
#include <QInputDialog>

AddAssetDialog::AddAssetDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi();
    loadComboData();
}

void AddAssetDialog::setupUi()
{
    setWindowTitle("Добавить актив");
    setMinimumWidth(450);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // Asset info group
    QGroupBox *assetGroup = new QGroupBox("Информация об активе", this);
    QFormLayout *assetLayout = new QFormLayout(assetGroup);
    assetLayout->setSpacing(10);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText("Название актива");
    assetLayout->addRow("Название:", m_nameEdit);

    // Category with add button
    QHBoxLayout *categoryLayout = new QHBoxLayout();
    m_categoryCombo = new QComboBox(this);
    m_categoryCombo->addItem("-- Не выбрано --", -1);
    m_categoryCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    categoryLayout->addWidget(m_categoryCombo);
    QPushButton *addCategoryBtn = new QPushButton("+", this);
    addCategoryBtn->setFixedWidth(30);
    addCategoryBtn->setCursor(Qt::PointingHandCursor);
    addCategoryBtn->setToolTip("Добавить категорию");
    connect(addCategoryBtn, &QPushButton::clicked, this, &AddAssetDialog::onAddCategoryClicked);
    categoryLayout->addWidget(addCategoryBtn);
    assetLayout->addRow("Категория:", categoryLayout);

    // Country with add button
    QHBoxLayout *countryLayout = new QHBoxLayout();
    m_countryCombo = new QComboBox(this);
    m_countryCombo->addItem("-- Не выбрано --", -1);
    m_countryCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    countryLayout->addWidget(m_countryCombo);
    QPushButton *addCountryBtn = new QPushButton("+", this);
    addCountryBtn->setFixedWidth(30);
    addCountryBtn->setCursor(Qt::PointingHandCursor);
    addCountryBtn->setToolTip("Добавить страну");
    connect(addCountryBtn, &QPushButton::clicked, this, &AddAssetDialog::onAddCountryClicked);
    countryLayout->addWidget(addCountryBtn);
    assetLayout->addRow("Страна:", countryLayout);

    m_currencyCombo = new QComboBox(this);
    assetLayout->addRow("Валюта:", m_currencyCombo);

    mainLayout->addWidget(assetGroup);

    // First purchase group
    QGroupBox *purchaseGroup = new QGroupBox("Первая покупка", this);
    QFormLayout *purchaseLayout = new QFormLayout(purchaseGroup);
    purchaseLayout->setSpacing(10);

    m_dateEdit = new QDateEdit(this);
    m_dateEdit->setCalendarPopup(true);
    m_dateEdit->setDisplayFormat("dd.MM.yyyy");
    m_dateEdit->setDate(QDate::currentDate());
    purchaseLayout->addRow("Дата:", m_dateEdit);

    m_quantitySpin = new QDoubleSpinBox(this);
    m_quantitySpin->setDecimals(6);
    m_quantitySpin->setRange(0.000001, 999999999.0);
    m_quantitySpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    purchaseLayout->addRow("Количество:", m_quantitySpin);

    m_buyPriceSpin = new QDoubleSpinBox(this);
    m_buyPriceSpin->setDecimals(2);
    m_buyPriceSpin->setRange(0.0, 999999999.99);
    m_buyPriceSpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    purchaseLayout->addRow("Цена покупки:", m_buyPriceSpin);

    m_currentPriceSpin = new QDoubleSpinBox(this);
    m_currentPriceSpin->setDecimals(2);
    m_currentPriceSpin->setRange(0.0, 999999999.99);
    m_currentPriceSpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    purchaseLayout->addRow("Текущая цена:", m_currentPriceSpin);

    m_commissionSpin = new QDoubleSpinBox(this);
    m_commissionSpin->setDecimals(2);
    m_commissionSpin->setRange(0.0, 999999999.99);
    m_commissionSpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    purchaseLayout->addRow("Комиссия:", m_commissionSpin);

    mainLayout->addWidget(purchaseGroup);

    // Connect buy price to current price for convenience
    connect(m_buyPriceSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [this](double value) {
                if (m_currentPriceSpin->value() == 0) {
                    m_currentPriceSpin->setValue(value);
                }
            });

    mainLayout->addSpacing(10);

    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();

    m_addBtn = new QPushButton("Добавить", this);
    m_addBtn->setObjectName("primaryButton");
    m_addBtn->setCursor(Qt::PointingHandCursor);
    connect(m_addBtn, &QPushButton::clicked, this, &AddAssetDialog::onAddClicked);
    btnLayout->addWidget(m_addBtn, 0, Qt::AlignCenter);

    mainLayout->addLayout(btnLayout);
}

void AddAssetDialog::loadComboData()
{
    // Load categories
    QList<InvestmentCategory> categories = Database::instance().getInvestmentCategories();
    for (const InvestmentCategory& cat : categories) {
        m_categoryCombo->addItem(cat.name(), cat.id());
    }

    // Load countries
    QList<Country> countries = Database::instance().getCountries();
    for (const Country& country : countries) {
        m_countryCombo->addItem(country.name(), country.id());
    }

    // Load currencies
    QList<Currency> currencies = Database::instance().getCurrencies();
    for (const Currency& curr : currencies) {
        m_currencyCombo->addItem(QString("%1 - %2").arg(curr.code(), curr.name()), curr.id());
    }
}

void AddAssetDialog::onAddClicked()
{
    QString name = m_nameEdit->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Укажите название актива");
        return;
    }

    if (m_currencyCombo->currentIndex() < 0) {
        QMessageBox::warning(this, "Ошибка", "Выберите валюту");
        return;
    }

    double quantity = m_quantitySpin->value();
    if (quantity <= 0) {
        QMessageBox::warning(this, "Ошибка", "Укажите количество");
        return;
    }

    double buyPrice = m_buyPriceSpin->value();
    if (buyPrice <= 0) {
        QMessageBox::warning(this, "Ошибка", "Укажите цену покупки");
        return;
    }

    double currentPrice = m_currentPriceSpin->value();
    if (currentPrice <= 0) {
        currentPrice = buyPrice;
    }

    // Create asset
    m_resultAsset.setName(name);
    m_resultAsset.setCategoryId(m_categoryCombo->currentData().toInt());
    m_resultAsset.setCountryId(m_countryCombo->currentData().toInt());
    m_resultAsset.setCurrencyId(m_currencyCombo->currentData().toInt());
    m_resultAsset.setCurrentPrice(currentPrice);
    m_resultAsset.setIsActive(true);

    // Create first operation
    m_resultOperation.setDate(m_dateEdit->date());
    m_resultOperation.setType(AssetOperation::Type::Buy);
    m_resultOperation.setQuantity(quantity);
    m_resultOperation.setPrice(buyPrice);
    m_resultOperation.setCommission(m_commissionSpin->value());

    accept();
}

PortfolioAsset AddAssetDialog::getAsset() const
{
    return m_resultAsset;
}

AssetOperation AddAssetDialog::getFirstOperation() const
{
    return m_resultOperation;
}

void AddAssetDialog::onAddCategoryClicked()
{
    bool ok;
    QString name = QInputDialog::getText(this, "Новая категория",
                                         "Название категории:", QLineEdit::Normal,
                                         QString(), &ok);
    if (ok && !name.trimmed().isEmpty()) {
        InvestmentCategory category;
        category.setName(name.trimmed());
        if (Database::instance().addInvestmentCategory(category)) {
            reloadCategories();
            // Select the new category
            int index = m_categoryCombo->findData(category.id());
            if (index >= 0) {
                m_categoryCombo->setCurrentIndex(index);
            }
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось добавить категорию.\nВозможно, такая категория уже существует.");
        }
    }
}

void AddAssetDialog::onAddCountryClicked()
{
    bool ok;
    QString name = QInputDialog::getText(this, "Новая страна",
                                         "Название страны:", QLineEdit::Normal,
                                         QString(), &ok);
    if (ok && !name.trimmed().isEmpty()) {
        Country country;
        country.setName(name.trimmed());
        if (Database::instance().addCountry(country)) {
            reloadCountries();
            // Select the new country
            int index = m_countryCombo->findData(country.id());
            if (index >= 0) {
                m_countryCombo->setCurrentIndex(index);
            }
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось добавить страну.\nВозможно, такая страна уже существует.");
        }
    }
}

void AddAssetDialog::reloadCategories()
{
    int currentId = m_categoryCombo->currentData().toInt();
    m_categoryCombo->clear();
    m_categoryCombo->addItem("-- Не выбрано --", -1);

    QList<InvestmentCategory> categories = Database::instance().getInvestmentCategories();
    for (const InvestmentCategory& cat : categories) {
        m_categoryCombo->addItem(cat.name(), cat.id());
    }

    int index = m_categoryCombo->findData(currentId);
    if (index >= 0) {
        m_categoryCombo->setCurrentIndex(index);
    }
}

void AddAssetDialog::reloadCountries()
{
    int currentId = m_countryCombo->currentData().toInt();
    m_countryCombo->clear();
    m_countryCombo->addItem("-- Не выбрано --", -1);

    QList<Country> countries = Database::instance().getCountries();
    for (const Country& country : countries) {
        m_countryCombo->addItem(country.name(), country.id());
    }

    int index = m_countryCombo->findData(currentId);
    if (index >= 0) {
        m_countryCombo->setCurrentIndex(index);
    }
}
