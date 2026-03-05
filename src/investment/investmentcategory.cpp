#include "investment/investmentcategory.h"

InvestmentCategory::InvestmentCategory()
    : m_id(-1)
{
}

InvestmentCategory::InvestmentCategory(int id, const QString& name)
    : m_id(id)
    , m_name(name)
{
}

bool InvestmentCategory::isValid() const
{
    return m_id >= 0 && !m_name.isEmpty();
}
