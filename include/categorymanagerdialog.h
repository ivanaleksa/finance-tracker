#ifndef CATEGORYMANAGERDIALOG_H
#define CATEGORYMANAGERDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

class CategoryManagerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CategoryManagerDialog(int parentCategoryId = -1, QWidget *parent = nullptr);

    // parentCategoryId = -1 categories
    // parentCategoryId >= 0 subcategories

signals:
    void categoriesChanged();

private slots:
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onSelectionChanged();

private:
    void setupUi();
    void loadCategories();

    QListWidget *m_listWidget;
    QPushButton *m_addBtn;
    QPushButton *m_editBtn;
    QPushButton *m_deleteBtn;
    QPushButton *m_closeBtn;

    int m_parentCategoryId;
};

#endif // CATEGORYMANAGERDIALOG_H
