#ifndef WITHDRAWALSPAGE_H
#define WITHDRAWALSPAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include "withdrawal.h"

class WithdrawalUndoNotification;

class WithdrawalsPage : public QWidget
{
    Q_OBJECT

public:
    explicit WithdrawalsPage(QWidget *parent = nullptr);

public slots:
    void refreshData();

private slots:
    void onAddClicked();
    void onDeleteClicked();
    void onUndoClicked(const Withdrawal& withdrawal);

private:
    void setupUi();
    void loadWithdrawals();
    void clearForm();

    // Form
    QDateEdit *m_dateEdit;
    QDoubleSpinBox *m_amountSpin;
    QLineEdit *m_commentEdit;
    QPushButton *m_addBtn;

    // Table
    QTableWidget *m_table;

    // Undo notification
    WithdrawalUndoNotification *m_undoNotification = nullptr;
};

#endif // WITHDRAWALSPAGE_H
