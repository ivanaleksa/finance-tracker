#include "investment/currencycard.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QEvent>

CurrencyCard::CurrencyCard(const Currency& currency, double rate, QWidget *parent)
    : QWidget(parent)
    , m_currency(currency)
    , m_rate(rate)
{
    setupUi();
}

void CurrencyCard::setupUi()
{
    setFixedSize(200, 120);
    setObjectName("currencyCard");

    // Shadow effect
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(15);
    shadow->setXOffset(0);
    shadow->setYOffset(2);
    shadow->setColor(QColor(0, 0, 0, 40));
    setGraphicsEffect(shadow);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 12, 15, 15);
    mainLayout->setSpacing(8);

    // Top row: code + menu button
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->setSpacing(0);

    m_codeLabel = new QLabel(m_currency.code(), this);
    m_codeLabel->setObjectName("currencyCode");
    m_codeLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50;");
    topLayout->addWidget(m_codeLabel);

    topLayout->addStretch();

    m_menuBtn = new QPushButton("⋮", this);
    m_menuBtn->setObjectName("cardMenuBtn");
    m_menuBtn->setFixedSize(24, 24);
    m_menuBtn->setCursor(Qt::PointingHandCursor);
    m_menuBtn->setStyleSheet(
        "QPushButton { border: none; background: transparent; font-size: 16px; color: #7f8c8d; }"
        "QPushButton:hover { color: #2c3e50; background: #ecf0f1; border-radius: 12px; }"
    );
    connect(m_menuBtn, &QPushButton::clicked, this, &CurrencyCard::onMenuClicked);
    topLayout->addWidget(m_menuBtn);

    mainLayout->addLayout(topLayout);

    // Name
    m_nameLabel = new QLabel(m_currency.name(), this);
    m_nameLabel->setObjectName("currencyName");
    m_nameLabel->setStyleSheet("font-size: 12px; color: #7f8c8d;");
    mainLayout->addWidget(m_nameLabel);

    mainLayout->addStretch();

    // Rate section
    QHBoxLayout *rateLayout = new QHBoxLayout();
    rateLayout->setSpacing(5);

    QLabel *rateTitle = new QLabel("Курс:", this);
    rateTitle->setStyleSheet("font-size: 11px; color: #95a5a6;");
    rateLayout->addWidget(rateTitle);

    // Clickable rate label
    m_rateLabel = new QLabel(this);
    m_rateLabel->setText(QString::number(m_rate, 'f', 4));
    m_rateLabel->setObjectName("currencyRate");
    m_rateLabel->setCursor(Qt::PointingHandCursor);
    m_rateLabel->setStyleSheet(
        "font-size: 14px; font-weight: bold; color: #27ae60; padding: 2px 4px; border-radius: 3px;"
    );
    m_rateLabel->setToolTip("Нажмите для изменения");
    m_rateLabel->installEventFilter(this);
    rateLayout->addWidget(m_rateLabel);

    // Hidden rate edit
    m_rateEdit = new QLineEdit(this);
    m_rateEdit->setObjectName("currencyRateEdit");
    m_rateEdit->setFixedWidth(80);
    m_rateEdit->setStyleSheet(
        "font-size: 14px; font-weight: bold; color: #27ae60; padding: 2px 4px; "
        "border: 1px solid #27ae60; border-radius: 3px; background: white;"
    );
    m_rateEdit->hide();
    connect(m_rateEdit, &QLineEdit::editingFinished, this, &CurrencyCard::onRateEditFinished);
    rateLayout->addWidget(m_rateEdit);

    rateLayout->addStretch();
    mainLayout->addLayout(rateLayout);

    // Context menu
    m_menu = new QMenu(this);
    m_menu->setStyleSheet(
        "QMenu { background-color: white; border: 1px solid #ddd; border-radius: 6px; padding: 5px; }"
        "QMenu::item { padding: 8px 20px; color: #2c3e50; }"
        "QMenu::item:selected { background-color: #3498db; color: white; border-radius: 4px; }"
        "QMenu::separator { height: 1px; background: #ecf0f1; margin: 5px 10px; }"
    );

    QAction *renameAction = m_menu->addAction("Изменить название");
    connect(renameAction, &QAction::triggered, this, &CurrencyCard::onRenameClicked);

    m_menu->addSeparator();

    QAction *deleteAction = m_menu->addAction("Удалить");
    connect(deleteAction, &QAction::triggered, this, &CurrencyCard::onDeleteClicked);
}

bool CurrencyCard::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_rateLabel && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            onRateLabelClicked();
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void CurrencyCard::paintEvent(QPaintEvent *event)
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

void CurrencyCard::setRate(double rate)
{
    m_rate = rate;
    m_rateLabel->setText(QString::number(m_rate, 'f', 4));
}

void CurrencyCard::onMenuClicked()
{
    QPoint pos = m_menuBtn->mapToGlobal(QPoint(0, m_menuBtn->height()));
    m_menu->exec(pos);
}

void CurrencyCard::onRenameClicked()
{
    emit renameRequested(m_currency.id());
}

void CurrencyCard::onDeleteClicked()
{
    emit deleteRequested(m_currency.id());
}

void CurrencyCard::onRateLabelClicked()
{
    m_rateLabel->hide();
    m_rateEdit->setText(QString::number(m_rate, 'f', 4));
    m_rateEdit->show();
    m_rateEdit->setFocus();
    m_rateEdit->selectAll();
}

void CurrencyCard::onRateEditFinished()
{
    m_rateEdit->hide();
    m_rateLabel->show();

    bool ok;
    double newRate = m_rateEdit->text().replace(',', '.').toDouble(&ok);
    if (ok && newRate > 0 && newRate != m_rate) {
        m_rate = newRate;
        m_rateLabel->setText(QString::number(m_rate, 'f', 4));
        emit rateChanged(m_currency.id(), m_rate);
    }
}
