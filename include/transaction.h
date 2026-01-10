#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <QString>
#include <QDate>
#include <QMetaType>

class Transaction
{
public:
    enum class Type {
        Income,
        Expense,
        Savings,
        All
    };

    Transaction();
    Transaction(int id, const QDate& date, Type type,
                const QString& description, int categoryId, int subcategoryId, double amount);

    int id() const { return m_id; }
    void setId(int id) { m_id = id; }

    QDate date() const { return m_date; }
    void setDate(const QDate& date) { m_date = date; }

    Type type() const { return m_type; }
    void setType(Type type) { m_type = type; }

    QString description() const { return m_description; }
    void setDescription(const QString& desc) { m_description = desc; }

    int categoryId() const { return m_categoryId; }
    void setCategoryId(int id) { m_categoryId = id; }

    int subcategoryId() const { return m_subcategoryId; }
    void setSubcategoryId(int id) { m_subcategoryId = id; }

    double amount() const { return m_amount; }
    void setAmount(double amount) { m_amount = amount; }

    bool isValid() const;

    static QString typeToString(Type type);
    static Type stringToType(const QString& str);

private:
    int m_id = -1;
    QDate m_date;
    Type m_type = Type::Expense;
    QString m_description;
    int m_categoryId = -1;
    int m_subcategoryId = -1;
    double m_amount = 0.0;
};

Q_DECLARE_METATYPE(Transaction)

#endif // TRANSACTION_H
