#ifndef ADDSNAPSHOTPAGE_H
#define ADDSNAPSHOTPAGE_H

#include <QWidget>
#include <QDateEdit>
#include <QLineEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QDoubleSpinBox>
#include "investment/snapshot.h"
#include "investment/currency.h"
#include "investment/investmentcategory.h"
#include "investment/country.h"

class SnapshotUndoNotification;

class AddSnapshotPage : public QWidget
{
    Q_OBJECT

public:
    explicit AddSnapshotPage(QWidget *parent = nullptr);

public slots:
    void refreshData();

private slots:
    void onSaveClicked();
    void onAddPositionClicked();
    void onDeletePositionClicked();
    void onAddCurrencyClicked();
    void onDeleteCurrencyClicked();
    void onCurrencyRateChanged();
    void onPositionChanged();
    void onManageCategoriesClicked();
    void onManageCountriesClicked();
    void onUndoClicked(const Snapshot& snapshot);

private:
    void setupUi();
    void loadFromPreviousSnapshot();
    void loadCurrencies();
    void loadCategories();
    void loadCountries();
    void updateTotalSum();
    void clearForm();
    Snapshot buildSnapshot();
    double getPositionSum(int row);

    // Header form
    QDateEdit *m_dateEdit;
    QLineEdit *m_descriptionEdit;

    // Currency rates table
    QTableWidget *m_currencyTable;
    QPushButton *m_addCurrencyBtn;

    // Positions table
    QTableWidget *m_positionsTable;
    QPushButton *m_addPositionBtn;
    QPushButton *m_manageCategoriesBtn;
    QPushButton *m_manageCountriesBtn;

    // Total and save
    QLabel *m_totalLabel;
    QPushButton *m_saveBtn;

    // Undo notification
    SnapshotUndoNotification *m_undoNotification = nullptr;

    // Cached data for combos
    QList<Currency> m_currencies;
    QList<InvestmentCategory> m_categories;
    QList<Country> m_countries;
};

#endif // ADDSNAPSHOTPAGE_H
