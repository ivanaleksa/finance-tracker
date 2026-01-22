#include "country.h"

Country::Country()
    : m_id(-1)
{
}

Country::Country(int id, const QString& name)
    : m_id(id)
    , m_name(name)
{
}

bool Country::isValid() const
{
    return m_id >= 0 && !m_name.isEmpty();
}
