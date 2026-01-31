#ifndef DEPOSITSPAGE_H
#define DEPOSITSPAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include "deposit.h"

class DepositUndoNotification;

class DepositsPage : public QWidget
{
    Q_OBJECT

public:
    explicit DepositsPage(QWidget *parent = nullptr);

public slots:
    void refreshData();

private slots:
    void onAddClicked();
    void onDeleteClicked();
    void onUndoClicked(const Deposit& deposit);

private:
    void setupUi();
    void loadDeposits();
    void clearForm();

    // Form
    QDateEdit *m_dateEdit;
    QDoubleSpinBox *m_amountSpin;
    QLineEdit *m_commentEdit;
    QPushButton *m_addBtn;

    // Table
    QTableWidget *m_table;

    // Undo notification
    DepositUndoNotification *m_undoNotification = nullptr;
};

#endif // DEPOSITSPAGE_H
