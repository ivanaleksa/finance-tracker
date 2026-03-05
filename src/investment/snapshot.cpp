#include "investment/snapshot.h"

Snapshot::Snapshot()
    : m_id(-1)
{
}

Snapshot::Snapshot(int id, const QDate& date, const QString& description)
    : m_id(id)
    , m_date(date)
    , m_description(description)
{
}

double Snapshot::totalInRub() const
{
    double total = 0.0;
    for (const auto& position : m_positions) {
        total += position.sumInRub();
    }
    return total;
}

double Snapshot::totalInCurrency(int currencyId) const
{
    double rate = getCurrencyRate(currencyId);
    if (rate <= 0) {
        return totalInRub();
    }
    return totalInRub() / rate;
}

bool Snapshot::isValid() const
{
    return m_date.isValid();
}
