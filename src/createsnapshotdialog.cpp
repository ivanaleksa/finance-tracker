#include "createsnapshotdialog.h"
#include "database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>

CreateSnapshotDialog::CreateSnapshotDialog(const QMap<int, double>& currencyRates, QWidget *parent)
    : QDialog(parent)
    , m_currencyRates(currencyRates)
{
    setupUi();
    loadCurrencies();
    updatePreview();
}

void CreateSnapshotDialog::setupUi()
{
    setWindowTitle("Создать снимок портфеля");
    setMinimumWidth(500);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // Date and description
    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(10);

    m_dateEdit = new QDateEdit(this);
    m_dateEdit->setCalendarPopup(true);
    m_dateEdit->setDisplayFormat("dd.MM.yyyy");
    m_dateEdit->setDate(QDate::currentDate());
    formLayout->addRow("Дата снимка:", m_dateEdit);

    m_descriptionEdit = new QLineEdit(this);
    m_descriptionEdit->setPlaceholderText("Описание (необязательно)");
    formLayout->addRow("Описание:", m_descriptionEdit);

    mainLayout->addLayout(formLayout);

    // Currency rates table
    QGroupBox *ratesGroup = new QGroupBox("Курсы валют", this);
    QVBoxLayout *ratesLayout = new QVBoxLayout(ratesGroup);

    m_ratesTable = new QTableWidget(this);
    m_ratesTable->setColumnCount(3);
    m_ratesTable->setHorizontalHeaderLabels({"Валюта", "Название", "Курс к RUB"});
    m_ratesTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_ratesTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_ratesTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_ratesTable->verticalHeader()->setVisible(false);
    m_ratesTable->setSelectionMode(QAbstractItemView::NoSelection);
    m_ratesTable->setAlternatingRowColors(true);
    m_ratesTable->setMaximumHeight(150);

    ratesLayout->addWidget(m_ratesTable);
    mainLayout->addWidget(ratesGroup);

    // Preview
    m_previewLabel = new QLabel(this);
    m_previewLabel->setStyleSheet("font-weight: bold; font-size: 14px; padding: 10px; background: #f0f0f0; border-radius: 4px;");
    m_previewLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_previewLabel);

    mainLayout->addSpacing(10);

    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();

    m_createBtn = new QPushButton("Создать снимок", this);
    m_createBtn->setObjectName("primaryButton");
    m_createBtn->setCursor(Qt::PointingHandCursor);
    connect(m_createBtn, &QPushButton::clicked, this, &CreateSnapshotDialog::onCreateClicked);
    btnLayout->addWidget(m_createBtn, 0, Qt::AlignCenter);

    mainLayout->addLayout(btnLayout);
}

void CreateSnapshotDialog::loadCurrencies()
{
    QList<Currency> currencies = Database::instance().getCurrencies();
    m_ratesTable->setRowCount(currencies.size());

    for (int i = 0; i < currencies.size(); ++i) {
        const Currency& curr = currencies[i];

        QTableWidgetItem *codeItem = new QTableWidgetItem(curr.code());
        codeItem->setData(Qt::UserRole, curr.id());
        codeItem->setFlags(codeItem->flags() & ~Qt::ItemIsEditable);
        m_ratesTable->setItem(i, 0, codeItem);

        QTableWidgetItem *nameItem = new QTableWidgetItem(curr.name());
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        m_ratesTable->setItem(i, 1, nameItem);

        double rate = m_currencyRates.value(
            curr.id(),
            curr.code() == "RUB" ? 1.0 : 100.0
            );

        QTableWidgetItem *rateItem =
            new QTableWidgetItem(QString::number(rate, 'f', 4));

        rateItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        rateItem->setFlags(rateItem->flags() & ~Qt::ItemIsEditable);

        m_ratesTable->setItem(i, 2, rateItem);
    }
}

void CreateSnapshotDialog::updatePreview()
{
    // Calculate total from active assets using currency rates
    QList<PortfolioAsset> assets = Database::instance().getActivePortfolioAssets();
    double totalRub = 0.0;

    for (const PortfolioAsset& asset : assets) {
        if (asset.totalQuantity() <= 0) continue;

        double rate = m_currencyRates.value(asset.currencyId(), 1.0);
        double valueInRub = asset.currentValue() * rate;
        totalRub += valueInRub;
    }

    m_previewLabel->setText(QString("Итоговая стоимость: %1 ₽").arg(totalRub, 0, 'f', 2));
}

void CreateSnapshotDialog::onCreateClicked()
{
    Snapshot snapshot;
    snapshot.setDate(m_dateEdit->date());
    snapshot.setDescription(m_descriptionEdit->text().trimmed());

    // Use currency rates passed to dialog
    if (Database::instance().createSnapshotFromPortfolio(snapshot, m_currencyRates)) {
        QMessageBox::information(this, "Успех", "Снимок портфеля успешно создан");
        accept();
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось создать снимок");
    }
}
