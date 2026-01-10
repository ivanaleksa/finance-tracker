#include "categorymanagerdialog.h"
#include "database.h"
#include <QHBoxLayout>
#include <QMessageBox>
#include <QInputDialog>

CategoryManagerDialog::CategoryManagerDialog(int parentCategoryId, QWidget *parent)
    : QDialog(parent)
    , m_parentCategoryId(parentCategoryId)
{
    setupUi();
    loadCategories();
}

void CategoryManagerDialog::setupUi()
{
    QString title = (m_parentCategoryId < 0) ? "Управление категориями" : "Управление подкатегориями";
    setWindowTitle(title);
    setMinimumSize(400, 350);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // category list
    m_listWidget = new QListWidget(this);
    m_listWidget->setAlternatingRowColors(true);
    connect(m_listWidget, &QListWidget::itemSelectionChanged,
            this, &CategoryManagerDialog::onSelectionChanged);
    connect(m_listWidget, &QListWidget::itemDoubleClicked,
            this, &CategoryManagerDialog::onEditClicked);
    mainLayout->addWidget(m_listWidget);

    // handling buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);

    m_addBtn = new QPushButton("Добавить", this);
    m_addBtn->setCursor(Qt::PointingHandCursor);
    connect(m_addBtn, &QPushButton::clicked, this, &CategoryManagerDialog::onAddClicked);
    btnLayout->addWidget(m_addBtn);

    m_editBtn = new QPushButton("Переименовать", this);
    m_editBtn->setCursor(Qt::PointingHandCursor);
    m_editBtn->setEnabled(false);
    connect(m_editBtn, &QPushButton::clicked, this, &CategoryManagerDialog::onEditClicked);
    btnLayout->addWidget(m_editBtn);

    m_deleteBtn = new QPushButton("Удалить", this);
    m_deleteBtn->setCursor(Qt::PointingHandCursor);
    m_deleteBtn->setEnabled(false);
    connect(m_deleteBtn, &QPushButton::clicked, this, &CategoryManagerDialog::onDeleteClicked);
    btnLayout->addWidget(m_deleteBtn);

    mainLayout->addLayout(btnLayout);

    // close button
    QHBoxLayout *closeLayout = new QHBoxLayout();
    closeLayout->addStretch();
    m_closeBtn = new QPushButton("Закрыть", this);
    m_closeBtn->setCursor(Qt::PointingHandCursor);
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    closeLayout->addWidget(m_closeBtn);
    closeLayout->addStretch();

    mainLayout->addLayout(closeLayout);
}

void CategoryManagerDialog::loadCategories()
{
    m_listWidget->clear();

    if (m_parentCategoryId < 0) {
        // load categiries
        QList<Category> categories = Database::instance().getCategories();
        for (const Category& cat : categories) {
            QListWidgetItem *item = new QListWidgetItem(cat.name());
            item->setData(Qt::UserRole, cat.id());
            m_listWidget->addItem(item);
        }
    } else {
        // load subcategories
        QList<Category> subcategories = Database::instance().getSubcategories(m_parentCategoryId);
        for (const Category& cat : subcategories) {
            QListWidgetItem *item = new QListWidgetItem(cat.name());
            item->setData(Qt::UserRole, cat.id());
            m_listWidget->addItem(item);
        }
    }

    onSelectionChanged();
}

void CategoryManagerDialog::onSelectionChanged()
{
    bool hasSelection = !m_listWidget->selectedItems().isEmpty();
    m_editBtn->setEnabled(hasSelection);
    m_deleteBtn->setEnabled(hasSelection);
}

void CategoryManagerDialog::onAddClicked()
{
    QString title = (m_parentCategoryId < 0) ? "Новая категория" : "Новая подкатегория";
    QString label = (m_parentCategoryId < 0) ? "Название категории:" : "Название подкатегории:";

    bool ok;
    QString name = QInputDialog::getText(this, title, label,
                                         QLineEdit::Normal, "", &ok);

    if (ok && !name.trimmed().isEmpty()) {
        Category category;
        category.setName(name.trimmed());
        category.setParentId(m_parentCategoryId);

        if (Database::instance().addCategory(category)) {
            loadCategories();
            emit categoriesChanged();
        } else {
            QMessageBox::warning(this, "Ошибка",
                                 "Не удалось добавить. Возможно, такое название уже существует.");
        }
    }
}

void CategoryManagerDialog::onEditClicked()
{
    QListWidgetItem *item = m_listWidget->currentItem();
    if (!item) return;

    int categoryId = item->data(Qt::UserRole).toInt();
    QString currentName = item->text();

    bool ok;
    QString newName = QInputDialog::getText(this, "Переименовать",
                                            "Новое название:",
                                            QLineEdit::Normal, currentName, &ok);

    if (ok && !newName.trimmed().isEmpty() && newName.trimmed() != currentName) {
        if (Database::instance().updateCategoryName(categoryId, newName.trimmed())) {
            loadCategories();
            emit categoriesChanged();
        } else {
            QMessageBox::warning(this, "Ошибка",
                                 "Не удалось переименовать. Возможно, такое название уже существует.");
        }
    }
}

void CategoryManagerDialog::onDeleteClicked()
{
    QListWidgetItem *item = m_listWidget->currentItem();
    if (!item) return;

    int categoryId = item->data(Qt::UserRole).toInt();
    QString name = item->text();

    QString message = (m_parentCategoryId < 0)
                          ? QString("Удалить категорию \"%1\"?\n\nВсе подкатегории также будут удалены.\nТранзакции с этой категорией потеряют привязку.").arg(name)
                          : QString("Удалить подкатегорию \"%1\"?\n\nТранзакции с этой подкатегорией потеряют привязку.").arg(name);

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Подтверждение удаления", message,
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (Database::instance().deleteCategory(categoryId)) {
            loadCategories();
            emit categoriesChanged();
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось удалить категорию.");
        }
    }
}
