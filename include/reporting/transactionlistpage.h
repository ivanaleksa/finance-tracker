#ifndef TRANSACTIONLISTPAGE_H
#define TRANSACTIONLISTPAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QDateEdit>
#include <QComboBox>
#include <QPushButton>
#include "reporting/categorycombobox.h"

class TransactionListPage : public QWidget
{
    Q_OBJECT

public:
    explicit TransactionListPage(QWidget *parent = nullptr);

public slots:
    void refreshData();

private slots:
    void onFilterChanged();
    void onDeleteClicked();
    void onClearFiltersClicked();

private:
    void setupUi();
    void loadTransactions();

    QDateEdit *m_fromDateEdit;
    QDateEdit *m_toDateEdit;
    CategoryComboBox *m_categoryCombo;
    QComboBox *m_typeCombo;
    QPushButton *m_clearFiltersBtn;
    QTableWidget *m_table;
};

#endif // TRANSACTIONLISTPAGE_H
