#ifndef ADDOPERATIONDIALOG_H
#define ADDOPERATIONDIALOG_H

#include <QDialog>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "investment/assetoperation.h"

class AddOperationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddOperationDialog(int assetId, AssetOperation::Type type,
                                double maxQuantity = 0, QWidget *parent = nullptr);

    AssetOperation getOperation() const;

private slots:
    void onAddClicked();
    void updateTotalLabel();

private:
    void setupUi();

    int m_assetId;
    AssetOperation::Type m_type;
    double m_maxQuantity;

    QDateEdit *m_dateEdit;
    QDoubleSpinBox *m_quantitySpin;
    QDoubleSpinBox *m_priceSpin;
    QDoubleSpinBox *m_commissionSpin;
    QLineEdit *m_commentEdit;
    QLabel *m_totalLabel;
    QPushButton *m_addBtn;

    AssetOperation m_result;
};

#endif // ADDOPERATIONDIALOG_H
