#include "assetcard.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QPainter>

AssetCard::AssetCard(const PortfolioAsset& asset, QWidget *parent)
    : QWidget(parent)
    , m_asset(asset)
{
    setupUi();
    updateDisplay();
}

void AssetCard::setupUi()
{
    setFixedSize(280, 200);
    setObjectName("assetCard");

    // Shadow effect
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(15);
    shadow->setXOffset(0);
    shadow->setYOffset(2);
    shadow->setColor(QColor(0, 0, 0, 40));
    setGraphicsEffect(shadow);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 12, 15, 12);
    mainLayout->setSpacing(8);

    // Header: name + menu button
    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(0);

    m_nameLabel = new QLabel(this);
    m_nameLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #2c3e50;");
    m_nameLabel->setWordWrap(true);
    headerLayout->addWidget(m_nameLabel, 1);

    m_menuBtn = new QPushButton("⋮", this);
    m_menuBtn->setObjectName("cardMenuBtn");
    m_menuBtn->setFixedSize(24, 24);
    m_menuBtn->setCursor(Qt::PointingHandCursor);
    m_menuBtn->setStyleSheet(
        "QPushButton { border: none; background: transparent; font-size: 16px; color: #7f8c8d; }"
        "QPushButton:hover { color: #2c3e50; background: #ecf0f1; border-radius: 12px; }"
    );
    connect(m_menuBtn, &QPushButton::clicked, this, &AssetCard::onMenuButtonClicked);
    headerLayout->addWidget(m_menuBtn);

    // Menu
    m_menu = new QMenu(this);
    m_menu->setStyleSheet(
        "QMenu { background-color: white; border: 1px solid #ddd; border-radius: 6px; padding: 5px; }"
        "QMenu::item { padding: 8px 20px; color: #2c3e50; }"
        "QMenu::item:selected { background-color: #3498db; color: white; border-radius: 4px; }"
        "QMenu::separator { height: 1px; background: #ecf0f1; margin: 5px 10px; }"
    );
    m_menu->addAction("Купить", this, &AssetCard::onBuyClicked);
    m_menu->addAction("Продать", this, &AssetCard::onSellClicked);
    m_menu->addAction("История", this, &AssetCard::onHistoryClicked);
    m_menu->addSeparator();
    m_menu->addAction("Удалить", this, &AssetCard::onDeleteClicked);

    mainLayout->addLayout(headerLayout);

    // Info grid
    QGridLayout *infoGrid = new QGridLayout();
    infoGrid->setSpacing(4);
    infoGrid->setColumnStretch(0, 1);
    infoGrid->setColumnStretch(1, 1);

    // Row 0: Quantity | Avg Price
    QLabel *qtyTitle = new QLabel("Кол-во:", this);
    qtyTitle->setStyleSheet("font-size: 10px; color: #95a5a6;");
    m_quantityLabel = new QLabel(this);
    m_quantityLabel->setStyleSheet("font-size: 12px; color: #2c3e50;");

    QLabel *avgTitle = new QLabel("Ср. цена:", this);
    avgTitle->setStyleSheet("font-size: 10px; color: #95a5a6;");
    m_avgPriceLabel = new QLabel(this);
    m_avgPriceLabel->setStyleSheet("font-size: 12px; color: #2c3e50;");

    infoGrid->addWidget(qtyTitle, 0, 0);
    infoGrid->addWidget(m_quantityLabel, 1, 0);
    infoGrid->addWidget(avgTitle, 0, 1);
    infoGrid->addWidget(m_avgPriceLabel, 1, 1);

    // Row 1: Current Price (editable) | Value
    QLabel *priceTitle = new QLabel("Тек. цена:", this);
    priceTitle->setStyleSheet("font-size: 10px; color: #95a5a6;");

    // Price container for label and edit
    QWidget *priceContainer = new QWidget(this);
    QHBoxLayout *priceLayout = new QHBoxLayout(priceContainer);
    priceLayout->setContentsMargins(0, 0, 0, 0);
    priceLayout->setSpacing(0);

    m_currentPriceLabel = new QLabel(this);
    m_currentPriceLabel->setStyleSheet(
        "font-size: 12px; color: #3498db; font-weight: bold; padding: 2px 4px; border-radius: 3px;"
    );
    m_currentPriceLabel->setCursor(Qt::PointingHandCursor);
    m_currentPriceLabel->setToolTip("Нажмите для изменения");
    m_currentPriceLabel->installEventFilter(this);
    priceLayout->addWidget(m_currentPriceLabel);

    m_priceEdit = new QLineEdit(this);
    m_priceEdit->setFixedWidth(100);
    m_priceEdit->setStyleSheet(
        "font-size: 12px; font-weight: bold; color: #3498db; padding: 2px 4px; "
        "border: 1px solid #3498db; border-radius: 3px; background: white;"
    );
    m_priceEdit->hide();
    connect(m_priceEdit, &QLineEdit::editingFinished, this, &AssetCard::onPriceEditFinished);
    priceLayout->addWidget(m_priceEdit);

    priceLayout->addStretch();

    QLabel *valueTitle = new QLabel("Стоимость:", this);
    valueTitle->setStyleSheet("font-size: 10px; color: #95a5a6;");
    m_valueLabel = new QLabel(this);
    m_valueLabel->setStyleSheet("font-size: 12px; color: #2c3e50; font-weight: bold;");

    infoGrid->addWidget(priceTitle, 2, 0);
    infoGrid->addWidget(priceContainer, 3, 0);
    infoGrid->addWidget(valueTitle, 2, 1);
    infoGrid->addWidget(m_valueLabel, 3, 1);

    // Row 2: Profit % | Profit RUB
    QLabel *profitPctTitle = new QLabel("Доходность:", this);
    profitPctTitle->setStyleSheet("font-size: 10px; color: #95a5a6;");
    m_profitPercentLabel = new QLabel(this);

    QLabel *profitRubTitle = new QLabel("Прибыль:", this);
    profitRubTitle->setStyleSheet("font-size: 10px; color: #95a5a6;");
    m_profitRubLabel = new QLabel(this);

    infoGrid->addWidget(profitPctTitle, 4, 0);
    infoGrid->addWidget(m_profitPercentLabel, 5, 0);
    infoGrid->addWidget(profitRubTitle, 4, 1);
    infoGrid->addWidget(m_profitRubLabel, 5, 1);

    mainLayout->addLayout(infoGrid);

    // Footer: Currency | Country
    QHBoxLayout *footerLayout = new QHBoxLayout();
    m_currencyLabel = new QLabel(this);
    m_currencyLabel->setStyleSheet("font-size: 10px; color: #7f8c8d;");
    m_countryLabel = new QLabel(this);
    m_countryLabel->setStyleSheet("font-size: 10px; color: #7f8c8d;");
    m_countryLabel->setAlignment(Qt::AlignRight);

    footerLayout->addWidget(m_currencyLabel);
    footerLayout->addStretch();
    footerLayout->addWidget(m_countryLabel);

    mainLayout->addStretch();
    mainLayout->addLayout(footerLayout);
}

void AssetCard::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Card background
    QRect cardRect = rect();
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255));
    painter.drawRoundedRect(cardRect, 12, 12);

    // Subtle border
    painter.setPen(QPen(QColor(0, 0, 0, 15), 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(cardRect.adjusted(0, 0, -1, -1), 12, 12);
}

void AssetCard::updateAsset(const PortfolioAsset& asset)
{
    m_asset = asset;
    updateDisplay();
}

void AssetCard::updateDisplay()
{
    m_nameLabel->setText(m_asset.name());
    m_quantityLabel->setText(QString::number(m_asset.totalQuantity(), 'f', 2));
    m_avgPriceLabel->setText(QString::number(m_asset.averageBuyPrice(), 'f', 6));
    m_currentPriceLabel->setText(QString::number(m_asset.currentPrice(), 'f', 6));
    m_valueLabel->setText(QString::number(m_asset.currentValue(), 'f', 2));

    double yieldPct = m_asset.yieldPercent();
    double profitRub = m_asset.profitInRub();

    // Color based on profit
    QString profitColor = yieldPct >= 0 ? "#27ae60" : "#e74c3c";
    QString profitSign = yieldPct >= 0 ? "+" : "";

    m_profitPercentLabel->setText(QString("%1%2%").arg(profitSign).arg(yieldPct, 0, 'f', 2));
    m_profitPercentLabel->setStyleSheet(QString("font-size: 12px; font-weight: bold; color: %1;").arg(profitColor));

    m_profitRubLabel->setText(QString("%1%2 ₽").arg(profitSign).arg(profitRub, 0, 'f', 2));
    m_profitRubLabel->setStyleSheet(QString("font-size: 12px; font-weight: bold; color: %1;").arg(profitColor));

    m_currencyLabel->setText(m_asset.currencyCode());
    m_countryLabel->setText(m_asset.countryName());
}


bool AssetCard::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_currentPriceLabel && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            onPriceLabelClicked();
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void AssetCard::onMenuButtonClicked()
{
    QPoint pos = m_menuBtn->mapToGlobal(QPoint(0, m_menuBtn->height()));
    m_menu->exec(pos);
}

void AssetCard::onBuyClicked()
{
    emit buyRequested(m_asset.id());
}

void AssetCard::onSellClicked()
{
    emit sellRequested(m_asset.id());
}

void AssetCard::onHistoryClicked()
{
    emit historyRequested(m_asset.id());
}

void AssetCard::onDeleteClicked()
{
    emit deleteRequested(m_asset.id());
}

void AssetCard::onPriceLabelClicked()
{
    m_currentPriceLabel->hide();
    m_priceEdit->setText(QString::number(m_asset.currentPrice(), 'f', 6));
    m_priceEdit->show();
    m_priceEdit->setFocus();
    m_priceEdit->selectAll();
}

void AssetCard::onPriceEditFinished()
{
    m_priceEdit->hide();
    m_currentPriceLabel->show();

    bool ok;
    double newPrice = m_priceEdit->text().replace(',', '.').toDouble(&ok);
    if (ok && newPrice > 0 && newPrice != m_asset.currentPrice()) {
        emit priceChanged(m_asset.id(), newPrice);
    }
}
