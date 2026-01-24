#ifndef CURRENCIESPAGE_H
#define CURRENCIESPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QMap>
#include "currency.h"

class CurrencyCard;

class CurrenciesPage : public QWidget
{
    Q_OBJECT

public:
    explicit CurrenciesPage(QWidget *parent = nullptr);

    QMap<int, double> getCurrencyRates() const { return m_currencyRates; }
    void setCurrencyRates(const QMap<int, double>& rates);

signals:
    void backRequested();
    void currencyRatesChanged();

public slots:
    void refreshData();

private slots:
    void onBackClicked();
    void onAddCurrencyClicked();
    void onRateChanged(int currencyId, double newRate);
    void onRenameRequested(int currencyId);
    void onDeleteRequested(int currencyId);

private:
    void setupUi();
    void loadCurrencies();

    QPushButton *m_backBtn;
    QPushButton *m_addBtn;
    QWidget *m_cardsContainer;
    QScrollArea *m_scrollArea;

    QMap<int, double> m_currencyRates;
    QList<CurrencyCard*> m_cards;
};

#endif // CURRENCIESPAGE_H
