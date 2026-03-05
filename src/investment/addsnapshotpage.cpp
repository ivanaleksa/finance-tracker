#include "investment/addsnapshotpage.h"
#include "investment/snapshotundonotification.h"
#include "investment/investmentcategorymanagerdialog.h"
#include "investment/countrymanagerdialog.h"
#include "database.h"
#include "widgets/uiutils.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QScrollArea>

AddSnapshotPage::AddSnapshotPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    loadCurrencies();
    loadCategories();
    loadCountries();
    loadFromPreviousSnapshot();

    connect(&Database::instance(), &Database::investmentDataChanged,
            this, &AddSnapshotPage::refreshData);
}

void AddSnapshotPage::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 30, 40, 30);
    mainLayout->setSpacing(15);

    // Title
    QLabel *titleLabel = new QLabel("Новый слепок портфеля", this);
    titleLabel->setObjectName("pageTitle");
    mainLayout->addWidget(titleLabel);

    // Scroll area for content
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget *scrollContent = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(scrollContent);
    contentLayout->setSpacing(15);

    // === Header form (date, description) ===
    QWidget *headerWidget = new QWidget();
    headerWidget->setObjectName("formGroup");
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(15, 10, 15, 10);
    headerLayout->setSpacing(15);

    QLabel *dateLabel = new QLabel("Дата:", this);
    headerLayout->addWidget(dateLabel);

    m_dateEdit = new QDateEdit(this);
    m_dateEdit->setCalendarPopup(true);
    m_dateEdit->setDisplayFormat("dd.MM.yyyy");
    m_dateEdit->setDate(QDate::currentDate());
    m_dateEdit->setMinimumWidth(130);
    headerLayout->addWidget(m_dateEdit);

    headerLayout->addSpacing(20);

    QLabel *descLabel = new QLabel("Описание:", this);
    headerLayout->addWidget(descLabel);

    m_descriptionEdit = new QLineEdit(this);
    m_descriptionEdit->setPlaceholderText("Необязательно");
    headerLayout->addWidget(m_descriptionEdit, 1);

    UiUtils::applyShadow(headerWidget);
    contentLayout->addWidget(headerWidget);

    // === Currency rates section ===
    QGroupBox *currencyGroup = new QGroupBox("Курсы валют", this);
    QVBoxLayout *currencyLayout = new QVBoxLayout(currencyGroup);

    m_currencyTable = new QTableWidget(this);
    m_currencyTable->setColumnCount(3);
    m_currencyTable->setHorizontalHeaderLabels({"Валюта", "Курс к рублю", ""});
    m_currencyTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_currencyTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_currencyTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    m_currencyTable->setColumnWidth(2, 40);
    m_currencyTable->verticalHeader()->setVisible(false);
    m_currencyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_currencyTable->setMaximumHeight(150);
    currencyLayout->addWidget(m_currencyTable);

    m_addCurrencyBtn = new QPushButton("+ Добавить валюту", this);
    m_addCurrencyBtn->setCursor(Qt::PointingHandCursor);
    connect(m_addCurrencyBtn, &QPushButton::clicked, this, &AddSnapshotPage::onAddCurrencyClicked);
    currencyLayout->addWidget(m_addCurrencyBtn);

    contentLayout->addWidget(currencyGroup);

    // === Positions section ===
    QGroupBox *positionsGroup = new QGroupBox("Позиции портфеля", this);
    QVBoxLayout *positionsLayout = new QVBoxLayout(positionsGroup);

    // Management buttons
    QHBoxLayout *manageBtnLayout = new QHBoxLayout();
    m_manageCategoriesBtn = new QPushButton("⚙ Категории", this);
    m_manageCategoriesBtn->setCursor(Qt::PointingHandCursor);
    connect(m_manageCategoriesBtn, &QPushButton::clicked, this, &AddSnapshotPage::onManageCategoriesClicked);
    manageBtnLayout->addWidget(m_manageCategoriesBtn);

    m_manageCountriesBtn = new QPushButton("⚙ Страны", this);
    m_manageCountriesBtn->setCursor(Qt::PointingHandCursor);
    connect(m_manageCountriesBtn, &QPushButton::clicked, this, &AddSnapshotPage::onManageCountriesClicked);
    manageBtnLayout->addWidget(m_manageCountriesBtn);

    manageBtnLayout->addStretch();
    positionsLayout->addLayout(manageBtnLayout);

    // Positions table
    m_positionsTable = new QTableWidget(this);
    m_positionsTable->setColumnCount(8);
    m_positionsTable->setHorizontalHeaderLabels({
        "Название", "Категория", "Цена", "Кол-во", "Страна", "Валюта", "Сумма (₽)", ""
    });
    m_positionsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_positionsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_positionsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_positionsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_positionsTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_positionsTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    m_positionsTable->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
    m_positionsTable->horizontalHeader()->setSectionResizeMode(7, QHeaderView::Fixed);
    m_positionsTable->setColumnWidth(7, 40);
    m_positionsTable->verticalHeader()->setVisible(false);
    m_positionsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_positionsTable->setMinimumHeight(200);
    positionsLayout->addWidget(m_positionsTable);

    m_addPositionBtn = new QPushButton("+ Добавить позицию", this);
    m_addPositionBtn->setCursor(Qt::PointingHandCursor);
    connect(m_addPositionBtn, &QPushButton::clicked, this, &AddSnapshotPage::onAddPositionClicked);
    positionsLayout->addWidget(m_addPositionBtn);

    contentLayout->addWidget(positionsGroup);

    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea, 1);

    // === Total and save ===
    QWidget *footerWidget = new QWidget();
    footerWidget->setObjectName("formGroup");
    QHBoxLayout *footerLayout = new QHBoxLayout(footerWidget);
    footerLayout->setContentsMargins(15, 10, 15, 10);

    m_totalLabel = new QLabel("Итого: 0.00 ₽", this);
    m_totalLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    footerLayout->addWidget(m_totalLabel);

    footerLayout->addStretch();

    m_saveBtn = new QPushButton("Сохранить слепок", this);
    m_saveBtn->setObjectName("primaryButton");
    m_saveBtn->setCursor(Qt::PointingHandCursor);
    connect(m_saveBtn, &QPushButton::clicked, this, &AddSnapshotPage::onSaveClicked);
    footerLayout->addWidget(m_saveBtn);

    UiUtils::applyShadow(footerWidget);
    mainLayout->addWidget(footerWidget);

    // Undo notification
    m_undoNotification = new SnapshotUndoNotification(this);
    connect(m_undoNotification, &SnapshotUndoNotification::undoRequested,
            this, &AddSnapshotPage::onUndoClicked);
}

void AddSnapshotPage::refreshData()
{
    loadCurrencies();
    loadCategories();
    loadCountries();
}

void AddSnapshotPage::loadCurrencies()
{
    m_currencies = Database::instance().getCurrencies();
}

void AddSnapshotPage::loadCategories()
{
    m_categories = Database::instance().getInvestmentCategories();

    // Update category combos in positions table
    for (int row = 0; row < m_positionsTable->rowCount(); ++row) {
        QComboBox *combo = qobject_cast<QComboBox*>(m_positionsTable->cellWidget(row, 1));
        if (combo) {
            int currentId = combo->currentData().toInt();
            combo->clear();
            combo->addItem("—", -1);
            for (const auto& cat : m_categories) {
                combo->addItem(cat.name(), cat.id());
            }
            int idx = combo->findData(currentId);
            if (idx >= 0) combo->setCurrentIndex(idx);
        }
    }
}

void AddSnapshotPage::loadCountries()
{
    m_countries = Database::instance().getCountries();

    // Update country combos in positions table
    for (int row = 0; row < m_positionsTable->rowCount(); ++row) {
        QComboBox *combo = qobject_cast<QComboBox*>(m_positionsTable->cellWidget(row, 4));
        if (combo) {
            int currentId = combo->currentData().toInt();
            combo->clear();
            combo->addItem("—", -1);
            for (const auto& country : m_countries) {
                combo->addItem(country.name(), country.id());
            }
            int idx = combo->findData(currentId);
            if (idx >= 0) combo->setCurrentIndex(idx);
        }
    }
}

void AddSnapshotPage::loadFromPreviousSnapshot()
{
    Snapshot prev = Database::instance().getLatestSnapshot();

    m_currencyTable->setRowCount(0);
    m_positionsTable->setRowCount(0);

    if (prev.isValid()) {
        // Load currency rates
        QMapIterator<int, double> rateIt(prev.currencyRates());
        while (rateIt.hasNext()) {
            rateIt.next();
            Currency curr = Database::instance().getCurrency(rateIt.key());
            if (curr.isValid()) {
                int row = m_currencyTable->rowCount();
                m_currencyTable->insertRow(row);

                QTableWidgetItem *nameItem = new QTableWidgetItem(QString("%1 (%2)").arg(curr.name(), curr.code()));
                nameItem->setData(Qt::UserRole, curr.id());
                nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
                m_currencyTable->setItem(row, 0, nameItem);

                QDoubleSpinBox *rateSpin = new QDoubleSpinBox(this);
                rateSpin->setRange(0.0001, 999999.9999);
                rateSpin->setDecimals(4);
                rateSpin->setValue(rateIt.value());
                rateSpin->setEnabled(curr.code() != "RUB");
                connect(rateSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                        this, &AddSnapshotPage::onCurrencyRateChanged);
                m_currencyTable->setCellWidget(row, 1, rateSpin);

                if (curr.code() != "RUB") {
                    QPushButton *delBtn = new QPushButton("✕", this);
                    delBtn->setFixedSize(30, 25);
                    delBtn->setCursor(Qt::PointingHandCursor);
                    delBtn->setProperty("row", row);
                    connect(delBtn, &QPushButton::clicked, this, &AddSnapshotPage::onDeleteCurrencyClicked);
                    m_currencyTable->setCellWidget(row, 2, delBtn);
                }
            }
        }

        // Load positions
        for (const SnapshotPosition& pos : prev.positions()) {
            onAddPositionClicked();
            int row = m_positionsTable->rowCount() - 1;

            // Name
            QLineEdit *nameEdit = qobject_cast<QLineEdit*>(m_positionsTable->cellWidget(row, 0));
            if (nameEdit) nameEdit->setText(pos.name());

            // Category
            QComboBox *catCombo = qobject_cast<QComboBox*>(m_positionsTable->cellWidget(row, 1));
            if (catCombo) {
                int idx = catCombo->findData(pos.categoryId());
                if (idx >= 0) catCombo->setCurrentIndex(idx);
            }

            // Price
            QDoubleSpinBox *priceSpin = qobject_cast<QDoubleSpinBox*>(m_positionsTable->cellWidget(row, 2));
            if (priceSpin) priceSpin->setValue(pos.price());

            // Quantity
            QDoubleSpinBox *qtySpin = qobject_cast<QDoubleSpinBox*>(m_positionsTable->cellWidget(row, 3));
            if (qtySpin) qtySpin->setValue(pos.quantity());

            // Country
            QComboBox *countryCombo = qobject_cast<QComboBox*>(m_positionsTable->cellWidget(row, 4));
            if (countryCombo) {
                int idx = countryCombo->findData(pos.countryId());
                if (idx >= 0) countryCombo->setCurrentIndex(idx);
            }

            // Currency
            QComboBox *currCombo = qobject_cast<QComboBox*>(m_positionsTable->cellWidget(row, 5));
            if (currCombo) {
                int idx = currCombo->findData(pos.currencyId());
                if (idx >= 0) currCombo->setCurrentIndex(idx);
            }
        }
    } else {
        // No previous snapshot - add RUB with rate 1.0
        Currency rub = Database::instance().getCurrencyByCode("RUB");
        if (rub.isValid()) {
            m_currencyTable->insertRow(0);

            QTableWidgetItem *nameItem = new QTableWidgetItem(QString("%1 (%2)").arg(rub.name(), rub.code()));
            nameItem->setData(Qt::UserRole, rub.id());
            nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
            m_currencyTable->setItem(0, 0, nameItem);

            QDoubleSpinBox *rateSpin = new QDoubleSpinBox(this);
            rateSpin->setRange(0.0001, 999999.9999);
            rateSpin->setDecimals(4);
            rateSpin->setValue(1.0);
            rateSpin->setEnabled(false);
            m_currencyTable->setCellWidget(0, 1, rateSpin);
        }
    }

    updateTotalSum();
}

void AddSnapshotPage::onAddCurrencyClicked()
{
    // Find currencies not yet in table
    QList<Currency> available;
    for (const Currency& curr : m_currencies) {
        bool found = false;
        for (int row = 0; row < m_currencyTable->rowCount(); ++row) {
            if (m_currencyTable->item(row, 0)->data(Qt::UserRole).toInt() == curr.id()) {
                found = true;
                break;
            }
        }
        if (!found) available.append(curr);
    }

    if (available.isEmpty()) {
        QMessageBox::information(this, "Информация", "Все доступные валюты уже добавлены.");
        return;
    }

    // Simple selection - take first available or show dialog
    Currency curr = available.first();

    int row = m_currencyTable->rowCount();
    m_currencyTable->insertRow(row);

    QTableWidgetItem *nameItem = new QTableWidgetItem(QString("%1 (%2)").arg(curr.name(), curr.code()));
    nameItem->setData(Qt::UserRole, curr.id());
    nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
    m_currencyTable->setItem(row, 0, nameItem);

    QDoubleSpinBox *rateSpin = new QDoubleSpinBox(this);
    rateSpin->setRange(0.0001, 999999.9999);
    rateSpin->setDecimals(4);
    rateSpin->setValue(1.0);
    connect(rateSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AddSnapshotPage::onCurrencyRateChanged);
    m_currencyTable->setCellWidget(row, 1, rateSpin);

    QPushButton *delBtn = new QPushButton("✕", this);
    delBtn->setFixedSize(30, 25);
    delBtn->setCursor(Qt::PointingHandCursor);
    delBtn->setProperty("row", row);
    connect(delBtn, &QPushButton::clicked, this, &AddSnapshotPage::onDeleteCurrencyClicked);
    m_currencyTable->setCellWidget(row, 2, delBtn);

    // Update currency combos in positions
    for (int r = 0; r < m_positionsTable->rowCount(); ++r) {
        QComboBox *combo = qobject_cast<QComboBox*>(m_positionsTable->cellWidget(r, 5));
        if (combo) {
            combo->addItem(curr.code(), curr.id());
        }
    }
}

void AddSnapshotPage::onDeleteCurrencyClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    // Find row by button
    for (int row = 0; row < m_currencyTable->rowCount(); ++row) {
        if (m_currencyTable->cellWidget(row, 2) == btn) {
            int currId = m_currencyTable->item(row, 0)->data(Qt::UserRole).toInt();
            m_currencyTable->removeRow(row);

            // Remove from currency combos in positions
            for (int r = 0; r < m_positionsTable->rowCount(); ++r) {
                QComboBox *combo = qobject_cast<QComboBox*>(m_positionsTable->cellWidget(r, 5));
                if (combo) {
                    int idx = combo->findData(currId);
                    if (idx >= 0) {
                        if (combo->currentIndex() == idx) {
                            combo->setCurrentIndex(0);
                        }
                        combo->removeItem(idx);
                    }
                }
            }
            break;
        }
    }

    updateTotalSum();
}

void AddSnapshotPage::onCurrencyRateChanged()
{
    updateTotalSum();
}

void AddSnapshotPage::onAddPositionClicked()
{
    int row = m_positionsTable->rowCount();
    m_positionsTable->insertRow(row);

    // Name
    QLineEdit *nameEdit = new QLineEdit(this);
    nameEdit->setPlaceholderText("Название/тикер");
    m_positionsTable->setCellWidget(row, 0, nameEdit);

    // Category
    QComboBox *catCombo = new QComboBox(this);
    catCombo->addItem("—", -1);
    for (const auto& cat : m_categories) {
        catCombo->addItem(cat.name(), cat.id());
    }
    m_positionsTable->setCellWidget(row, 1, catCombo);

    // Price
    QDoubleSpinBox *priceSpin = new QDoubleSpinBox(this);
    priceSpin->setRange(0, 999999999.99);
    priceSpin->setDecimals(2);
    connect(priceSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AddSnapshotPage::onPositionChanged);
    m_positionsTable->setCellWidget(row, 2, priceSpin);

    // Quantity
    QDoubleSpinBox *qtySpin = new QDoubleSpinBox(this);
    qtySpin->setRange(0, 999999999.9999);
    qtySpin->setDecimals(4);
    connect(qtySpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AddSnapshotPage::onPositionChanged);
    m_positionsTable->setCellWidget(row, 3, qtySpin);

    // Country
    QComboBox *countryCombo = new QComboBox(this);
    countryCombo->addItem("—", -1);
    for (const auto& country : m_countries) {
        countryCombo->addItem(country.name(), country.id());
    }
    m_positionsTable->setCellWidget(row, 4, countryCombo);

    // Currency
    QComboBox *currCombo = new QComboBox(this);
    for (int r = 0; r < m_currencyTable->rowCount(); ++r) {
        int currId = m_currencyTable->item(r, 0)->data(Qt::UserRole).toInt();
        Currency curr = Database::instance().getCurrency(currId);
        if (curr.isValid()) {
            currCombo->addItem(curr.code(), curr.id());
        }
    }
    connect(currCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AddSnapshotPage::onPositionChanged);
    m_positionsTable->setCellWidget(row, 5, currCombo);

    // Sum (calculated, read-only)
    QTableWidgetItem *sumItem = new QTableWidgetItem("0.00");
    sumItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    sumItem->setFlags(sumItem->flags() & ~Qt::ItemIsEditable);
    m_positionsTable->setItem(row, 6, sumItem);

    // Delete button
    QPushButton *delBtn = new QPushButton("✕", this);
    delBtn->setFixedSize(30, 25);
    delBtn->setCursor(Qt::PointingHandCursor);
    connect(delBtn, &QPushButton::clicked, this, &AddSnapshotPage::onDeletePositionClicked);
    m_positionsTable->setCellWidget(row, 7, delBtn);
}

void AddSnapshotPage::onDeletePositionClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    for (int row = 0; row < m_positionsTable->rowCount(); ++row) {
        if (m_positionsTable->cellWidget(row, 7) == btn) {
            m_positionsTable->removeRow(row);
            break;
        }
    }

    updateTotalSum();
}

void AddSnapshotPage::onPositionChanged()
{
    updateTotalSum();
}

double AddSnapshotPage::getPositionSum(int row)
{
    QDoubleSpinBox *priceSpin = qobject_cast<QDoubleSpinBox*>(m_positionsTable->cellWidget(row, 2));
    QDoubleSpinBox *qtySpin = qobject_cast<QDoubleSpinBox*>(m_positionsTable->cellWidget(row, 3));
    QComboBox *currCombo = qobject_cast<QComboBox*>(m_positionsTable->cellWidget(row, 5));

    if (!priceSpin || !qtySpin || !currCombo) return 0;

    double price = priceSpin->value();
    double qty = qtySpin->value();
    int currId = currCombo->currentData().toInt();

    // Find rate
    double rate = 1.0;
    for (int r = 0; r < m_currencyTable->rowCount(); ++r) {
        if (m_currencyTable->item(r, 0)->data(Qt::UserRole).toInt() == currId) {
            QDoubleSpinBox *rateSpin = qobject_cast<QDoubleSpinBox*>(m_currencyTable->cellWidget(r, 1));
            if (rateSpin) rate = rateSpin->value();
            break;
        }
    }

    return price * qty * rate;
}

void AddSnapshotPage::updateTotalSum()
{
    double total = 0;

    for (int row = 0; row < m_positionsTable->rowCount(); ++row) {
        double sum = getPositionSum(row);

        QTableWidgetItem *sumItem = m_positionsTable->item(row, 6);
        if (sumItem) {
            sumItem->setText(QString::number(sum, 'f', 2));
        }

        total += sum;
    }

    m_totalLabel->setText(QString("Итого: %1 ₽").arg(total, 0, 'f', 2));
}

void AddSnapshotPage::onManageCategoriesClicked()
{
    InvestmentCategoryManagerDialog dialog(this);
    connect(&dialog, &InvestmentCategoryManagerDialog::categoriesChanged,
            this, &AddSnapshotPage::loadCategories);
    dialog.exec();
}

void AddSnapshotPage::onManageCountriesClicked()
{
    CountryManagerDialog dialog(this);
    connect(&dialog, &CountryManagerDialog::countriesChanged,
            this, &AddSnapshotPage::loadCountries);
    dialog.exec();
}

Snapshot AddSnapshotPage::buildSnapshot()
{
    Snapshot snapshot;
    snapshot.setDate(m_dateEdit->date());
    snapshot.setDescription(m_descriptionEdit->text().trimmed());

    // Currency rates
    for (int row = 0; row < m_currencyTable->rowCount(); ++row) {
        int currId = m_currencyTable->item(row, 0)->data(Qt::UserRole).toInt();
        QDoubleSpinBox *rateSpin = qobject_cast<QDoubleSpinBox*>(m_currencyTable->cellWidget(row, 1));
        if (rateSpin) {
            snapshot.setCurrencyRate(currId, rateSpin->value());
        }
    }

    // Positions
    for (int row = 0; row < m_positionsTable->rowCount(); ++row) {
        QLineEdit *nameEdit = qobject_cast<QLineEdit*>(m_positionsTable->cellWidget(row, 0));
        QComboBox *catCombo = qobject_cast<QComboBox*>(m_positionsTable->cellWidget(row, 1));
        QDoubleSpinBox *priceSpin = qobject_cast<QDoubleSpinBox*>(m_positionsTable->cellWidget(row, 2));
        QDoubleSpinBox *qtySpin = qobject_cast<QDoubleSpinBox*>(m_positionsTable->cellWidget(row, 3));
        QComboBox *countryCombo = qobject_cast<QComboBox*>(m_positionsTable->cellWidget(row, 4));
        QComboBox *currCombo = qobject_cast<QComboBox*>(m_positionsTable->cellWidget(row, 5));

        if (!nameEdit || !catCombo || !priceSpin || !qtySpin || !countryCombo || !currCombo) continue;

        SnapshotPosition pos;
        pos.setName(nameEdit->text().trimmed());
        pos.setCategoryId(catCombo->currentData().toInt());
        pos.setPrice(priceSpin->value());
        pos.setQuantity(qtySpin->value());
        pos.setCountryId(countryCombo->currentData().toInt());
        pos.setCurrencyId(currCombo->currentData().toInt());

        // Set rate for sum calculation
        pos.setCurrencyRate(snapshot.getCurrencyRate(pos.currencyId()));

        if (!pos.name().isEmpty() && pos.currencyId() >= 0) {
            snapshot.addPosition(pos);
        }
    }

    return snapshot;
}

void AddSnapshotPage::onSaveClicked()
{
    Snapshot snapshot = buildSnapshot();

    if (snapshot.positions().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Добавьте хотя бы одну позицию в портфель");
        return;
    }

    if (Database::instance().addSnapshot(snapshot)) {
        m_undoNotification->showNotification(snapshot);
        clearForm();
        loadFromPreviousSnapshot();
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось сохранить слепок");
    }
}

void AddSnapshotPage::onUndoClicked(const Snapshot& snapshot)
{
    if (Database::instance().deleteSnapshot(snapshot.id())) {
        loadFromPreviousSnapshot();
    }
}

void AddSnapshotPage::clearForm()
{
    m_dateEdit->setDate(QDate::currentDate());
    m_descriptionEdit->clear();
}
