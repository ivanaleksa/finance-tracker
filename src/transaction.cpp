#include "transaction.h"

Transaction::Transaction()
    : m_id(-1)
    , m_date(QDate::currentDate())
    , m_type(Type::Expense)
    , m_categoryId(-1)
    , m_subcategoryId(-1)
    , m_amount(0.0)
{
}

Transaction::Transaction(int id, const QDate& date, Type type,
                         const QString& description, int categoryId, int subcategoryId, double amount)
    : m_id(id)
    , m_date(date)
    , m_type(type)
    , m_description(description)
    , m_categoryId(categoryId)
    , m_subcategoryId(subcategoryId)
    , m_amount(amount)
{
}

bool Transaction::isValid() const
{
    if (!m_date.isValid()) return false;
    if (m_amount <= 0) return false;
    if (m_type == Type::Expense && m_categoryId < 0) return false;
    return true;
}

QString Transaction::typeToString(Type type)
{
    switch (type) {
    case Type::Income: return "Доход";
    case Type::Expense: return "Расход";
    case Type::Savings: return "Сбережения";
    case Type::All: return "Все";
    }
    return QString();
}

Transaction::Type Transaction::stringToType(const QString& str)
{
    if (str == "Доход" || str == "income") return Type::Income;
    if (str == "Расход" || str == "expense") return Type::Expense;
    if (str == "Сбережения" || str == "savings") return Type::Savings;
    return Type::All;
}
