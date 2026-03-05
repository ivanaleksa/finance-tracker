#include "investment/countrymanagerdialog.h"
#include "database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QInputDialog>

CountryManagerDialog::CountryManagerDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi();
    loadCountries();
}

void CountryManagerDialog::setupUi()
{
    setWindowTitle("Страны");
    setMinimumSize(400, 350);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    m_listWidget = new QListWidget(this);
    m_listWidget->setAlternatingRowColors(true);
    connect(m_listWidget, &QListWidget::itemSelectionChanged,
            this, &CountryManagerDialog::onSelectionChanged);
    connect(m_listWidget, &QListWidget::itemDoubleClicked,
            this, &CountryManagerDialog::onEditClicked);
    mainLayout->addWidget(m_listWidget);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);

    m_addBtn = new QPushButton("Добавить", this);
    m_addBtn->setCursor(Qt::PointingHandCursor);
    connect(m_addBtn, &QPushButton::clicked, this, &CountryManagerDialog::onAddClicked);
    btnLayout->addWidget(m_addBtn);

    m_editBtn = new QPushButton("Переименовать", this);
    m_editBtn->setCursor(Qt::PointingHandCursor);
    m_editBtn->setEnabled(false);
    connect(m_editBtn, &QPushButton::clicked, this, &CountryManagerDialog::onEditClicked);
    btnLayout->addWidget(m_editBtn);

    m_deleteBtn = new QPushButton("Удалить", this);
    m_deleteBtn->setCursor(Qt::PointingHandCursor);
    m_deleteBtn->setEnabled(false);
    connect(m_deleteBtn, &QPushButton::clicked, this, &CountryManagerDialog::onDeleteClicked);
    btnLayout->addWidget(m_deleteBtn);

    mainLayout->addLayout(btnLayout);

    QHBoxLayout *closeLayout = new QHBoxLayout();
    closeLayout->addStretch();
    m_closeBtn = new QPushButton("Закрыть", this);
    m_closeBtn->setCursor(Qt::PointingHandCursor);
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    closeLayout->addWidget(m_closeBtn);
    closeLayout->addStretch();

    mainLayout->addLayout(closeLayout);
}

void CountryManagerDialog::loadCountries()
{
    m_listWidget->clear();

    QList<Country> countries = Database::instance().getCountries();
    for (const Country& country : countries) {
        QListWidgetItem *item = new QListWidgetItem(country.name());
        item->setData(Qt::UserRole, country.id());
        m_listWidget->addItem(item);
    }

    onSelectionChanged();
}

void CountryManagerDialog::onSelectionChanged()
{
    bool hasSelection = !m_listWidget->selectedItems().isEmpty();
    m_editBtn->setEnabled(hasSelection);
    m_deleteBtn->setEnabled(hasSelection);
}

void CountryManagerDialog::onAddClicked()
{
    bool ok;
    QString name = QInputDialog::getText(this, "Новая страна",
                                         "Название страны:",
                                         QLineEdit::Normal, "", &ok);

    if (ok && !name.trimmed().isEmpty()) {
        Country country;
        country.setName(name.trimmed());

        if (Database::instance().addCountry(country)) {
            loadCountries();
            emit countriesChanged();
        } else {
            QMessageBox::warning(this, "Ошибка",
                                 "Не удалось добавить. Возможно, такое название уже существует.");
        }
    }
}

void CountryManagerDialog::onEditClicked()
{
    QListWidgetItem *item = m_listWidget->currentItem();
    if (!item) return;

    int countryId = item->data(Qt::UserRole).toInt();
    QString currentName = item->text();

    bool ok;
    QString newName = QInputDialog::getText(this, "Переименовать",
                                            "Новое название:",
                                            QLineEdit::Normal, currentName, &ok);

    if (ok && !newName.trimmed().isEmpty() && newName.trimmed() != currentName) {
        if (Database::instance().updateCountry(countryId, newName.trimmed())) {
            loadCountries();
            emit countriesChanged();
        } else {
            QMessageBox::warning(this, "Ошибка",
                                 "Не удалось переименовать. Возможно, такое название уже существует.");
        }
    }
}

void CountryManagerDialog::onDeleteClicked()
{
    QListWidgetItem *item = m_listWidget->currentItem();
    if (!item) return;

    int countryId = item->data(Qt::UserRole).toInt();
    QString name = item->text();

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Подтверждение удаления",
        QString("Удалить страну \"%1\"?").arg(name),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (Database::instance().deleteCountry(countryId)) {
            loadCountries();
            emit countriesChanged();
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось удалить страну.");
        }
    }
}
