#ifndef CREATESNAPSHOTDIALOG_H
#define CREATESNAPSHOTDIALOG_H

#include <QDialog>
#include <QDateEdit>
#include <QLineEdit>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QMap>
#include "investment/snapshot.h"

class CreateSnapshotDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateSnapshotDialog(const QMap<int, double>& currencyRates, QWidget *parent = nullptr);

private slots:
    void onCreateClicked();
    void updatePreview();

private:
    void setupUi();
    void loadCurrencies();

    QMap<int, double> m_currencyRates;

    QDateEdit *m_dateEdit;
    QLineEdit *m_descriptionEdit;
    QTableWidget *m_ratesTable;
    QLabel *m_previewLabel;
    QPushButton *m_createBtn;
};

#endif // CREATESNAPSHOTDIALOG_H
