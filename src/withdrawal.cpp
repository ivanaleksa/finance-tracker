#include "withdrawal.h"

Withdrawal::Withdrawal()
    : m_id(-1)
    , m_amount(0.0)
    , m_transactionId(-1)
{
}

Withdrawal::Withdrawal(int id, const QDate& date, double amount, const QString& comment, int transactionId)
    : m_id(id)
    , m_date(date)
    , m_amount(amount)
    , m_comment(comment)
    , m_transactionId(transactionId)
{
}

bool Withdrawal::isValid() const
{
    return m_date.isValid() && m_amount > 0;
}
