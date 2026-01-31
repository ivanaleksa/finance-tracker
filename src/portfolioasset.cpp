#include "portfolioasset.h"

PortfolioAsset::PortfolioAsset()
    : m_id(-1)
    , m_categoryId(-1)
    , m_countryId(-1)
    , m_currencyId(-1)
    , m_currentPrice(0.0)
    , m_isActive(true)
    , m_currencyRate(1.0)
    , m_totalQuantity(0.0)
    , m_totalInvested(0.0)
{
}

PortfolioAsset::PortfolioAsset(int id, const QString& name, int categoryId, int countryId,
                               int currencyId, double currentPrice, const QDateTime& createdAt,
                               const QDateTime& updatedAt, bool isActive)
    : m_id(id)
    , m_name(name)
    , m_categoryId(categoryId)
    , m_countryId(countryId)
    , m_currencyId(currencyId)
    , m_currentPrice(currentPrice)
    , m_createdAt(createdAt)
    , m_updatedAt(updatedAt)
    , m_isActive(isActive)
    , m_currencyRate(1.0)
    , m_totalQuantity(0.0)
    , m_totalInvested(0.0)
{
}

double PortfolioAsset::averageBuyPrice() const
{
    if (m_totalQuantity <= 0) {
        return 0.0;
    }
    return m_totalInvested / m_totalQuantity;
}

double PortfolioAsset::currentValue() const
{
    return m_totalQuantity * m_currentPrice;
}

double PortfolioAsset::profit() const
{
    return currentValue() - m_totalInvested;
}

double PortfolioAsset::yieldPercent() const
{
    if (m_totalInvested <= 0) {
        return 0.0;
    }
    return (profit() / m_totalInvested) * 100.0;
}

double PortfolioAsset::currentValueInRub() const
{
    return currentValue() * m_currencyRate;
}

double PortfolioAsset::profitInRub() const
{
    return profit() * m_currencyRate;
}

bool PortfolioAsset::isCurrencyAsset() const
{
    return m_categoryName.toLower() == "валюта" || m_categoryName.toLower() == "валюты";
}

bool PortfolioAsset::isValid() const
{
    return !m_name.isEmpty() && m_currencyId >= 0;
}
