#include "investment/currency.h"

Currency::Currency()
    : m_id(-1)
    , m_rate(1.0)
{
}

Currency::Currency(int id, const QString& code, const QString& name, double rate)
    : m_id(id)
    , m_code(code)
    , m_name(name)
    , m_rate(rate)
{
}

bool Currency::isValid() const
{
    return m_id >= 0 && !m_code.isEmpty();
}
