#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QList>
#include <QDate>
#include "transaction.h"
#include "category.h"

class Database : public QObject
{
    Q_OBJECT

public:
    static Database& instance();
    
    bool initialize();
    void close();
    
    // transactions
    bool addTransaction(Transaction& transaction);
    bool updateTransaction(const Transaction& transaction);
    bool deleteTransaction(int id);
    Transaction getTransaction(int id);
    QList<Transaction> getTransactions(const QDate& fromDate = QDate(),
                                       const QDate& toDate = QDate(),
                                       int categoryId = -1,
                                       Transaction::Type type = Transaction::Type::All);
    QList<Transaction> getTransactionsByMonth(int year, int month, Transaction::Type type);
    QList<Transaction> getTransactionsByYear(int year, Transaction::Type type);
    
    // categories
    bool addCategory(Category& category);
    bool deleteCategory(int id);
    QList<Category> getCategories();
    Category getCategory(int id);
    
    // statistic
    double getTotalByMonth(int year, int month, Transaction::Type type);
    double getTotalByYear(int year, Transaction::Type type);
    QMap<QString, double> getCategoryTotalsByMonth(int year, int month, Transaction::Type type);
    QMap<int, double> getMonthlyTotals(int year, Transaction::Type type);
    double getBalanceUpToMonth(int year, int month);

signals:
    void transactionAdded(const Transaction& transaction);
    void transactionDeleted(int id);
    void transactionUpdated(const Transaction& transaction);
    void categoryAdded(const Category& category);
    void categoryDeleted(int id);
    void dataChanged();

private:
    Database();
    ~Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    
    bool createTables();
    void insertDefaultCategories();
    
    QSqlDatabase m_db;
};

#endif // DATABASE_H
