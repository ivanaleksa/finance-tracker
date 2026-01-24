#include "currenciespage.h"
#include "currencycard.h"
#include "database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QScrollArea>

CurrenciesPage::CurrenciesPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();

    // Load default rates from latest snapshot
    Snapshot latestSnapshot = Database::instance().getLatestSnapshot();
    m_currencyRates = latestSnapshot.currencyRates();

    // Ensure RUB has rate 1
    QList<Currency> currencies = Database::instance().getCurrencies();
    for (const Currency& curr : currencies) {
        if (curr.code() == "RUB") {
            m_currencyRates[curr.id()] = 1.0;
        } else if (!m_currencyRates.contains(curr.id())) {
            m_currencyRates[curr.id()] = 100.0; // Default rate
        }
    }

    loadCurrencies();

    connect(&Database::instance(), &Database::investmentDataChanged,
            this, &CurrenciesPage::refreshData);
}

void CurrenciesPage::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 30, 40, 30);
    mainLayout->setSpacing(20);

    // Top bar with back button and add button
    QHBoxLayout *topLayout = new QHBoxLayout();

    // Back button (left)
    m_backBtn = new QPushButton("← Назад", this);
    m_backBtn->setObjectName("backButton");
    m_backBtn->setCursor(Qt::PointingHandCursor);
    m_backBtn->setStyleSheet(
        "QPushButton { "
        "   border: none; "
        "   background: transparent; "
        "   color: #3498db; "
        "   font-size: 14px; "
        "   padding: 8px 12px; "
        "} "
        "QPushButton:hover { "
        "   color: #2980b9; "
        "   background: #ecf0f1; "
        "   border-radius: 6px; "
        "}"
    );
    connect(m_backBtn, &QPushButton::clicked, this, &CurrenciesPage::onBackClicked);
    topLayout->addWidget(m_backBtn);

    topLayout->addStretch();

    // Title (center)
    QLabel *titleLabel = new QLabel("Валюты", this);
    titleLabel->setObjectName("pageTitle");
    topLayout->addWidget(titleLabel);

    topLayout->addStretch();

    // Add currency button (right)
    m_addBtn = new QPushButton("+", this);
    m_addBtn->setObjectName("primaryButton");
    m_addBtn->setCursor(Qt::PointingHandCursor);
    m_addBtn->setFixedSize(40, 40);
    connect(m_addBtn, &QPushButton::clicked, this, &CurrenciesPage::onAddCurrencyClicked);
    topLayout->addWidget(m_addBtn);

    mainLayout->addLayout(topLayout);

    // Scroll area for cards
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setStyleSheet("QScrollArea { background: transparent; }");

    m_cardsContainer = new QWidget();
    m_cardsContainer->setStyleSheet("background: transparent;");
    m_scrollArea->setWidget(m_cardsContainer);

    mainLayout->addWidget(m_scrollArea, 1);
}

void CurrenciesPage::refreshData()
{
    loadCurrencies();
}

void CurrenciesPage::loadCurrencies()
{
    // Clear existing cards
    qDeleteAll(m_cards);
    m_cards.clear();

    if (m_cardsContainer->layout()) {
        delete m_cardsContainer->layout();
    }

    QGridLayout *gridLayout = new QGridLayout(m_cardsContainer);
    gridLayout->setContentsMargins(10, 10, 10, 10);
    gridLayout->setSpacing(20);

    QList<Currency> currencies = Database::instance().getCurrencies();

    int col = 0;
    int row = 0;
    const int maxCols = 4;

    for (const Currency& curr : currencies) {
        double rate = m_currencyRates.value(curr.id(), curr.code() == "RUB" ? 1.0 : 100.0);

        CurrencyCard *card = new CurrencyCard(curr, rate, m_cardsContainer);
        connect(card, &CurrencyCard::rateChanged, this, &CurrenciesPage::onRateChanged);
        connect(card, &CurrencyCard::renameRequested, this, &CurrenciesPage::onRenameRequested);
        connect(card, &CurrencyCard::deleteRequested, this, &CurrenciesPage::onDeleteRequested);

        gridLayout->addWidget(card, row, col);
        m_cards.append(card);

        col++;
        if (col >= maxCols) {
            col = 0;
            row++;
        }
    }

    // Add stretch to push cards to top-left
    gridLayout->setRowStretch(row + 1, 1);
    gridLayout->setColumnStretch(maxCols, 1);
}

void CurrenciesPage::setCurrencyRates(const QMap<int, double>& rates)
{
    m_currencyRates = rates;

    // Update cards
    for (CurrencyCard *card : m_cards) {
        if (m_currencyRates.contains(card->currencyId())) {
            card->setRate(m_currencyRates[card->currencyId()]);
        }
    }
}

void CurrenciesPage::onBackClicked()
{
    emit backRequested();
}

void CurrenciesPage::onAddCurrencyClicked()
{
    bool ok;
    QString code = QInputDialog::getText(this, "Новая валюта",
                                         "Код валюты (например, EUR):",
                                         QLineEdit::Normal, QString(), &ok);
    if (!ok || code.trimmed().isEmpty()) {
        return;
    }

    code = code.trimmed().toUpper();

    QString name = QInputDialog::getText(this, "Новая валюта",
                                         "Название валюты:",
                                         QLineEdit::Normal, QString(), &ok);
    if (!ok || name.trimmed().isEmpty()) {
        return;
    }

    Currency currency;
    currency.setCode(code);
    currency.setName(name.trimmed());

    if (Database::instance().addCurrency(currency)) {
        m_currencyRates[currency.id()] = 1.0;
        loadCurrencies();
        emit currencyRatesChanged();
    } else {
        QMessageBox::warning(this, "Ошибка",
                            "Не удалось добавить валюту.\nВозможно, такой код уже существует.");
    }
}

void CurrenciesPage::onRateChanged(int currencyId, double newRate)
{
    m_currencyRates[currencyId] = newRate;
    emit currencyRatesChanged();
}

void CurrenciesPage::onRenameRequested(int currencyId)
{
    Currency currency = Database::instance().getCurrency(currencyId);
    if (!currency.isValid()) {
        return;
    }

    bool ok;
    QString newName = QInputDialog::getText(this, "Переименовать валюту",
                                            "Новое название:",
                                            QLineEdit::Normal,
                                            currency.name(), &ok);
    if (ok && !newName.trimmed().isEmpty()) {
        if (Database::instance().updateCurrency(currencyId, currency.code(), newName.trimmed())) {
            loadCurrencies();
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось переименовать валюту.");
        }
    }
}

void CurrenciesPage::onDeleteRequested(int currencyId)
{
    Currency currency = Database::instance().getCurrency(currencyId);
    if (!currency.isValid()) {
        return;
    }

    if (currency.code() == "RUB") {
        QMessageBox::warning(this, "Ошибка", "Нельзя удалить базовую валюту (RUB).");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Удаление валюты",
        QString("Вы уверены, что хотите удалить валюту %1 (%2)?")
            .arg(currency.code(), currency.name()),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (Database::instance().deleteCurrency(currencyId)) {
            m_currencyRates.remove(currencyId);
            loadCurrencies();
            emit currencyRatesChanged();
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось удалить валюту.");
        }
    }
}
