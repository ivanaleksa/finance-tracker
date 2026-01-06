#ifndef ADDTRANSACTIONPAGE_H
#define ADDTRANSACTIONPAGE_H

#include <QWidget>
#include <QDateEdit>
#include <QComboBox>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLabel>
#include "categorycombobox.h"
#include "transaction.h"

class UndoNotification;

class AddTransactionPage : public QWidget
{
    Q_OBJECT

public:
    explicit AddTransactionPage(QWidget *parent = nullptr);

signals:
    void transactionAdded();

private slots:
    void onTypeChanged(int index);
    void onAddClicked();
    void onAddCategoryClicked();
    void onUndoClicked(const Transaction& transaction);
    void refreshCategories();

private:
    void setupUi();
    void clearForm();
    void fillFormWithTransaction(const Transaction& transaction);

    QDateEdit *m_dateEdit;
    QComboBox *m_typeCombo;
    QLineEdit *m_descriptionEdit;
    CategoryComboBox *m_categoryCombo;
    QPushButton *m_addCategoryBtn;
    QDoubleSpinBox *m_amountSpin;
    QPushButton *m_addBtn;
    
    UndoNotification *m_undoNotification = nullptr;
};

#endif // ADDTRANSACTIONPAGE_H
