#include "database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

Database::Database()
{
}

Database::~Database()
{
    close();
}

Database& Database::instance()
{
    static Database instance;
    return instance;
}

bool Database::initialize()
{
    // the file will store in data dir
    QString dataPath = QDir::currentPath() + "/data";
    QDir dir;
    if (!dir.exists(dataPath)) {
        if (!dir.mkpath(dataPath)) {
            qWarning() << "Не удалось создать папку data";
            return false;
        }
    }
    
    QString dbPath = dataPath + "/finance.db";
    
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);
    
    if (!m_db.open()) {
        qWarning() << "Ошибка открытия БД:" << m_db.lastError().text();
        return false;
    }
    
    if (!createTables()) {
        return false;
    }
    
    // default categories
    insertDefaultCategories();
    
    return true;
}

void Database::close()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool Database::createTables()
{
    QSqlQuery query;

    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS categories (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            parent_id INTEGER DEFAULT -1,
            UNIQUE(name, parent_id),
            FOREIGN KEY (parent_id) REFERENCES categories(id) ON DELETE CASCADE
        )
    )")) {
        qWarning() << "Ошибка создания таблицы categories:" << query.lastError().text();
        return false;
    }

    query.exec("PRAGMA table_info(categories)");
    bool hasParentId = false;
    while (query.next()) {
        if (query.value(1).toString() == "parent_id") {
            hasParentId = true;
            break;
        }
    }
    if (!hasParentId) {
        query.exec("ALTER TABLE categories ADD COLUMN parent_id INTEGER DEFAULT -1");
    }

    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS transactions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            date TEXT NOT NULL,
            type TEXT NOT NULL,
            description TEXT,
            category_id INTEGER,
            subcategory_id INTEGER DEFAULT -1,
            amount REAL NOT NULL,
            FOREIGN KEY (category_id) REFERENCES categories(id),
            FOREIGN KEY (subcategory_id) REFERENCES categories(id)
        )
    )")) {
        qWarning() << "Ошибка создания таблицы transactions:" << query.lastError().text();
        return false;
    }

    query.exec("PRAGMA table_info(transactions)");
    bool hasSubcategoryId = false;
    while (query.next()) {
        if (query.value(1).toString() == "subcategory_id") {
            hasSubcategoryId = true;
            break;
        }
    }
    if (!hasSubcategoryId) {
        query.exec("ALTER TABLE transactions ADD COLUMN subcategory_id INTEGER DEFAULT -1");
    }

    query.exec("CREATE INDEX IF NOT EXISTS idx_transactions_date ON transactions(date)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_transactions_type ON transactions(type)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_transactions_category ON transactions(category_id)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_categories_parent ON categories(parent_id)");

    return true;
}
void Database::insertDefaultCategories()
{
    QSqlQuery query;
    query.exec("SELECT COUNT(*) FROM categories");
    if (query.next() && query.value(0).toInt() > 0) {
        return;
    }
    
    QStringList defaultCategories = {
        "Продукты",
        "Транспорт",
        "Жильё",
        "Коммунальные услуги",
        "Связь и интернет",
        "Развлечения",
        "Одежда",
        "Здоровье",
        "Образование",
        "Рестораны и кафе",
        "Подарки",
        "Путешествия",
        "Спорт",
        "Красота",
        "Домашние животные",
        "Бытовая техника",
        "Подписки",
        "Другое"
    };
    
    for (const QString& name : defaultCategories) {
        query.prepare("INSERT INTO categories (name) VALUES (:name)");
        query.bindValue(":name", name);
        query.exec();
    }
}

QMap<QString, double> Database::getSubcategoryTotalsByMonth(int year, int month, int categoryId)
{
    QMap<QString, double> result;

    QDate firstDay(year, month, 1);
    QDate lastDay(year, month, firstDay.daysInMonth());

    QSqlQuery query;
    query.prepare(R"(
        SELECT c.name, COALESCE(SUM(t.amount), 0)
        FROM transactions t
        LEFT JOIN categories c ON t.subcategory_id = c.id
        WHERE t.date >= :fromDate AND t.date <= :toDate
          AND t.type = 'expense'
          AND t.category_id = :categoryId
          AND t.subcategory_id >= 0
        GROUP BY c.name
    )");
    query.bindValue(":fromDate", firstDay.toString(Qt::ISODate));
    query.bindValue(":toDate", lastDay.toString(Qt::ISODate));
    query.bindValue(":categoryId", categoryId);

    if (query.exec()) {
        while (query.next()) {
            QString name = query.value(0).toString();
            if (name.isEmpty()) name = "Без подкатегории";
            result[name] = query.value(1).toDouble();
        }
    }

    query.prepare(R"(
        SELECT COALESCE(SUM(t.amount), 0)
        FROM transactions t
        WHERE t.date >= :fromDate AND t.date <= :toDate
          AND t.type = 'expense'
          AND t.category_id = :categoryId
          AND (t.subcategory_id < 0 OR t.subcategory_id IS NULL)
    )");
    query.bindValue(":fromDate", firstDay.toString(Qt::ISODate));
    query.bindValue(":toDate", lastDay.toString(Qt::ISODate));
    query.bindValue(":categoryId", categoryId);

    if (query.exec() && query.next()) {
        double amount = query.value(0).toDouble();
        if (amount > 0) {
            result["Без подкатегории"] = amount;
        }
    }

    return result;
}

bool Database::addTransaction(Transaction& transaction)
{
    QSqlQuery query;
    query.prepare(R"(
        INSERT INTO transactions (date, type, description, category_id, subcategory_id, amount)
        VALUES (:date, :type, :description, :category_id, :subcategory_id, :amount)
    )");

    query.bindValue(":date", transaction.date().toString(Qt::ISODate));
    query.bindValue(":type", transaction.type() == Transaction::Type::Income ? "income" :
                                 (transaction.type() == Transaction::Type::Savings ? "savings" : "expense"));
    query.bindValue(":description", transaction.description());
    query.bindValue(":category_id", transaction.categoryId() >= 0 ? transaction.categoryId() : QVariant());
    query.bindValue(":subcategory_id", transaction.subcategoryId() >= 0 ? transaction.subcategoryId() : QVariant());
    query.bindValue(":amount", transaction.amount());

    if (!query.exec()) {
        qWarning() << "Ошибка добавления транзакции:" << query.lastError().text();
        return false;
    }

    transaction.setId(query.lastInsertId().toInt());
    emit transactionAdded(transaction);
    emit dataChanged();
    return true;
}

bool Database::updateTransaction(const Transaction& transaction)
{
    QSqlQuery query;
    query.prepare(R"(
        UPDATE transactions 
        SET date = :date, type = :type, description = :description, 
            category_id = :category_id, amount = :amount
        WHERE id = :id
    )");
    
    query.bindValue(":id", transaction.id());
    query.bindValue(":date", transaction.date().toString(Qt::ISODate));
    query.bindValue(":type", transaction.type() == Transaction::Type::Income ? "income" : "expense");
    query.bindValue(":description", transaction.description());
    query.bindValue(":category_id", transaction.categoryId() >= 0 ? transaction.categoryId() : QVariant());
    query.bindValue(":amount", transaction.amount());
    
    if (!query.exec()) {
        qWarning() << "Ошибка обновления транзакции:" << query.lastError().text();
        return false;
    }
    
    emit transactionUpdated(transaction);
    emit dataChanged();
    return true;
}

bool Database::deleteTransaction(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM transactions WHERE id = :id");
    query.bindValue(":id", id);
    
    if (!query.exec()) {
        qWarning() << "Ошибка удаления транзакции:" << query.lastError().text();
        return false;
    }
    
    emit transactionDeleted(id);
    emit dataChanged();
    return true;
}

Transaction Database::getTransaction(int id)
{
    QSqlQuery query;
    query.prepare("SELECT id, date, type, description, category_id, subcategory_id, amount FROM transactions WHERE id = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        return Transaction(
            query.value(0).toInt(),
            QDate::fromString(query.value(1).toString(), Qt::ISODate),
            Transaction::stringToType(query.value(2).toString()),
            query.value(3).toString(),
            query.value(4).isNull() ? -1 : query.value(4).toInt(),
            query.value(5).isNull() ? -1 : query.value(5).toInt(),
            query.value(6).toDouble()
            );
    }

    return Transaction();
}

QList<Transaction> Database::getTransactions(const QDate& fromDate, const QDate& toDate,
                                             int categoryId, Transaction::Type type)
{
    QList<Transaction> result;

    QString sql = "SELECT id, date, type, description, category_id, subcategory_id, amount FROM transactions WHERE 1=1";

    if (fromDate.isValid()) {
        sql += " AND date >= :fromDate";
    }
    if (toDate.isValid()) {
        sql += " AND date <= :toDate";
    }
    if (categoryId >= 0) {
        sql += " AND category_id = :categoryId";
    }
    if (type != Transaction::Type::All) {
        sql += " AND type = :type";
    }

    sql += " ORDER BY date DESC, id DESC";

    QSqlQuery query;
    query.prepare(sql);

    if (fromDate.isValid()) {
        query.bindValue(":fromDate", fromDate.toString(Qt::ISODate));
    }
    if (toDate.isValid()) {
        query.bindValue(":toDate", toDate.toString(Qt::ISODate));
    }
    if (categoryId >= 0) {
        query.bindValue(":categoryId", categoryId);
    }
    if (type != Transaction::Type::All) {
        QString typeStr;
        switch (type) {
        case Transaction::Type::Income: typeStr = "income"; break;
        case Transaction::Type::Savings: typeStr = "savings"; break;
        default: typeStr = "expense"; break;
        }
        query.bindValue(":type", typeStr);
    }

    if (query.exec()) {
        while (query.next()) {
            result.append(Transaction(
                query.value(0).toInt(),
                QDate::fromString(query.value(1).toString(), Qt::ISODate),
                Transaction::stringToType(query.value(2).toString()),
                query.value(3).toString(),
                query.value(4).isNull() ? -1 : query.value(4).toInt(),
                query.value(5).isNull() ? -1 : query.value(5).toInt(),
                query.value(6).toDouble()
                ));
        }
    }

    return result;
}

QList<Transaction> Database::getTransactionsByMonth(int year, int month, Transaction::Type type)
{
    QDate firstDay(year, month, 1);
    QDate lastDay(year, month, firstDay.daysInMonth());
    return getTransactions(firstDay, lastDay, -1, type);
}

QList<Transaction> Database::getTransactionsByYear(int year, Transaction::Type type)
{
    QDate firstDay(year, 1, 1);
    QDate lastDay(year, 12, 31);
    return getTransactions(firstDay, lastDay, -1, type);
}

bool Database::addCategory(Category& category)
{
    QSqlQuery query;
    query.prepare("INSERT INTO categories (name, parent_id) VALUES (:name, :parent_id)");
    query.bindValue(":name", category.name());
    query.bindValue(":parent_id", category.parentId());

    if (!query.exec()) {
        qWarning() << "Ошибка добавления категории:" << query.lastError().text();
        return false;
    }

    category.setId(query.lastInsertId().toInt());
    emit categoryAdded(category);
    return true;
}
bool Database::deleteCategory(int id)
{
    QSqlQuery query;

    query.prepare("DELETE FROM categories WHERE parent_id = :id");
    query.bindValue(":id", id);
    query.exec();

    query.prepare("DELETE FROM categories WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "Ошибка удаления категории:" << query.lastError().text();
        return false;
    }

    emit categoryDeleted(id);
    emit dataChanged();
    return true;
}

QList<Category> Database::getCategories()
{
    QList<Category> result;

    QSqlQuery query("SELECT id, name, parent_id FROM categories WHERE parent_id = -1 ORDER BY name");
    while (query.next()) {
        result.append(Category(
            query.value(0).toInt(),
            query.value(1).toString(),
            query.value(2).toInt()
            ));
    }

    return result;
}

Category Database::getCategory(int id)
{
    QSqlQuery query;
    query.prepare("SELECT id, name, parent_id FROM categories WHERE id = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        return Category(
            query.value(0).toInt(),
            query.value(1).toString(),
            query.value(2).toInt()
            );
    }

    return Category();
}

QList<Category> Database::getSubcategories(int parentId)
{
    QList<Category> result;

    QSqlQuery query;
    query.prepare("SELECT id, name, parent_id FROM categories WHERE parent_id = :parentId ORDER BY name");
    query.bindValue(":parentId", parentId);

    if (query.exec()) {
        while (query.next()) {
            result.append(Category(
                query.value(0).toInt(),
                query.value(1).toString(),
                query.value(2).toInt()
                ));
        }
    }

    return result;
}

bool Database::updateCategoryName(int id, const QString& newName)
{
    QSqlQuery query;
    query.prepare("UPDATE categories SET name = :name WHERE id = :id");
    query.bindValue(":name", newName);
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "Ошибка обновления категории:" << query.lastError().text();
        return false;
    }

    emit dataChanged();
    return true;
}


double Database::getTotalByMonth(int year, int month, Transaction::Type type)
{
    QDate firstDay(year, month, 1);
    QDate lastDay(year, month, firstDay.daysInMonth());
    
    QSqlQuery query;
    query.prepare(R"(
        SELECT COALESCE(SUM(amount), 0) FROM transactions 
        WHERE date >= :fromDate AND date <= :toDate AND type = :type
    )");
    query.bindValue(":fromDate", firstDay.toString(Qt::ISODate));
    query.bindValue(":toDate", lastDay.toString(Qt::ISODate));
    query.bindValue(":type", type == Transaction::Type::Income ? "income" : "expense");
    
    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }
    
    return 0.0;
}

double Database::getTotalByYear(int year, Transaction::Type type)
{
    QSqlQuery query;
    query.prepare(R"(
        SELECT COALESCE(SUM(amount), 0) FROM transactions 
        WHERE strftime('%Y', date) = :year AND type = :type
    )");
    query.bindValue(":year", QString::number(year));
    query.bindValue(":type", type == Transaction::Type::Income ? "income" : "expense");
    
    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }
    
    return 0.0;
}

QMap<QString, double> Database::getCategoryTotalsByMonth(int year, int month, Transaction::Type type)
{
    QMap<QString, double> result;
    
    QDate firstDay(year, month, 1);
    QDate lastDay(year, month, firstDay.daysInMonth());
    
    QSqlQuery query;
    query.prepare(R"(
        SELECT c.name, COALESCE(SUM(t.amount), 0) 
        FROM transactions t
        LEFT JOIN categories c ON t.category_id = c.id
        WHERE t.date >= :fromDate AND t.date <= :toDate AND t.type = :type
        GROUP BY c.name
    )");
    query.bindValue(":fromDate", firstDay.toString(Qt::ISODate));
    query.bindValue(":toDate", lastDay.toString(Qt::ISODate));
    query.bindValue(":type", type == Transaction::Type::Income ? "income" : "expense");
    
    if (query.exec()) {
        while (query.next()) {
            QString name = query.value(0).toString();
            if (name.isEmpty()) name = "Без категории";
            result[name] = query.value(1).toDouble();
        }
    }
    
    return result;
}

QMap<int, double> Database::getMonthlyTotals(int year, Transaction::Type type)
{
    QMap<int, double> result;

    for (int i = 1; i <= 12; ++i) {
        result[i] = 0.0;
    }
    
    QSqlQuery query;
    query.prepare(R"(
        SELECT CAST(strftime('%m', date) AS INTEGER) as month, COALESCE(SUM(amount), 0) 
        FROM transactions 
        WHERE strftime('%Y', date) = :year AND type = :type
        GROUP BY month
    )");
    query.bindValue(":year", QString::number(year));
    query.bindValue(":type", type == Transaction::Type::Income ? "income" : "expense");
    
    if (query.exec()) {
        while (query.next()) {
            result[query.value(0).toInt()] = query.value(1).toDouble();
        }
    }
    
    return result;
}

double Database::getBalanceUpToMonth(int year, int month)
{
    QDate endDate(year, month, 1);
    endDate = endDate.addDays(-1);

    if (!endDate.isValid() || endDate < QDate(2000, 1, 1)) {
        return 0.0;
    }

    QSqlQuery query;
    query.prepare(R"(
        SELECT
            COALESCE(SUM(CASE WHEN type = 'income' THEN amount ELSE 0 END), 0) -
            COALESCE(SUM(CASE WHEN type = 'expense' THEN amount ELSE 0 END), 0) -
            COALESCE(SUM(CASE WHEN type = 'savings' THEN amount ELSE 0 END), 0)
        FROM transactions
        WHERE date <= :endDate
    )");
    query.bindValue(":endDate", endDate.toString(Qt::ISODate));

    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }

    return 0.0;
}

double Database::getSavingsByMonth(int year, int month)
{
    QDate firstDay(year, month, 1);
    QDate lastDay(year, month, firstDay.daysInMonth());

    QSqlQuery query;
    query.prepare(R"(
        SELECT COALESCE(SUM(amount), 0) FROM transactions
        WHERE date >= :fromDate AND date <= :toDate AND type = 'savings'
    )");
    query.bindValue(":fromDate", firstDay.toString(Qt::ISODate));
    query.bindValue(":toDate", lastDay.toString(Qt::ISODate));

    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }

    return 0.0;
}

QMap<int, double> Database::getMonthlySavings(int year)
{
    QMap<int, double> result;

    for (int i = 1; i <= 12; ++i) {
        result[i] = 0.0;
    }

    QSqlQuery query;
    query.prepare(R"(
        SELECT CAST(strftime('%m', date) AS INTEGER) as month, COALESCE(SUM(amount), 0)
        FROM transactions
        WHERE strftime('%Y', date) = :year AND type = 'savings'
        GROUP BY month
    )");
    query.bindValue(":year", QString::number(year));

    if (query.exec()) {
        while (query.next()) {
            result[query.value(0).toInt()] = query.value(1).toDouble();
        }
    }

    return result;
}
