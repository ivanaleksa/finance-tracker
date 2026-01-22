#ifndef CURRENCY_H
#define CURRENCY_H

#include <QString>
#include <QMetaType>

class Currency
{
public:
    Currency();
    Currency(int id, const QString& code, const QString& name);

    int id() const { return m_id; }
    void setId(int id) { m_id = id; }

    QString code() const { return m_code; }
    void setCode(const QString& code) { m_code = code; }

    QString name() const { return m_name; }
    void setName(const QString& name) { m_name = name; }

    bool isValid() const;

private:
    int m_id = -1;
    QString m_code;  // RUB, USD, EUR
    QString m_name;  // Рубль, Доллар США
};

Q_DECLARE_METATYPE(Currency)

#endif // CURRENCY_H
