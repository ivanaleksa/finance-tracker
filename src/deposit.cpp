#include "deposit.h"

Deposit::Deposit()
    : m_id(-1)
    , m_date()
    , m_amount(0.0)
    , m_comment()
    , m_transactionId(-1)
{
}

Deposit::Deposit(int id, const QDate& date, double amount, const QString& comment, int transactionId)
    : m_id(id)
    , m_date(date)
    , m_amount(amount)
    , m_comment(comment)
    , m_transactionId(transactionId)
{
}

bool Deposit::isValid() const
{
    return m_id >= 0 && m_date.isValid() && m_amount > 0;
}
