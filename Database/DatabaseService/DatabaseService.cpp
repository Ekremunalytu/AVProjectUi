#include "DatabaseService.h"
#include <QDebug>

/**
 * @brief Gets the singleton instance of the DatabaseService.
 * 
 * This implementation uses the Meyer's singleton pattern which is thread-safe
 * in C++11 and beyond.
 * 
 * @return Reference to the singleton DatabaseService instance.
 */
DatabaseService& DatabaseService::getInstance() {
    static DatabaseService instance;
    return instance;
}

/**
 * @brief Constructs a new DatabaseService instance.
 * 
 * This constructor is private to enforce the singleton pattern.
 * It initializes the DbManager instance.
 */
DatabaseService::DatabaseService()
    : m_dbManager(std::make_unique<DbManager>())
{
    // Database connection will be initialized explicitly via connectDatabase
}

/**
 * @brief Destroys the DatabaseService instance.
 * 
 * The unique_ptr to DbManager will be automatically deleted.
 */
DatabaseService::~DatabaseService() = default;

/**
 * @brief Connects to the database using the specified path.
 * 
 * @param dbPath Path to the database file.
 * @return True if the connection was successful, false otherwise.
 */
bool DatabaseService::connectDatabase(const QString& dbPath) {
    if (!m_dbManager) {
        return false;
    }
    
    auto ec = m_dbManager->connectDatabase(dbPath);
    return !ec; // If error code is cleared (false), connection is successful
}

/**
 * @brief Checks if the database is currently connected.
 * 
 * @return True if database is connected, false otherwise.
 */
bool DatabaseService::isDatabaseConnected() const {
    if (!m_dbManager) {
        return false;
    }
    
    return m_dbManager->isDatabaseConnected();
}

/**
 * @brief Gets the underlying DbManager instance.
 * 
 * @return Pointer to the DbManager instance.
 */
DbManager* DatabaseService::getDbManager() const {
    return m_dbManager.get();
} 