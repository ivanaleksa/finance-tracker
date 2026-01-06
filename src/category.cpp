#include "category.h"

Category::Category()
    : m_id(-1)
{
}

Category::Category(int id, const QString& name)
    : m_id(id)
    , m_name(name)
{
}

bool Category::isValid() const
{
    return m_id >= 0 && !m_name.isEmpty();
}
