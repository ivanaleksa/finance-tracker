#ifndef CATEGORY_H
#define CATEGORY_H

#include <QString>
#include <QMetaType>

class Category
{
public:
    Category();
    Category(int id, const QString& name, int parentId = -1);

    int id() const { return m_id; }
    void setId(int id) { m_id = id; }

    QString name() const { return m_name; }
    void setName(const QString& name) { m_name = name; }

    int parentId() const { return m_parentId; }
    void setParentId(int parentId) { m_parentId = parentId; }

    bool isValid() const;
    bool isSubcategory() const { return m_parentId >= 0; }

private:
    int m_id = -1;
    QString m_name;
    int m_parentId = -1;
};

Q_DECLARE_METATYPE(Category)

#endif // CATEGORY_H
