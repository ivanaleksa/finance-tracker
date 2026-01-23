#ifndef PORTFOLIOASSET_H
#define PORTFOLIOASSET_H

#include <QString>
#include <QDateTime>
#include <QMetaType>

class PortfolioAsset
{
public:
    PortfolioAsset();
    PortfolioAsset(int id, const QString& name, int categoryId, int countryId,
                   int currencyId, double currentPrice, const QDateTime& createdAt,
                   const QDateTime& updatedAt, bool isActive = true);

    int id() const { return m_id; }
    void setId(int id) { m_id = id; }

    QString name() const { return m_name; }
    void setName(const QString& name) { m_name = name; }

    int categoryId() const { return m_categoryId; }
    void setCategoryId(int categoryId) { m_categoryId = categoryId; }

    int countryId() const { return m_countryId; }
    void setCountryId(int countryId) { m_countryId = countryId; }

    int currencyId() const { return m_currencyId; }
    void setCurrencyId(int currencyId) { m_currencyId = currencyId; }

    double currentPrice() const { return m_currentPrice; }
    void setCurrentPrice(double price) { m_currentPrice = price; }

    QDateTime createdAt() const { return m_createdAt; }
    void setCreatedAt(const QDateTime& createdAt) { m_createdAt = createdAt; }

    QDateTime updatedAt() const { return m_updatedAt; }
    void setUpdatedAt(const QDateTime& updatedAt) { m_updatedAt = updatedAt; }

    bool isActive() const { return m_isActive; }
    void setIsActive(bool isActive) { m_isActive = isActive; }

    // Helper fields (filled when loading from DB with joins)
    QString categoryName() const { return m_categoryName; }
    void setCategoryName(const QString& name) { m_categoryName = name; }

    QString countryName() const { return m_countryName; }
    void setCountryName(const QString& name) { m_countryName = name; }

    QString currencyCode() const { return m_currencyCode; }
    void setCurrencyCode(const QString& code) { m_currencyCode = code; }

    double currencyRate() const { return m_currencyRate; }
    void setCurrencyRate(double rate) { m_currencyRate = rate; }

    // Computed fields (set by database when loading with aggregated operations)
    double totalQuantity() const { return m_totalQuantity; }
    void setTotalQuantity(double quantity) { m_totalQuantity = quantity; }

    double totalInvested() const { return m_totalInvested; }
    void setTotalInvested(double invested) { m_totalInvested = invested; }

    // Calculated values
    double averageBuyPrice() const;
    double currentValue() const;
    double profit() const;
    double yieldPercent() const;

    // Current value in RUB
    double currentValueInRub() const;
    double profitInRub() const;

    bool isValid() const;

private:
    int m_id = -1;
    QString m_name;
    int m_categoryId = -1;
    int m_countryId = -1;
    int m_currencyId = -1;
    double m_currentPrice = 0.0;
    QDateTime m_createdAt;
    QDateTime m_updatedAt;
    bool m_isActive = true;

    // Helper fields
    QString m_categoryName;
    QString m_countryName;
    QString m_currencyCode;
    double m_currencyRate = 1.0;

    // Computed fields
    double m_totalQuantity = 0.0;
    double m_totalInvested = 0.0;
};

Q_DECLARE_METATYPE(PortfolioAsset)

#endif // PORTFOLIOASSET_H
