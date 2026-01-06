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
    
    // category table
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS categories (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE
        )
    )")) {
        qWarning() << "Ошибка создания таблицы categories:" << query.lastError().text();
        return false;
    }
    
    // transaction table
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS transactions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            date TEXT NOT NULL,
            type TEXT NOT NULL,
            description TEXT,
            category_id INTEGER,
            amount REAL NOT NULL,
            FOREIGN KEY (category_id) REFERENCES categories(id)
        )
    )")) {
        qWarning() << "Ошибка создания таблицы transactions:" << query.lastError().text();
        return false;
    }
    
    query.exec("CREATE INDEX IF NOT EXISTS idx_transactions_date ON transactions(date)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_transactions_type ON transactions(type)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_transactions_category ON transactions(category_id)");
    
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

bool Database::addTransaction(Transaction& transaction)
{
    QSqlQuery query;
    query.prepare(R"(
        INSERT INTO transactions (date, type, description, category_id, amount)
        VALUES (:date, :type, :description, :category_id, :amount)
    )");
    
    query.bindValue(":date", transaction.date().toString(Qt::ISODate));
    query.bindValue(":type", transaction.type() == Transaction::Type::Income ? "income" : "expense");
    query.bindValue(":description", transaction.description());
    query.bindValue(":category_id", transaction.categoryId() >= 0 ? transaction.categoryId() : QVariant());
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
    query.prepare("SELECT id, date, type, description, category_id, amount FROM transactions WHERE id = :id");
    query.bindValue(":id", id);
    
    if (query.exec() && query.next()) {
        return Transaction(
            query.value(0).toInt(),
            QDate::fromString(query.value(1).toString(), Qt::ISODate),
            Transaction::stringToType(query.value(2).toString()),
            query.value(3).toString(),
            query.value(4).isNull() ? -1 : query.value(4).toInt(),
            query.value(5).toDouble()
        );
    }
    
    return Transaction();
}

QList<Transaction> Database::getTransactions(const QDate& fromDate, const QDate& toDate,
                                             int categoryId, Transaction::Type type)
{
    QList<Transaction> result;
    
    QString sql = "SELECT id, date, type, description, category_id, amount FROM transactions WHERE 1=1";
    
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
        query.bindValue(":type", type == Transaction::Type::Income ? "income" : "expense");
    }
    
    if (query.exec()) {
        while (query.next()) {
            result.append(Transaction(
                query.value(0).toInt(),
                QDate::fromString(query.value(1).toString(), Qt::ISODate),
                Transaction::stringToType(query.value(2).toString()),
                query.value(3).toString(),
                query.value(4).isNull() ? -1 : query.value(4).toInt(),
                query.value(5).toDouble()
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
    query.prepare("INSERT INTO categories (name) VALUES (:name)");
    query.bindValue(":name", category.name());
    
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
    QSqlQuery check;
    check.prepare("SELECT COUNT(*) FROM transactions WHERE category_id = :id");
    check.bindValue(":id", id);
    if (check.exec() && check.next() && check.value(0).toInt() > 0) {
        return false;
    }
    
    QSqlQuery query;
    query.prepare("DELETE FROM categories WHERE id = :id");
    query.bindValue(":id", id);
    
    if (!query.exec()) {
        qWarning() << "Ошибка удаления категории:" << query.lastError().text();
        return false;
    }
    
    emit categoryDeleted(id);
    return true;
}

QList<Category> Database::getCategories()
{
    QList<Category> result;
    
    QSqlQuery query("SELECT id, name FROM categories ORDER BY name");
    while (query.next()) {
        result.append(Category(
            query.value(0).toInt(),
            query.value(1).toString()
        ));
    }
    
    return result;
}

Category Database::getCategory(int id)
{
    QSqlQuery query;
    query.prepare("SELECT id, name FROM categories WHERE id = :id");
    query.bindValue(":id", id);
    
    if (query.exec() && query.next()) {
        return Category(
            query.value(0).toInt(),
            query.value(1).toString()
        );
    }
    
    return Category();
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
    endDate = endDate.addDays(-1); // the last day of the previous month
    
    if (!endDate.isValid() || endDate < QDate(2000, 1, 1)) {
        return 0.0;
    }
    
    QSqlQuery query;
    query.prepare(R"(
        SELECT 
            COALESCE(SUM(CASE WHEN type = 'income' THEN amount ELSE 0 END), 0) -
            COALESCE(SUM(CASE WHEN type = 'expense' THEN amount ELSE 0 END), 0)
        FROM transactions 
        WHERE date <= :endDate
    )");
    query.bindValue(":endDate", endDate.toString(Qt::ISODate));
    
    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }
    
    return 0.0;
}
