#include "investment/snapshotitemwidget.h"
#include "database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QMessageBox>

SnapshotItemWidget::SnapshotItemWidget(const Snapshot& snapshot, QWidget *parent)
    : QWidget(parent)
    , m_snapshot(snapshot)
{
    setupUi();
    updateExpandedState();
}

void SnapshotItemWidget::setupUi()
{
    setObjectName("snapshotItem");

    // Shadow effect
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(10);
    shadow->setXOffset(0);
    shadow->setYOffset(2);
    shadow->setColor(QColor(0, 0, 0, 30));
    setGraphicsEffect(shadow);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Header widget (always visible)
    m_headerWidget = new QWidget(this);
    m_headerWidget->setCursor(Qt::PointingHandCursor);
    QHBoxLayout *headerLayout = new QHBoxLayout(m_headerWidget);
    headerLayout->setContentsMargins(20, 15, 20, 15);
    headerLayout->setSpacing(15);

    // Date
    m_dateLabel = new QLabel(m_snapshot.date().toString("dd.MM.yyyy"), this);
    m_dateLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #2c3e50;");
    m_dateLabel->setFixedWidth(100);
    headerLayout->addWidget(m_dateLabel);

    // Description (truncated in header)
    m_fullDescription = m_snapshot.description().isEmpty() ? "—" : m_snapshot.description();
    m_descriptionLabel = new QLabel(this);
    m_descriptionLabel->setText(m_fullDescription);
    m_descriptionLabel->setStyleSheet("font-size: 13px; color: #7f8c8d;");
    m_descriptionLabel->setTextFormat(Qt::PlainText);
    m_descriptionLabel->setWordWrap(false);
    // Elide text if too long
    QFontMetrics fm(m_descriptionLabel->font());
    m_descriptionLabel->setMinimumWidth(50);
    m_descriptionLabel->setMaximumWidth(300);
    QString elidedText = fm.elidedText(m_fullDescription, Qt::ElideRight, 280);
    m_descriptionLabel->setText(elidedText);
    headerLayout->addWidget(m_descriptionLabel, 1);

    // Total value
    double total = calculateTotalValue();
    m_totalLabel = new QLabel(QString("%1 ₽").arg(total, 0, 'f', 2), this);
    m_totalLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #27ae60;");
    m_totalLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    headerLayout->addWidget(m_totalLabel);

    // Delete button
    m_deleteBtn = new QPushButton("🗑", this);
    m_deleteBtn->setFixedSize(30, 30);
    m_deleteBtn->setCursor(Qt::PointingHandCursor);
    m_deleteBtn->setStyleSheet(
        "QPushButton { border: none; background: transparent; font-size: 14px; }"
        "QPushButton:hover { background: #ffebee; border-radius: 15px; }"
    );
    m_deleteBtn->setToolTip("Удалить снимок");
    connect(m_deleteBtn, &QPushButton::clicked, this, &SnapshotItemWidget::onDeleteClicked);
    headerLayout->addWidget(m_deleteBtn);

    // Expand button
    m_expandBtn = new QPushButton("▼", this);
    m_expandBtn->setFixedSize(30, 30);
    m_expandBtn->setCursor(Qt::PointingHandCursor);
    m_expandBtn->setStyleSheet(
        "QPushButton { border: none; background: transparent; font-size: 12px; color: #7f8c8d; }"
        "QPushButton:hover { color: #2c3e50; }"
    );
    connect(m_expandBtn, &QPushButton::clicked, this, &SnapshotItemWidget::toggleExpanded);
    headerLayout->addWidget(m_expandBtn);

    mainLayout->addWidget(m_headerWidget);

    // Content widget (expandable)
    m_contentWidget = new QWidget(this);
    QVBoxLayout *contentLayout = new QVBoxLayout(m_contentWidget);
    contentLayout->setContentsMargins(20, 10, 20, 20);
    contentLayout->setSpacing(15);

    // Separator line
    QFrame *separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("background-color: #ecf0f1;");
    separator->setFixedHeight(1);
    contentLayout->addWidget(separator);

    // Full description (shown when expanded)
    if (!m_snapshot.description().isEmpty()) {
        m_fullDescriptionLabel = new QLabel(this);
        m_fullDescriptionLabel->setText(m_fullDescription);
        m_fullDescriptionLabel->setStyleSheet("font-size: 13px; color: #7f8c8d; padding: 5px 0;");
        m_fullDescriptionLabel->setWordWrap(true);
        contentLayout->addWidget(m_fullDescriptionLabel);
    } else {
        m_fullDescriptionLabel = nullptr;
    }

    // Positions table
    QLabel *positionsTitle = new QLabel("Позиции", this);
    positionsTitle->setStyleSheet("font-size: 13px; font-weight: bold; color: #2c3e50;");
    contentLayout->addWidget(positionsTitle);

    m_positionsTable = new QTableWidget(this);
    m_positionsTable->setColumnCount(5);
    m_positionsTable->setHorizontalHeaderLabels({"Актив", "Категория", "Кол-во", "Цена", "Стоимость"});
    m_positionsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_positionsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_positionsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_positionsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_positionsTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_positionsTable->verticalHeader()->setVisible(false);
    m_positionsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_positionsTable->setSelectionMode(QAbstractItemView::NoSelection);
    m_positionsTable->setAlternatingRowColors(true);
    m_positionsTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_positionsTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Fill positions
    QList<SnapshotPosition> positions = m_snapshot.positions();
    m_positionsTable->setRowCount(positions.size());
    for (int i = 0; i < positions.size(); ++i) {
        const SnapshotPosition& pos = positions[i];

        m_positionsTable->setItem(i, 0, new QTableWidgetItem(pos.name()));

        QString catName = pos.categoryId() > 0
            ? Database::instance().getInvestmentCategory(pos.categoryId()).name()
            : "—";
        m_positionsTable->setItem(i, 1, new QTableWidgetItem(catName));

        QTableWidgetItem *qtyItem = new QTableWidgetItem(QString::number(pos.quantity(), 'f', 2));
        qtyItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_positionsTable->setItem(i, 2, qtyItem);

        Currency curr = Database::instance().getCurrency(pos.currencyId());
        QString priceStr = QString("%1 %2").arg(pos.price(), 0, 'f', 2).arg(curr.code());
        QTableWidgetItem *priceItem = new QTableWidgetItem(priceStr);
        priceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_positionsTable->setItem(i, 3, priceItem);

        double rate = m_snapshot.currencyRates().value(pos.currencyId(), 1.0);
        double valueRub = pos.price() * pos.quantity() * rate;
        QTableWidgetItem *valueItem = new QTableWidgetItem(QString("%1 ₽").arg(valueRub, 0, 'f', 2));
        valueItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_positionsTable->setItem(i, 4, valueItem);
    }

    // Auto-size table height
    int posTableHeight = m_positionsTable->horizontalHeader()->height() + 2;
    for (int i = 0; i < m_positionsTable->rowCount(); ++i) {
        posTableHeight += m_positionsTable->rowHeight(i);
    }
    m_positionsTable->setFixedHeight(posTableHeight);

    contentLayout->addWidget(m_positionsTable);

    // Currency rates table
    QLabel *currenciesTitle = new QLabel("Курсы валют", this);
    currenciesTitle->setStyleSheet("font-size: 13px; font-weight: bold; color: #2c3e50;");
    contentLayout->addWidget(currenciesTitle);

    m_currenciesTable = new QTableWidget(this);
    m_currenciesTable->setColumnCount(2);
    m_currenciesTable->setHorizontalHeaderLabels({"Валюта", "Курс к RUB"});
    m_currenciesTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_currenciesTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_currenciesTable->verticalHeader()->setVisible(false);
    m_currenciesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_currenciesTable->setSelectionMode(QAbstractItemView::NoSelection);
    m_currenciesTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_currenciesTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Fill currency rates
    QMap<int, double> rates = m_snapshot.currencyRates();
    m_currenciesTable->setRowCount(rates.size());
    int row = 0;
    for (auto it = rates.begin(); it != rates.end(); ++it, ++row) {
        Currency curr = Database::instance().getCurrency(it.key());
        m_currenciesTable->setItem(row, 0, new QTableWidgetItem(curr.code() + " - " + curr.name()));

        QTableWidgetItem *rateItem = new QTableWidgetItem(QString::number(it.value(), 'f', 4));
        rateItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_currenciesTable->setItem(row, 1, rateItem);
    }

    // Auto-size table height
    int currTableHeight = m_currenciesTable->horizontalHeader()->height() + 2;
    for (int i = 0; i < m_currenciesTable->rowCount(); ++i) {
        currTableHeight += m_currenciesTable->rowHeight(i);
    }
    m_currenciesTable->setFixedHeight(currTableHeight);

    contentLayout->addWidget(m_currenciesTable);

    // Summary
    m_summaryLabel = new QLabel(this);
    m_summaryLabel->setStyleSheet(
        "font-size: 14px; font-weight: bold; color: #2c3e50; "
        "padding: 10px; background: #f8f9fa; border-radius: 6px;"
    );
    m_summaryLabel->setAlignment(Qt::AlignCenter);
    m_summaryLabel->setText(QString("Итоговая стоимость портфеля: %1 ₽").arg(calculateTotalValue(), 0, 'f', 2));
    contentLayout->addWidget(m_summaryLabel);

    mainLayout->addWidget(m_contentWidget);
}

void SnapshotItemWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Background
    QRect cardRect = rect();
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255));
    painter.drawRoundedRect(cardRect, 10, 10);

    // Border
    painter.setPen(QPen(QColor(0, 0, 0, 15), 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(cardRect.adjusted(0, 0, -1, -1), 10, 10);
}

void SnapshotItemWidget::toggleExpanded()
{
    m_expanded = !m_expanded;
    updateExpandedState();
}

void SnapshotItemWidget::updateExpandedState()
{
    m_contentWidget->setVisible(m_expanded);
    m_expandBtn->setText(m_expanded ? "▲" : "▼");
}

void SnapshotItemWidget::onDeleteClicked()
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Удаление снимка");
    msgBox.setText(QString("Вы уверены, что хотите удалить снимок от %1?")
                   .arg(m_snapshot.date().toString("dd.MM.yyyy")));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    msgBox.setStyleSheet(
        "QMessageBox { background-color: white; }"
        "QMessageBox QLabel { color: #2c3e50; }"
        "QPushButton { "
        "   background-color: #3498db; color: white; "
        "   border: none; border-radius: 4px; "
        "   padding: 8px 16px; min-width: 80px; "
        "}"
        "QPushButton:hover { background-color: #2980b9; }"
    );

    if (msgBox.exec() == QMessageBox::Yes) {
        emit deleteRequested(m_snapshot.id());
    }
}

double SnapshotItemWidget::calculateTotalValue() const
{
    double total = 0.0;
    QList<SnapshotPosition> positions = m_snapshot.positions();
    QMap<int, double> rates = m_snapshot.currencyRates();

    for (const SnapshotPosition& pos : positions) {
        double rate = rates.value(pos.currencyId(), 1.0);
        total += pos.price() * pos.quantity() * rate;
    }

    return total;
}
