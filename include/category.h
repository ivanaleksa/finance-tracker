#ifndef CATEGORY_H
#define CATEGORY_H

#include <QString>
#include <QMetaType>

class Category
{
public:
    Category();
    Category(int id, const QString& name);
    
    int id() const { return m_id; }
    void setId(int id) { m_id = id; }
    
    QString name() const { return m_name; }
    void setName(const QString& name) { m_name = name; }
    
    bool isValid() const;

private:
    int m_id = -1;
    QString m_name;
};

Q_DECLARE_METATYPE(Category)

#endif // CATEGORY_H
