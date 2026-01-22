#ifndef SNAPSHOTPOSITION_H
#define SNAPSHOTPOSITION_H

#include <QString>
#include <QMetaType>

class SnapshotPosition
{
public:
    SnapshotPosition();
    SnapshotPosition(int id, int snapshotId, const QString& name,
                     int categoryId, double price, double quantity,
                     int countryId, int currencyId);

    int id() const { return m_id; }
    void setId(int id) { m_id = id; }

    int snapshotId() const { return m_snapshotId; }
    void setSnapshotId(int snapshotId) { m_snapshotId = snapshotId; }

    QString name() const { return m_name; }
    void setName(const QString& name) { m_name = name; }

    int categoryId() const { return m_categoryId; }
    void setCategoryId(int categoryId) { m_categoryId = categoryId; }

    double price() const { return m_price; }
    void setPrice(double price) { m_price = price; }

    double quantity() const { return m_quantity; }
    void setQuantity(double quantity) { m_quantity = quantity; }

    int countryId() const { return m_countryId; }
    void setCountryId(int countryId) { m_countryId = countryId; }

    int currencyId() const { return m_currencyId; }
    void setCurrencyId(int currencyId) { m_currencyId = currencyId; }

    // Helper fields (not stored in DB, filled when loading)
    QString categoryName() const { return m_categoryName; }
    void setCategoryName(const QString& name) { m_categoryName = name; }

    QString countryName() const { return m_countryName; }
    void setCountryName(const QString& name) { m_countryName = name; }

    QString currencyCode() const { return m_currencyCode; }
    void setCurrencyCode(const QString& code) { m_currencyCode = code; }

    double currencyRate() const { return m_currencyRate; }
    void setCurrencyRate(double rate) { m_currencyRate = rate; }

    // Calculated value in RUB: price * quantity * currencyRate
    double sumInRub() const;

    bool isValid() const;

private:
    int m_id = -1;
    int m_snapshotId = -1;
    QString m_name;
    int m_categoryId = -1;
    double m_price = 0.0;
    double m_quantity = 0.0;
    int m_countryId = -1;
    int m_currencyId = -1;

    // Helper fields
    QString m_categoryName;
    QString m_countryName;
    QString m_currencyCode;
    double m_currencyRate = 1.0;
};

Q_DECLARE_METATYPE(SnapshotPosition)

#endif // SNAPSHOTPOSITION_H
