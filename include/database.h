#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QList>
#include <QDate>
#include "transaction.h"
#include "category.h"
#include "investmentcategory.h"
#include "country.h"
#include "currency.h"
#include "snapshot.h"
#include "snapshotposition.h"
#include "withdrawal.h"
#include "portfolioasset.h"
#include "assetoperation.h"

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

    // categories (for transactions)
    bool addCategory(Category& category);
    bool deleteCategory(int id);
    QList<Category> getCategories();
    Category getCategory(int id);
    QList<Category> getSubcategories(int parentId);
    bool updateCategoryName(int id, const QString& newName);
    QMap<QString, double> getSubcategoryTotalsByMonth(int year, int month, int categoryId);
    int getOrCreateInvestmentIncomeCategory();

    // statistic
    double getTotalByMonth(int year, int month, Transaction::Type type);
    double getTotalByYear(int year, Transaction::Type type);
    QMap<QString, double> getCategoryTotalsByMonth(int year, int month, Transaction::Type type);
    QMap<int, double> getMonthlyTotals(int year, Transaction::Type type);
    double getBalanceUpToMonth(int year, int month);
    double getSavingsByMonth(int year, int month);
    QMap<int, double> getMonthlySavings(int year);
    double getTotalSavings();

    // ========== INVESTMENTS ==========

    // Investment categories
    bool addInvestmentCategory(InvestmentCategory& category);
    bool updateInvestmentCategory(int id, const QString& newName);
    bool deleteInvestmentCategory(int id);
    QList<InvestmentCategory> getInvestmentCategories();
    InvestmentCategory getInvestmentCategory(int id);

    // Countries
    bool addCountry(Country& country);
    bool updateCountry(int id, const QString& newName);
    bool deleteCountry(int id);
    QList<Country> getCountries();
    Country getCountry(int id);

    // Currencies
    bool addCurrency(Currency& currency);
    bool updateCurrency(int id, const QString& code, const QString& name);
    bool updateCurrencyRate(int id, double rate);
    bool deleteCurrency(int id);
    QList<Currency> getCurrencies();
    Currency getCurrency(int id);
    Currency getCurrencyByCode(const QString& code);

    // Snapshots
    bool addSnapshot(Snapshot& snapshot);
    bool deleteSnapshot(int id);
    Snapshot getSnapshot(int id);
    Snapshot getLatestSnapshot();
    Snapshot getLatestSnapshotBefore(const QDate& date);
    QList<Snapshot> getAllSnapshots();
    QMap<QDate, double> getWeeklyPortfolioHistory(int weeks = 52);

    // Withdrawals
    bool addWithdrawal(Withdrawal& withdrawal);
    bool deleteWithdrawal(int id);
    QList<Withdrawal> getWithdrawals();
    Withdrawal getWithdrawal(int id);
    double getTotalWithdrawals();

    // Portfolio statistics
    double getPortfolioReturn();

    // ========== LIVE PORTFOLIO ==========

    // Portfolio Assets
    bool addPortfolioAsset(PortfolioAsset& asset);
    bool updatePortfolioAsset(const PortfolioAsset& asset);
    bool updatePortfolioAssetPrice(int id, double newPrice);
    bool deletePortfolioAsset(int id);
    PortfolioAsset getPortfolioAsset(int id);
    QList<PortfolioAsset> getActivePortfolioAssets();
    QList<PortfolioAsset> getAllPortfolioAssets();

    // Asset Operations
    bool addAssetOperation(AssetOperation& operation);
    bool deleteAssetOperation(int id);
    AssetOperation getAssetOperation(int id);
    QList<AssetOperation> getAssetOperations(int assetId);
    double getAssetTotalQuantity(int assetId);
    double getAssetTotalInvested(int assetId);

    // Create snapshot from live portfolio
    bool createSnapshotFromPortfolio(Snapshot& snapshot, const QMap<int, double>& currencyRates);

signals:
    void transactionAdded(const Transaction& transaction);
    void transactionDeleted(int id);
    void transactionUpdated(const Transaction& transaction);
    void categoryAdded(const Category& category);
    void categoryDeleted(int id);
    void dataChanged();

    // Investment signals
    void investmentDataChanged();
    void snapshotAdded(const Snapshot& snapshot);
    void snapshotDeleted(int id);
    void withdrawalAdded(const Withdrawal& withdrawal);
    void withdrawalDeleted(int id);

    // Portfolio signals
    void portfolioAssetAdded(const PortfolioAsset& asset);
    void portfolioAssetUpdated(const PortfolioAsset& asset);
    void portfolioAssetDeleted(int id);
    void assetOperationAdded(const AssetOperation& operation);
    void assetOperationDeleted(int id);
    void portfolioDataChanged();

private:
    Database();
    ~Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    bool createTables();
    bool createInvestmentTables();
    bool createPortfolioTables();
    void insertDefaultCategories();
    void insertDefaultCurrencies();

    QSqlDatabase m_db;
};

#endif // DATABASE_H
