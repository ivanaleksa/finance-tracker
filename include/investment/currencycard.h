#ifndef CURRENCYCARD_H
#define CURRENCYCARD_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMenu>
#include "investment/currency.h"

class CurrencyCard : public QWidget
{
    Q_OBJECT

public:
    explicit CurrencyCard(const Currency& currency, double rate, QWidget *parent = nullptr);

    int currencyId() const { return m_currency.id(); }
    double rate() const { return m_rate; }
    void setRate(double rate);

signals:
    void rateChanged(int currencyId, double newRate);
    void renameRequested(int currencyId);
    void deleteRequested(int currencyId);

protected:
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onMenuClicked();
    void onRenameClicked();
    void onDeleteClicked();
    void onRateLabelClicked();
    void onRateEditFinished();

private:
    void setupUi();

    Currency m_currency;
    double m_rate;

    QLabel *m_codeLabel;
    QLabel *m_nameLabel;
    QLabel *m_rateLabel;
    QLineEdit *m_rateEdit;
    QPushButton *m_menuBtn;
    QMenu *m_menu;
};

#endif // CURRENCYCARD_H
