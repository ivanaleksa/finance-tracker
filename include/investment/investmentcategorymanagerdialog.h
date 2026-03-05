#ifndef INVESTMENTCATEGORYMANAGERDIALOG_H
#define INVESTMENTCATEGORYMANAGERDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>

class InvestmentCategoryManagerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InvestmentCategoryManagerDialog(QWidget *parent = nullptr);

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
};

#endif // INVESTMENTCATEGORYMANAGERDIALOG_H
