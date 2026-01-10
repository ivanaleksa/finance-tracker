#ifndef CATEGORYCOMBOBOX_H
#define CATEGORYCOMBOBOX_H

#include <QComboBox>
#include <QLineEdit>
#include <QListView>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>

class CategoryComboBox : public QComboBox
{
    Q_OBJECT

public:
    explicit CategoryComboBox(QWidget *parent = nullptr);
    
    void loadCategories(bool includeAll = false);
    void loadSubcategories(int parentCategoryId);
    int currentCategoryId() const;
    void setCurrentCategoryId(int id);
    void clear();

signals:
    void categorySelected(int categoryId);

private slots:
    void onTextChanged(const QString& text);
    void onItemActivated(const QModelIndex& index);

private:
    void setupFilter();
    
    QStandardItemModel *m_sourceModel;
    QSortFilterProxyModel *m_filterModel;
    QLineEdit *m_lineEdit;
    bool m_includeAll = false;
};

#endif // CATEGORYCOMBOBOX_H
