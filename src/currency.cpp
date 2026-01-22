#include "currency.h"

Currency::Currency()
    : m_id(-1)
{
}

Currency::Currency(int id, const QString& code, const QString& name)
    : m_id(id)
    , m_code(code)
    , m_name(name)
{
}

bool Currency::isValid() const
{
    return m_id >= 0 && !m_code.isEmpty();
}
