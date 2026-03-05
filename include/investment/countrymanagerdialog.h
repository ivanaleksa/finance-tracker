#ifndef COUNTRYMANAGERDIALOG_H
#define COUNTRYMANAGERDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>

class CountryManagerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CountryManagerDialog(QWidget *parent = nullptr);

signals:
    void countriesChanged();

private slots:
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onSelectionChanged();

private:
    void setupUi();
    void loadCountries();

    QListWidget *m_listWidget;
    QPushButton *m_addBtn;
    QPushButton *m_editBtn;
    QPushButton *m_deleteBtn;
    QPushButton *m_closeBtn;
};

#endif // COUNTRYMANAGERDIALOG_H
