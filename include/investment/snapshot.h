#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#include <QString>
#include <QDate>
#include <QList>
#include <QMap>
#include <QMetaType>
#include "investment/snapshotposition.h"

class Snapshot
{
public:
    Snapshot();
    Snapshot(int id, const QDate& date, const QString& description = QString());

    int id() const { return m_id; }
    void setId(int id) { m_id = id; }

    QDate date() const { return m_date; }
    void setDate(const QDate& date) { m_date = date; }

    QString description() const { return m_description; }
    void setDescription(const QString& description) { m_description = description; }

    // Positions
    QList<SnapshotPosition> positions() const { return m_positions; }
    void setPositions(const QList<SnapshotPosition>& positions) { m_positions = positions; }
    void addPosition(const SnapshotPosition& position) { m_positions.append(position); }

    // Currency rates stored in this snapshot (currencyId -> rate)
    QMap<int, double> currencyRates() const { return m_currencyRates; }
    void setCurrencyRates(const QMap<int, double>& rates) { m_currencyRates = rates; }
    void setCurrencyRate(int currencyId, double rate) { m_currencyRates[currencyId] = rate; }
    double getCurrencyRate(int currencyId) const { return m_currencyRates.value(currencyId, 1.0); }

    // Total sum in RUB (calculated from positions)
    double totalInRub() const;

    // Total sum in specified currency
    double totalInCurrency(int currencyId) const;

    bool isValid() const;

private:
    int m_id = -1;
    QDate m_date;
    QString m_description;
    QList<SnapshotPosition> m_positions;
    QMap<int, double> m_currencyRates;  // currencyId -> rate to RUB
};

Q_DECLARE_METATYPE(Snapshot)

#endif // SNAPSHOT_H
