#ifndef ADDASSETDIALOG_H
#define ADDASSETDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QPushButton>
#include "investment/portfolioasset.h"
#include "investment/assetoperation.h"
#include "investment/investmentcategory.h"
#include "investment/country.h"

class AddAssetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddAssetDialog(QWidget *parent = nullptr);

    PortfolioAsset getAsset() const;
    AssetOperation getFirstOperation() const;

private slots:
    void onAddClicked();
    void onAddCategoryClicked();
    void onAddCountryClicked();

private:
    void setupUi();
    void loadComboData();
    void reloadCategories();
    void reloadCountries();

    // Asset fields
    QLineEdit *m_nameEdit;
    QComboBox *m_categoryCombo;
    QComboBox *m_countryCombo;
    QComboBox *m_currencyCombo;

    // First purchase fields
    QDateEdit *m_dateEdit;
    QDoubleSpinBox *m_quantitySpin;
    QDoubleSpinBox *m_buyPriceSpin;
    QDoubleSpinBox *m_currentPriceSpin;
    QDoubleSpinBox *m_commissionSpin;

    QPushButton *m_addBtn;

    PortfolioAsset m_resultAsset;
    AssetOperation m_resultOperation;
};

#endif // ADDASSETDIALOG_H
