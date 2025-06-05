#ifndef DATABASE_SERVICE_H
#define DATABASE_SERVICE_H

#include <memory>
#include "../DbManager/DbManager.h"

/**
 * @brief The DatabaseService class provides a centralized singleton for database connections.
 * 
 * This service allows for a single database connection point that can be accessed
 * throughout the application, ensuring consistency and better resource management.
 */
class DatabaseService {
public:
    /**
     * @brief Get the singleton instance of the DatabaseService.
     * @return Reference to the singleton DatabaseService instance.
     */
    static DatabaseService& getInstance();
    
    /**
     * @brief Connect to the database.
     * @param dbPath Path to the database file.
     * @return True if connection was successful, false otherwise.
     */
    bool connectDatabase(const QString& dbPath);
    
    /**
     * @brief Check if database is connected.
     * @return True if database is connected, false otherwise.
     */
    bool isDatabaseConnected() const;
    
    /**
     * @brief Get the underlying DbManager instance.
     * @return Pointer to the DbManager instance. This is owned by DatabaseService.
     */
    DbManager* getDbManager() const;

private:
    // Private constructor and destructor for singleton pattern
    DatabaseService();
    ~DatabaseService();
    
    // Delete copy and move constructors/operators to enforce singleton
    DatabaseService(const DatabaseService&) = delete;
    DatabaseService& operator=(const DatabaseService&) = delete;
    DatabaseService(DatabaseService&&) = delete;
    DatabaseService& operator=(DatabaseService&&) = delete;
    
    std::unique_ptr<DbManager> m_dbManager; ///< Database manager instance
};

#endif // DATABASE_SERVICE_H 