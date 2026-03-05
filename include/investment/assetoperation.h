#ifndef ASSETOPERATION_H
#define ASSETOPERATION_H

#include <QString>
#include <QDate>
#include <QDateTime>
#include <QMetaType>

class AssetOperation
{
public:
    enum class Type {
        Buy,
        Sell
    };

    AssetOperation();
    AssetOperation(int id, int assetId, const QDate& date, Type type,
                   double quantity, double price, double commission = 0.0,
                   const QString& comment = QString(), const QDateTime& createdAt = QDateTime());

    int id() const { return m_id; }
    void setId(int id) { m_id = id; }

    int assetId() const { return m_assetId; }
    void setAssetId(int assetId) { m_assetId = assetId; }

    QDate date() const { return m_date; }
    void setDate(const QDate& date) { m_date = date; }

    Type type() const { return m_type; }
    void setType(Type type) { m_type = type; }

    double quantity() const { return m_quantity; }
    void setQuantity(double quantity) { m_quantity = quantity; }

    double price() const { return m_price; }
    void setPrice(double price) { m_price = price; }

    double commission() const { return m_commission; }
    void setCommission(double commission) { m_commission = commission; }

    QString comment() const { return m_comment; }
    void setComment(const QString& comment) { m_comment = comment; }

    QDateTime createdAt() const { return m_createdAt; }
    void setCreatedAt(const QDateTime& createdAt) { m_createdAt = createdAt; }

    // Calculated values
    double totalAmount() const;          // quantity * price
    double totalWithCommission() const;  // totalAmount + commission

    // Type conversion helpers
    static QString typeToString(Type type);
    static Type stringToType(const QString& str);
    static QString typeToDisplayString(Type type);

    bool isValid() const;

private:
    int m_id = -1;
    int m_assetId = -1;
    QDate m_date;
    Type m_type = Type::Buy;
    double m_quantity = 0.0;
    double m_price = 0.0;
    double m_commission = 0.0;
    QString m_comment;
    QDateTime m_createdAt;
};

Q_DECLARE_METATYPE(AssetOperation)

#endif // ASSETOPERATION_H
