#include "investment/snapshotposition.h"

SnapshotPosition::SnapshotPosition()
    : m_id(-1)
    , m_snapshotId(-1)
    , m_categoryId(-1)
    , m_price(0.0)
    , m_quantity(0.0)
    , m_countryId(-1)
    , m_currencyId(-1)
    , m_currencyRate(1.0)
{
}

SnapshotPosition::SnapshotPosition(int id, int snapshotId, const QString& name,
                                   int categoryId, double price, double quantity,
                                   int countryId, int currencyId)
    : m_id(id)
    , m_snapshotId(snapshotId)
    , m_name(name)
    , m_categoryId(categoryId)
    , m_price(price)
    , m_quantity(quantity)
    , m_countryId(countryId)
    , m_currencyId(currencyId)
    , m_currencyRate(1.0)
{
}

double SnapshotPosition::sumInRub() const
{
    return m_price * m_quantity * m_currencyRate;
}

bool SnapshotPosition::isValid() const
{
    return !m_name.isEmpty() && m_categoryId >= 0 && m_price >= 0 &&
           m_quantity >= 0 && m_countryId >= 0 && m_currencyId >= 0;
}
