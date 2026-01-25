#include "addoperationdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>

AddOperationDialog::AddOperationDialog(int assetId, AssetOperation::Type type,
                                       double maxQuantity, QWidget *parent)
    : QDialog(parent)
    , m_assetId(assetId)
    , m_type(type)
    , m_maxQuantity(maxQuantity)
{
    setupUi();
}

void AddOperationDialog::setupUi()
{
    QString title = (m_type == AssetOperation::Type::Buy) ? "Добавить покупку" : "Добавить продажу";
    setWindowTitle(title);
    setMinimumWidth(400);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(10);

    // Date
    m_dateEdit = new QDateEdit(this);
    m_dateEdit->setCalendarPopup(true);
    m_dateEdit->setDisplayFormat("dd.MM.yyyy");
    m_dateEdit->setDate(QDate::currentDate());
    formLayout->addRow("Дата:", m_dateEdit);

    // Quantity
    m_quantitySpin = new QDoubleSpinBox(this);
    m_quantitySpin->setDecimals(6);
    m_quantitySpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_quantitySpin->setRange(0.000001, 999999999.0);
    if (m_type == AssetOperation::Type::Sell && m_maxQuantity > 0) {
        m_quantitySpin->setMaximum(m_maxQuantity);
    }
    connect(m_quantitySpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AddOperationDialog::updateTotalLabel);
    formLayout->addRow("Количество:", m_quantitySpin);

    // Price
    m_priceSpin = new QDoubleSpinBox(this);
    m_priceSpin->setDecimals(2);
    m_priceSpin->setRange(0.0, 999999999.99);
    m_priceSpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    connect(m_priceSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AddOperationDialog::updateTotalLabel);
    formLayout->addRow("Цена за единицу:", m_priceSpin);

    // Commission
    m_commissionSpin = new QDoubleSpinBox(this);
    m_commissionSpin->setDecimals(2);
    m_commissionSpin->setRange(0.0, 999999999.99);
    m_commissionSpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    connect(m_commissionSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AddOperationDialog::updateTotalLabel);
    formLayout->addRow("Комиссия:", m_commissionSpin);

    // Comment
    m_commentEdit = new QLineEdit(this);
    m_commentEdit->setPlaceholderText("Необязательно");
    formLayout->addRow("Комментарий:", m_commentEdit);

    mainLayout->addLayout(formLayout);

    // Total label
    m_totalLabel = new QLabel("Итого: 0.00", this);
    m_totalLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    mainLayout->addWidget(m_totalLabel);

    mainLayout->addSpacing(10);

    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();

    QString addBtnText = (m_type == AssetOperation::Type::Buy) ? "Купить" : "Продать";
    m_addBtn = new QPushButton(addBtnText, this);
    m_addBtn->setObjectName("primaryButton");
    m_addBtn->setCursor(Qt::PointingHandCursor);
    connect(m_addBtn, &QPushButton::clicked, this, &AddOperationDialog::onAddClicked);
    btnLayout->addWidget(m_addBtn, 0, Qt::AlignCenter);

    mainLayout->addLayout(btnLayout);

    updateTotalLabel();
}

void AddOperationDialog::updateTotalLabel()
{
    double total = m_quantitySpin->value() * m_priceSpin->value() + m_commissionSpin->value();
    m_totalLabel->setText(QString("Итого: %1").arg(total, 0, 'f', 2));
}

void AddOperationDialog::onAddClicked()
{
    double quantity = m_quantitySpin->value();
    if (quantity <= 0) {
        QMessageBox::warning(this, "Ошибка", "Укажите количество");
        return;
    }

    if (m_type == AssetOperation::Type::Sell && quantity > m_maxQuantity) {
        QMessageBox::warning(this, "Ошибка",
            QString("Нельзя продать больше, чем есть (%1)").arg(m_maxQuantity, 0, 'f', 6));
        return;
    }

    double price = m_priceSpin->value();
    if (price <= 0) {
        QMessageBox::warning(this, "Ошибка", "Укажите цену");
        return;
    }

    m_result.setAssetId(m_assetId);
    m_result.setDate(m_dateEdit->date());
    m_result.setType(m_type);
    m_result.setQuantity(quantity);
    m_result.setPrice(price);
    m_result.setCommission(m_commissionSpin->value());
    m_result.setComment(m_commentEdit->text().trimmed());

    accept();
}

AssetOperation AddOperationDialog::getOperation() const
{
    return m_result;
}
