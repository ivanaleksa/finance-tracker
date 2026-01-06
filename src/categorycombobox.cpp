#include "categorycombobox.h"
#include "database.h"
#include <QCompleter>
#include <QAbstractItemView>

CategoryComboBox::CategoryComboBox(QWidget *parent)
    : QComboBox(parent)
{
    setEditable(true);
    setInsertPolicy(QComboBox::NoInsert);

    m_sourceModel = new QStandardItemModel(this);
    m_filterModel = new QSortFilterProxyModel(this);
    m_filterModel->setSourceModel(m_sourceModel);
    m_filterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    setModel(m_sourceModel);

    m_lineEdit = lineEdit();
    m_lineEdit->setPlaceholderText("Начните вводить...");

    setupFilter();
}

void CategoryComboBox::setupFilter()
{
    QCompleter *completer = new QCompleter(m_sourceModel, this);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    setCompleter(completer);

    connect(completer, QOverload<const QModelIndex&>::of(&QCompleter::activated),
            this, &CategoryComboBox::onItemActivated);
}

void CategoryComboBox::loadCategories(bool includeAll)
{
    m_includeAll = includeAll;
    m_sourceModel->clear();

    if (includeAll) {
        QStandardItem *allItem = new QStandardItem("Все категории");
        allItem->setData(-1, Qt::UserRole);
        m_sourceModel->appendRow(allItem);
    }

    QList<Category> categories = Database::instance().getCategories();
    for (const Category& cat : categories) {
        QStandardItem *item = new QStandardItem(cat.name());
        item->setData(cat.id(), Qt::UserRole);
        m_sourceModel->appendRow(item);
    }

    if (m_sourceModel->rowCount() > 0) {
        setCurrentIndex(0);
    }
}

int CategoryComboBox::currentCategoryId() const
{
    QString text = m_lineEdit->text().trimmed();
    for (int i = 0; i < m_sourceModel->rowCount(); ++i) {
        if (m_sourceModel->item(i)->text().compare(text, Qt::CaseInsensitive) == 0) {
            return m_sourceModel->item(i)->data(Qt::UserRole).toInt();
        }
    }

    return -1;
}

void CategoryComboBox::setCurrentCategoryId(int id)
{
    for (int i = 0; i < m_sourceModel->rowCount(); ++i) {
        if (m_sourceModel->item(i)->data(Qt::UserRole).toInt() == id) {
            setCurrentIndex(i);
            m_lineEdit->setText(m_sourceModel->item(i)->text());
            return;
        }
    }
}

void CategoryComboBox::clear()
{
    m_lineEdit->clear();
    if (m_sourceModel->rowCount() > 0) {
        setCurrentIndex(0);
        m_lineEdit->setText(m_sourceModel->item(0)->text());
    }
}

void CategoryComboBox::onTextChanged(const QString& text)
{
    Q_UNUSED(text);
}

void CategoryComboBox::onItemActivated(const QModelIndex& index)
{
    int categoryId = index.data(Qt::UserRole).toInt();
    emit categorySelected(categoryId);
}
