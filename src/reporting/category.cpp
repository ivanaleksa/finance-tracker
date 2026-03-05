#include "reporting/category.h"

Category::Category()
    : m_id(-1)
    , m_parentId(-1)
{
}

Category::Category(int id, const QString& name, int parentId)
    : m_id(id)
    , m_name(name)
    , m_parentId(parentId)
{
}

bool Category::isValid() const
{
    return m_id >= 0 && !m_name.isEmpty();
}
