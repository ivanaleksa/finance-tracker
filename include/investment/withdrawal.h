#ifndef WITHDRAWAL_H
#define WITHDRAWAL_H

#include <QString>
#include <QDate>
#include <QMetaType>

class Withdrawal
{
public:
    Withdrawal();
    Withdrawal(int id, const QDate& date, double amount, const QString& comment = QString(), int transactionId = -1);

    int id() const { return m_id; }
    void setId(int id) { m_id = id; }

    QDate date() const { return m_date; }
    void setDate(const QDate& date) { m_date = date; }

    double amount() const { return m_amount; }
    void setAmount(double amount) { m_amount = amount; }

    QString comment() const { return m_comment; }
    void setComment(const QString& comment) { m_comment = comment; }

    // ID of auto-created income transaction
    int transactionId() const { return m_transactionId; }
    void setTransactionId(int transactionId) { m_transactionId = transactionId; }

    bool isValid() const;

private:
    int m_id = -1;
    QDate m_date;
    double m_amount = 0.0;
    QString m_comment;
    int m_transactionId = -1;
};

Q_DECLARE_METATYPE(Withdrawal)

#endif // WITHDRAWAL_H
