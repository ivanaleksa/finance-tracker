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

    // investment tables
    if (!createInvestmentTables()) {
        return false;
    }

    // default currencies
    insertDefaultCurrencies();

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

double Database::getTotalSavings()
{
    QSqlQuery query;
    query.prepare("SELECT COALESCE(SUM(amount), 0) FROM transactions WHERE type = 'savings'");

    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }

    return 0.0;
}

int Database::getOrCreateInvestmentIncomeCategory()
{
    QSqlQuery query;
    query.prepare("SELECT id FROM categories WHERE name = :name AND parent_id = -1");
    query.bindValue(":name", "Доход с инвестиций");

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    // Create if not exists
    query.prepare("INSERT INTO categories (name, parent_id) VALUES (:name, -1)");
    query.bindValue(":name", "Доход с инвестиций");

    if (query.exec()) {
        return query.lastInsertId().toInt();
    }

    return -1;
}

// ========== INVESTMENT TABLES ==========

bool Database::createInvestmentTables()
{
    QSqlQuery query;

    // Investment categories table
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS investment_categories (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE
        )
    )")) {
        qWarning() << "Error creating investment_categories:" << query.lastError().text();
        return false;
    }

    // Countries table
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS countries (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE
        )
    )")) {
        qWarning() << "Error creating countries:" << query.lastError().text();
        return false;
    }

    // Currencies table
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS currencies (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            code TEXT NOT NULL UNIQUE,
            name TEXT NOT NULL
        )
    )")) {
        qWarning() << "Error creating currencies:" << query.lastError().text();
        return false;
    }

    // Snapshots table
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS snapshots (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            date TEXT NOT NULL,
            description TEXT
        )
    )")) {
        qWarning() << "Error creating snapshots:" << query.lastError().text();
        return false;
    }

    // Snapshot currency rates table
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS snapshot_currency_rates (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            snapshot_id INTEGER NOT NULL,
            currency_id INTEGER NOT NULL,
            rate REAL NOT NULL,
            FOREIGN KEY (snapshot_id) REFERENCES snapshots(id) ON DELETE CASCADE,
            FOREIGN KEY (currency_id) REFERENCES currencies(id),
            UNIQUE(snapshot_id, currency_id)
        )
    )")) {
        qWarning() << "Error creating snapshot_currency_rates:" << query.lastError().text();
        return false;
    }

    // Snapshot positions table
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS snapshot_positions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            snapshot_id INTEGER NOT NULL,
            name TEXT NOT NULL,
            category_id INTEGER,
            price REAL NOT NULL,
            quantity REAL NOT NULL,
            country_id INTEGER,
            currency_id INTEGER NOT NULL,
            FOREIGN KEY (snapshot_id) REFERENCES snapshots(id) ON DELETE CASCADE,
            FOREIGN KEY (category_id) REFERENCES investment_categories(id),
            FOREIGN KEY (country_id) REFERENCES countries(id),
            FOREIGN KEY (currency_id) REFERENCES currencies(id)
        )
    )")) {
        qWarning() << "Error creating snapshot_positions:" << query.lastError().text();
        return false;
    }

    // Withdrawals table
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS withdrawals (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            date TEXT NOT NULL,
            amount REAL NOT NULL,
            comment TEXT,
            transaction_id INTEGER,
            FOREIGN KEY (transaction_id) REFERENCES transactions(id)
        )
    )")) {
        qWarning() << "Error creating withdrawals:" << query.lastError().text();
        return false;
    }

    // Indexes
    query.exec("CREATE INDEX IF NOT EXISTS idx_snapshots_date ON snapshots(date)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_snapshot_positions_snapshot ON snapshot_positions(snapshot_id)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_snapshot_rates_snapshot ON snapshot_currency_rates(snapshot_id)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_withdrawals_date ON withdrawals(date)");

    return true;
}

void Database::insertDefaultCurrencies()
{
    QSqlQuery query;
    query.exec("SELECT COUNT(*) FROM currencies");
    if (query.next() && query.value(0).toInt() > 0) {
        return;
    }

    // RUB - base currency
    query.prepare("INSERT INTO currencies (code, name) VALUES (:code, :name)");
    query.bindValue(":code", "RUB");
    query.bindValue(":name", "Рубль");
    query.exec();

    // USD
    query.prepare("INSERT INTO currencies (code, name) VALUES (:code, :name)");
    query.bindValue(":code", "USD");
    query.bindValue(":name", "Доллар США");
    query.exec();
}

// ========== INVESTMENT CATEGORIES ==========

bool Database::addInvestmentCategory(InvestmentCategory& category)
{
    QSqlQuery query;
    query.prepare("INSERT INTO investment_categories (name) VALUES (:name)");
    query.bindValue(":name", category.name());

    if (!query.exec()) {
        qWarning() << "Error adding investment category:" << query.lastError().text();
        return false;
    }

    category.setId(query.lastInsertId().toInt());
    emit investmentDataChanged();
    return true;
}

bool Database::updateInvestmentCategory(int id, const QString& newName)
{
    QSqlQuery query;
    query.prepare("UPDATE investment_categories SET name = :name WHERE id = :id");
    query.bindValue(":name", newName);
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "Error updating investment category:" << query.lastError().text();
        return false;
    }

    emit investmentDataChanged();
    return true;
}

bool Database::deleteInvestmentCategory(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM investment_categories WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "Error deleting investment category:" << query.lastError().text();
        return false;
    }

    emit investmentDataChanged();
    return true;
}

QList<InvestmentCategory> Database::getInvestmentCategories()
{
    QList<InvestmentCategory> result;

    QSqlQuery query("SELECT id, name FROM investment_categories ORDER BY name");
    while (query.next()) {
        result.append(InvestmentCategory(
            query.value(0).toInt(),
            query.value(1).toString()
        ));
    }

    return result;
}

InvestmentCategory Database::getInvestmentCategory(int id)
{
    QSqlQuery query;
    query.prepare("SELECT id, name FROM investment_categories WHERE id = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        return InvestmentCategory(
            query.value(0).toInt(),
            query.value(1).toString()
        );
    }

    return InvestmentCategory();
}

// ========== COUNTRIES ==========

bool Database::addCountry(Country& country)
{
    QSqlQuery query;
    query.prepare("INSERT INTO countries (name) VALUES (:name)");
    query.bindValue(":name", country.name());

    if (!query.exec()) {
        qWarning() << "Error adding country:" << query.lastError().text();
        return false;
    }

    country.setId(query.lastInsertId().toInt());
    emit investmentDataChanged();
    return true;
}

bool Database::updateCountry(int id, const QString& newName)
{
    QSqlQuery query;
    query.prepare("UPDATE countries SET name = :name WHERE id = :id");
    query.bindValue(":name", newName);
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "Error updating country:" << query.lastError().text();
        return false;
    }

    emit investmentDataChanged();
    return true;
}

bool Database::deleteCountry(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM countries WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "Error deleting country:" << query.lastError().text();
        return false;
    }

    emit investmentDataChanged();
    return true;
}

QList<Country> Database::getCountries()
{
    QList<Country> result;

    QSqlQuery query("SELECT id, name FROM countries ORDER BY name");
    while (query.next()) {
        result.append(Country(
            query.value(0).toInt(),
            query.value(1).toString()
        ));
    }

    return result;
}

Country Database::getCountry(int id)
{
    QSqlQuery query;
    query.prepare("SELECT id, name FROM countries WHERE id = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        return Country(
            query.value(0).toInt(),
            query.value(1).toString()
        );
    }

    return Country();
}

// ========== CURRENCIES ==========

bool Database::addCurrency(Currency& currency)
{
    QSqlQuery query;
    query.prepare("INSERT INTO currencies (code, name) VALUES (:code, :name)");
    query.bindValue(":code", currency.code());
    query.bindValue(":name", currency.name());

    if (!query.exec()) {
        qWarning() << "Error adding currency:" << query.lastError().text();
        return false;
    }

    currency.setId(query.lastInsertId().toInt());
    emit investmentDataChanged();
    return true;
}

bool Database::updateCurrency(int id, const QString& code, const QString& name)
{
    QSqlQuery query;
    query.prepare("UPDATE currencies SET code = :code, name = :name WHERE id = :id");
    query.bindValue(":code", code);
    query.bindValue(":name", name);
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "Error updating currency:" << query.lastError().text();
        return false;
    }

    emit investmentDataChanged();
    return true;
}

bool Database::deleteCurrency(int id)
{
    // Don't allow deleting RUB
    Currency curr = getCurrency(id);
    if (curr.code() == "RUB") {
        qWarning() << "Cannot delete base currency RUB";
        return false;
    }

    QSqlQuery query;
    query.prepare("DELETE FROM currencies WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "Error deleting currency:" << query.lastError().text();
        return false;
    }

    emit investmentDataChanged();
    return true;
}

QList<Currency> Database::getCurrencies()
{
    QList<Currency> result;

    QSqlQuery query("SELECT id, code, name FROM currencies ORDER BY code");
    while (query.next()) {
        result.append(Currency(
            query.value(0).toInt(),
            query.value(1).toString(),
            query.value(2).toString()
        ));
    }

    return result;
}

Currency Database::getCurrency(int id)
{
    QSqlQuery query;
    query.prepare("SELECT id, code, name FROM currencies WHERE id = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        return Currency(
            query.value(0).toInt(),
            query.value(1).toString(),
            query.value(2).toString()
        );
    }

    return Currency();
}

Currency Database::getCurrencyByCode(const QString& code)
{
    QSqlQuery query;
    query.prepare("SELECT id, code, name FROM currencies WHERE code = :code");
    query.bindValue(":code", code);

    if (query.exec() && query.next()) {
        return Currency(
            query.value(0).toInt(),
            query.value(1).toString(),
            query.value(2).toString()
        );
    }

    return Currency();
}

// ========== SNAPSHOTS ==========

bool Database::addSnapshot(Snapshot& snapshot)
{
    QSqlQuery query;

    // Insert snapshot
    query.prepare("INSERT INTO snapshots (date, description) VALUES (:date, :description)");
    query.bindValue(":date", snapshot.date().toString(Qt::ISODate));
    query.bindValue(":description", snapshot.description());

    if (!query.exec()) {
        qWarning() << "Error adding snapshot:" << query.lastError().text();
        return false;
    }

    int snapshotId = query.lastInsertId().toInt();
    snapshot.setId(snapshotId);

    // Insert currency rates
    QMapIterator<int, double> rateIt(snapshot.currencyRates());
    while (rateIt.hasNext()) {
        rateIt.next();
        query.prepare("INSERT INTO snapshot_currency_rates (snapshot_id, currency_id, rate) VALUES (:snapshot_id, :currency_id, :rate)");
        query.bindValue(":snapshot_id", snapshotId);
        query.bindValue(":currency_id", rateIt.key());
        query.bindValue(":rate", rateIt.value());
        query.exec();
    }

    // Insert positions
    for (const SnapshotPosition& pos : snapshot.positions()) {
        query.prepare(R"(
            INSERT INTO snapshot_positions (snapshot_id, name, category_id, price, quantity, country_id, currency_id)
            VALUES (:snapshot_id, :name, :category_id, :price, :quantity, :country_id, :currency_id)
        )");
        query.bindValue(":snapshot_id", snapshotId);
        query.bindValue(":name", pos.name());
        query.bindValue(":category_id", pos.categoryId() >= 0 ? pos.categoryId() : QVariant());
        query.bindValue(":price", pos.price());
        query.bindValue(":quantity", pos.quantity());
        query.bindValue(":country_id", pos.countryId() >= 0 ? pos.countryId() : QVariant());
        query.bindValue(":currency_id", pos.currencyId());
        query.exec();
    }

    emit snapshotAdded(snapshot);
    emit investmentDataChanged();
    return true;
}

bool Database::deleteSnapshot(int id)
{
    QSqlQuery query;

    // Delete positions (CASCADE should handle this, but be explicit)
    query.prepare("DELETE FROM snapshot_positions WHERE snapshot_id = :id");
    query.bindValue(":id", id);
    query.exec();

    // Delete currency rates
    query.prepare("DELETE FROM snapshot_currency_rates WHERE snapshot_id = :id");
    query.bindValue(":id", id);
    query.exec();

    // Delete snapshot
    query.prepare("DELETE FROM snapshots WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "Error deleting snapshot:" << query.lastError().text();
        return false;
    }

    emit snapshotDeleted(id);
    emit investmentDataChanged();
    return true;
}

Snapshot Database::getSnapshot(int id)
{
    QSqlQuery query;
    query.prepare("SELECT id, date, description FROM snapshots WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec() || !query.next()) {
        return Snapshot();
    }

    Snapshot snapshot(
        query.value(0).toInt(),
        QDate::fromString(query.value(1).toString(), Qt::ISODate),
        query.value(2).toString()
    );

    // Load currency rates
    query.prepare("SELECT currency_id, rate FROM snapshot_currency_rates WHERE snapshot_id = :id");
    query.bindValue(":id", id);
    if (query.exec()) {
        while (query.next()) {
            snapshot.setCurrencyRate(query.value(0).toInt(), query.value(1).toDouble());
        }
    }

    // Load positions with helper data
    query.prepare(R"(
        SELECT p.id, p.snapshot_id, p.name, p.category_id, p.price, p.quantity, p.country_id, p.currency_id,
               ic.name as category_name, co.name as country_name, cu.code as currency_code
        FROM snapshot_positions p
        LEFT JOIN investment_categories ic ON p.category_id = ic.id
        LEFT JOIN countries co ON p.country_id = co.id
        LEFT JOIN currencies cu ON p.currency_id = cu.id
        WHERE p.snapshot_id = :id
    )");
    query.bindValue(":id", id);

    if (query.exec()) {
        while (query.next()) {
            SnapshotPosition pos(
                query.value(0).toInt(),
                query.value(1).toInt(),
                query.value(2).toString(),
                query.value(3).isNull() ? -1 : query.value(3).toInt(),
                query.value(4).toDouble(),
                query.value(5).toDouble(),
                query.value(6).isNull() ? -1 : query.value(6).toInt(),
                query.value(7).toInt()
            );
            pos.setCategoryName(query.value(8).toString());
            pos.setCountryName(query.value(9).toString());
            pos.setCurrencyCode(query.value(10).toString());
            pos.setCurrencyRate(snapshot.getCurrencyRate(pos.currencyId()));
            snapshot.addPosition(pos);
        }
    }

    return snapshot;
}

Snapshot Database::getLatestSnapshot()
{
    QSqlQuery query("SELECT id FROM snapshots ORDER BY date DESC, id DESC LIMIT 1");
    if (query.exec() && query.next()) {
        return getSnapshot(query.value(0).toInt());
    }
    return Snapshot();
}

Snapshot Database::getLatestSnapshotBefore(const QDate& date)
{
    QSqlQuery query;
    query.prepare("SELECT id FROM snapshots WHERE date <= :date ORDER BY date DESC, id DESC LIMIT 1");
    query.bindValue(":date", date.toString(Qt::ISODate));

    if (query.exec() && query.next()) {
        return getSnapshot(query.value(0).toInt());
    }
    return Snapshot();
}

QList<Snapshot> Database::getAllSnapshots()
{
    QList<Snapshot> result;

    QSqlQuery query("SELECT id FROM snapshots ORDER BY date DESC, id DESC");
    while (query.next()) {
        result.append(getSnapshot(query.value(0).toInt()));
    }

    return result;
}

QMap<QDate, double> Database::getWeeklyPortfolioHistory(int weeks)
{
    QMap<QDate, double> result;

    QDate today = QDate::currentDate();
    // Find the most recent Sunday
    int daysToSunday = (today.dayOfWeek() % 7);
    QDate lastSunday = today.addDays(-daysToSunday);

    for (int i = 0; i < weeks; ++i) {
        QDate sunday = lastSunday.addDays(-7 * i);
        Snapshot snapshot = getLatestSnapshotBefore(sunday);

        if (snapshot.isValid()) {
            result[sunday] = snapshot.totalInRub();
        }
    }

    return result;
}

// ========== WITHDRAWALS ==========

bool Database::addWithdrawal(Withdrawal& withdrawal)
{
    // First, create income transaction
    int categoryId = getOrCreateInvestmentIncomeCategory();

    Transaction incomeTransaction;
    incomeTransaction.setDate(withdrawal.date());
    incomeTransaction.setType(Transaction::Type::Income);
    incomeTransaction.setDescription(withdrawal.comment().isEmpty() ? "Вывод с инвестиций" : withdrawal.comment());
    incomeTransaction.setCategoryId(categoryId);
    incomeTransaction.setAmount(withdrawal.amount());

    if (!addTransaction(incomeTransaction)) {
        return false;
    }

    withdrawal.setTransactionId(incomeTransaction.id());

    // Now add withdrawal record
    QSqlQuery query;
    query.prepare("INSERT INTO withdrawals (date, amount, comment, transaction_id) VALUES (:date, :amount, :comment, :transaction_id)");
    query.bindValue(":date", withdrawal.date().toString(Qt::ISODate));
    query.bindValue(":amount", withdrawal.amount());
    query.bindValue(":comment", withdrawal.comment());
    query.bindValue(":transaction_id", withdrawal.transactionId());

    if (!query.exec()) {
        qWarning() << "Error adding withdrawal:" << query.lastError().text();
        // Rollback transaction
        deleteTransaction(incomeTransaction.id());
        return false;
    }

    withdrawal.setId(query.lastInsertId().toInt());
    emit withdrawalAdded(withdrawal);
    emit investmentDataChanged();
    return true;
}

bool Database::deleteWithdrawal(int id)
{
    Withdrawal withdrawal = getWithdrawal(id);
    if (!withdrawal.isValid()) {
        return false;
    }

    // Delete associated transaction
    if (withdrawal.transactionId() >= 0) {
        deleteTransaction(withdrawal.transactionId());
    }

    QSqlQuery query;
    query.prepare("DELETE FROM withdrawals WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "Error deleting withdrawal:" << query.lastError().text();
        return false;
    }

    emit withdrawalDeleted(id);
    emit investmentDataChanged();
    return true;
}

QList<Withdrawal> Database::getWithdrawals()
{
    QList<Withdrawal> result;

    QSqlQuery query("SELECT id, date, amount, comment, transaction_id FROM withdrawals ORDER BY date DESC, id DESC");
    while (query.next()) {
        result.append(Withdrawal(
            query.value(0).toInt(),
            QDate::fromString(query.value(1).toString(), Qt::ISODate),
            query.value(2).toDouble(),
            query.value(3).toString(),
            query.value(4).isNull() ? -1 : query.value(4).toInt()
        ));
    }

    return result;
}

Withdrawal Database::getWithdrawal(int id)
{
    QSqlQuery query;
    query.prepare("SELECT id, date, amount, comment, transaction_id FROM withdrawals WHERE id = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        return Withdrawal(
            query.value(0).toInt(),
            QDate::fromString(query.value(1).toString(), Qt::ISODate),
            query.value(2).toDouble(),
            query.value(3).toString(),
            query.value(4).isNull() ? -1 : query.value(4).toInt()
        );
    }

    return Withdrawal();
}

double Database::getTotalWithdrawals()
{
    QSqlQuery query;
    query.prepare("SELECT COALESCE(SUM(amount), 0) FROM withdrawals");

    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }

    return 0.0;
}

// ========== PORTFOLIO STATISTICS ==========

double Database::getPortfolioReturn()
{
    double totalSavings = getTotalSavings();
    if (totalSavings <= 0) {
        return 0.0;
    }

    double totalWithdrawals = getTotalWithdrawals();
    Snapshot latest = getLatestSnapshot();
    double currentValue = latest.isValid() ? latest.totalInRub() : 0.0;

    // Return = (Current + Withdrawals - Savings) / Savings * 100
    return (currentValue + totalWithdrawals - totalSavings) / totalSavings * 100.0;
}
