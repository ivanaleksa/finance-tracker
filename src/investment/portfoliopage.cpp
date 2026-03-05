#include "investment/portfoliopage.h"
#include "investment/addassetdialog.h"
#include "investment/assethistorydialog.h"
#include "investment/addoperationdialog.h"
#include "investment/createsnapshotdialog.h"
#include "widgets/flowlayout.h"
#include "database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFrame>
#include <QScrollBar>

PortfolioPage::PortfolioPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    loadCurrencyRates();
    loadAssets();

    connect(&Database::instance(), &Database::portfolioDataChanged,
            this, &PortfolioPage::refreshData);
    connect(&Database::instance(), &Database::investmentDataChanged,
            this, &PortfolioPage::refreshData);
}

void PortfolioPage::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 30, 40, 30);
    mainLayout->setSpacing(20);

    // Title
    QLabel *titleLabel = new QLabel("Портфель", this);
    titleLabel->setObjectName("pageTitle");
    mainLayout->addWidget(titleLabel);

    // Action buttons row
    QHBoxLayout *actionLayout = new QHBoxLayout();

    m_addAssetBtn = new QPushButton("Добавить актив", this);
    m_addAssetBtn->setObjectName("primaryButton");
    m_addAssetBtn->setCursor(Qt::PointingHandCursor);
    connect(m_addAssetBtn, &QPushButton::clicked, this, &PortfolioPage::onAddAssetClicked);
    actionLayout->addWidget(m_addAssetBtn);

    m_createSnapshotBtn = new QPushButton("Сделать снимок", this);
    m_createSnapshotBtn->setObjectName("additionalyButton");
    m_createSnapshotBtn->setCursor(Qt::PointingHandCursor);
    connect(m_createSnapshotBtn, &QPushButton::clicked, this, &PortfolioPage::onCreateSnapshotClicked);
    actionLayout->addWidget(m_createSnapshotBtn);

    actionLayout->addStretch();

    // Currencies button
    m_currenciesBtn = new QPushButton("$ ₽", this);
    m_currenciesBtn->setObjectName("currenciesButton");
    m_currenciesBtn->setCursor(Qt::PointingHandCursor);
    m_currenciesBtn->setToolTip("Управление валютами");
    m_currenciesBtn->setStyleSheet(
        "QPushButton { "
        "   border: 2px solid #3498db; "
        "   border-radius: 8px; "
        "   background: transparent; "
        "   color: #3498db; "
        "   font-size: 16px; "
        "   font-weight: bold; "
        "   padding: 8px 16px; "
        "} "
        "QPushButton:hover { "
        "   background: #3498db; "
        "   color: white; "
        "}"
    );
    connect(m_currenciesBtn, &QPushButton::clicked, this, &PortfolioPage::onCurrenciesClicked);
    actionLayout->addWidget(m_currenciesBtn);

    mainLayout->addLayout(actionLayout);

    // Scroll area for cards
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setStyleSheet("QScrollArea { background: transparent; }");

    m_cardsContainer = new QWidget();
    m_cardsContainer->setStyleSheet("background: transparent;");
    m_cardsLayout = new QVBoxLayout(m_cardsContainer);
    m_cardsLayout->setContentsMargins(4, 4, 4, 4);
    m_cardsLayout->setSpacing(20);
    m_cardsLayout->addStretch();

    m_scrollArea->setWidget(m_cardsContainer);
    mainLayout->addWidget(m_scrollArea, 1);
}

void PortfolioPage::refreshData()
{
    loadCurrencyRates();
    loadAssets();
}

void PortfolioPage::loadCurrencyRates()
{
    m_currencyRates.clear();

    // Load rates directly from currencies table
    QList<Currency> currencies = Database::instance().getCurrencies();
    for (const Currency& curr : currencies) {
        m_currencyRates[curr.id()] = curr.rate();
    }
}

void PortfolioPage::setCurrencyRates(const QMap<int, double>& rates)
{
    m_currencyRates = rates;
    loadAssets();
}

void PortfolioPage::clearCards()
{
    for (AssetCard *card : m_assetCards) {
        card->deleteLater();
    }
    m_assetCards.clear();

    // Remove all widgets from layout except the stretch
    while (m_cardsLayout->count() > 1) {
        QLayoutItem *item = m_cardsLayout->takeAt(0);
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
}

void PortfolioPage::loadAssets()
{
    clearCards();

    QList<PortfolioAsset> assets = Database::instance().getActivePortfolioAssets();

    // Group assets by category
    QMap<QString, QList<PortfolioAsset>> assetsByCategory;

    for (const PortfolioAsset& assetConst : assets) {
        PortfolioAsset asset = assetConst;

        // Skip assets with zero quantity
        if (asset.totalQuantity() <= 0) {
            continue;
        }

        // Set currency rate from our cache
        double rate = m_currencyRates.value(asset.currencyId(), 1.0);
        asset.setCurrencyRate(rate);

        QString category = asset.categoryName().isEmpty() ? "Без категории" : asset.categoryName();
        assetsByCategory[category].append(asset);
    }

    // Create category sections
    if (assetsByCategory.count() == 0) {
        QLabel *emptyLabel = new QLabel("Активы отсутствуют", this);
        emptyLabel->setStyleSheet("font-size: 14px; color: #7f8c8d; padding: 40px;");
        emptyLabel->setAlignment(Qt::AlignCenter);
        m_cardsLayout->insertWidget(0, emptyLabel);
        return;
    }
    else {
        for (auto it = assetsByCategory.begin(); it != assetsByCategory.end(); ++it) {
            QString categoryName = it.key();
            QList<PortfolioAsset> categoryAssets = it.value();

            // Category header
            QWidget *headerWidget = new QWidget();
            QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
            headerLayout->setContentsMargins(0, 10, 0, 5);

            QLabel *categoryLabel = new QLabel(categoryName, headerWidget);
            categoryLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #2c3e50;");
            headerLayout->addWidget(categoryLabel);

            // Separator line
            QFrame *line = new QFrame(headerWidget);
            line->setFrameShape(QFrame::HLine);
            line->setStyleSheet("background-color: #bdc3c7;");
            line->setFixedHeight(1);
            headerLayout->addWidget(line, 1);

            // Insert before the stretch
            m_cardsLayout->insertWidget(m_cardsLayout->count() - 1, headerWidget);

            // Cards container for this category with flow layout (auto wrap)
            QWidget *cardsRow = new QWidget();
            FlowLayout *rowLayout = new FlowLayout(cardsRow, 8, 15, 15);

            // Create cards for each asset in this category
            for (const PortfolioAsset& asset : categoryAssets) {
                AssetCard *card = new AssetCard(asset, cardsRow);

                connect(card, &AssetCard::buyRequested, this, &PortfolioPage::onAssetBuyRequested);
                connect(card, &AssetCard::sellRequested, this, &PortfolioPage::onAssetSellRequested);
                connect(card, &AssetCard::historyRequested, this, &PortfolioPage::onAssetHistoryRequested);
                connect(card, &AssetCard::deleteRequested, this, &PortfolioPage::onAssetDeleteRequested);
                connect(card, &AssetCard::priceChanged, this, &PortfolioPage::onAssetPriceChanged);

                rowLayout->addWidget(card);
                m_assetCards.append(card);
            }

            // Insert before the stretch
            m_cardsLayout->insertWidget(m_cardsLayout->count() - 1, cardsRow);
        }
    }
}

void PortfolioPage::onAddAssetClicked()
{
    AddAssetDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        PortfolioAsset asset = dialog.getAsset();
        AssetOperation operation = dialog.getFirstOperation();

        if (Database::instance().addPortfolioAsset(asset)) {
            operation.setAssetId(asset.id());
            if (!Database::instance().addAssetOperation(operation)) {
                QMessageBox::warning(this, "Предупреждение",
                    "Актив добавлен, но не удалось добавить первую операцию");
            }
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось добавить актив");
        }
    }
}

void PortfolioPage::onCreateSnapshotClicked()
{
    CreateSnapshotDialog dialog(m_currencyRates, this);
    dialog.exec();
}

void PortfolioPage::onCurrenciesClicked()
{
    emit currenciesPageRequested();
}

void PortfolioPage::onAssetHistoryRequested(int assetId)
{
    AssetHistoryDialog dialog(assetId, this);
    dialog.exec();
}

void PortfolioPage::onAssetBuyRequested(int assetId)
{
    AddOperationDialog dialog(assetId, AssetOperation::Type::Buy, 0, this);
    if (dialog.exec() == QDialog::Accepted) {
        AssetOperation op = dialog.getOperation();
        if (!Database::instance().addAssetOperation(op)) {
            QMessageBox::critical(this, "Ошибка", "Не удалось добавить операцию покупки");
        }
    }
}

void PortfolioPage::onAssetSellRequested(int assetId)
{
    double maxQty = Database::instance().getAssetTotalQuantity(assetId);
    if (maxQty <= 0) {
        QMessageBox::warning(this, "Предупреждение", "Нет активов для продажи");
        return;
    }

    AddOperationDialog dialog(assetId, AssetOperation::Type::Sell, maxQty, this);
    if (dialog.exec() == QDialog::Accepted) {
        AssetOperation op = dialog.getOperation();
        if (!Database::instance().addAssetOperation(op)) {
            QMessageBox::critical(this, "Ошибка", "Не удалось добавить операцию продажи");
        }
    }
}

void PortfolioPage::onAssetDeleteRequested(int assetId)
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Подтверждение",
        "Вы уверены, что хотите удалить этот актив?\nВсе операции по нему также будут удалены.",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        if (!Database::instance().deletePortfolioAsset(assetId)) {
            QMessageBox::critical(this, "Ошибка", "Не удалось удалить актив");
        }
    }
}

void PortfolioPage::onAssetPriceChanged(int assetId, double newPrice)
{
    PortfolioAsset asset = Database::instance().getPortfolioAsset(assetId);
    if (asset.isValid()) {
        asset.setCurrentPrice(newPrice);
        if (!Database::instance().updatePortfolioAsset(asset)) {
            QMessageBox::critical(this, "Ошибка", "Не удалось обновить цену");
        }
    }
}
