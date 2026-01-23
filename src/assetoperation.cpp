#include "assetoperation.h"

AssetOperation::AssetOperation()
    : m_id(-1)
    , m_assetId(-1)
    , m_type(Type::Buy)
    , m_quantity(0.0)
    , m_price(0.0)
    , m_commission(0.0)
{
}

AssetOperation::AssetOperation(int id, int assetId, const QDate& date, Type type,
                               double quantity, double price, double commission,
                               const QString& comment, const QDateTime& createdAt)
    : m_id(id)
    , m_assetId(assetId)
    , m_date(date)
    , m_type(type)
    , m_quantity(quantity)
    , m_price(price)
    , m_commission(commission)
    , m_comment(comment)
    , m_createdAt(createdAt)
{
}

double AssetOperation::totalAmount() const
{
    return m_quantity * m_price;
}

double AssetOperation::totalWithCommission() const
{
    return totalAmount() + m_commission;
}

QString AssetOperation::typeToString(Type type)
{
    switch (type) {
    case Type::Buy:
        return "buy";
    case Type::Sell:
        return "sell";
    }
    return "buy";
}

AssetOperation::Type AssetOperation::stringToType(const QString& str)
{
    if (str.toLower() == "sell") {
        return Type::Sell;
    }
    return Type::Buy;
}

QString AssetOperation::typeToDisplayString(Type type)
{
    switch (type) {
    case Type::Buy:
        return "Покупка";
    case Type::Sell:
        return "Продажа";
    }
    return "Покупка";
}

bool AssetOperation::isValid() const
{
    return m_assetId >= 0 && m_date.isValid() && m_quantity > 0 && m_price >= 0;
}
