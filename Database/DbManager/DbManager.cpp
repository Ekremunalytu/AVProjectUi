//
// Created by Ekrem Ünal on 9.05.2025.
//

#include "DbManager.h"      // Definition of the DbManager class
#include <QSqlDatabase>     // Qt's class for database connections
#include <QSqlQuery>        // Qt's class for executing SQL queries
#include <QSqlError>        // Qt's class for database error information
#include <QUuid>            // For generating unique connection names (recommended improvement)

/**
 * @struct DbManager::Impl
 * @brief Private implementation details for the DbManager class (Pimpl idiom).
 *
 * This struct encapsulates the actual database connection object and any other
 * private members needed by DbManager, hiding Qt-specific SQL details from
 * the DbManager header file.
 */
struct DbManager::Impl {
    QSqlDatabase db; ///< The Qt SQL database connection object.
    // QString connectionName; // Recommended: Store a unique connection name here.
                              // e.g., initialized in Impl's constructor:
                              // Impl() : connectionName(QUuid::createUuid().toString(QUuid::WithoutBraces)) {}

    /**
     * @brief Default constructor for the Impl struct.
     * If a unique connectionName member were used, it would be initialized here.
     */
    Impl() = default;
};

/**
 * @brief Constructs a DbManager object.
 * Initializes the private implementation (Pimpl).
 * @note This constructor is marked noexcept. However, std::make_unique can throw
 * std::bad_alloc if memory allocation fails. In such a case, as no C++
 * exception can escape a noexcept function, std::terminate will be called.
 * This is often an acceptable outcome if a core object cannot be allocated.
 */
DbManager::DbManager() noexcept
    : pImpl(std::make_unique<Impl>()) {}

/**
 * @brief Destroys the DbManager object.
 * The std::unique_ptr pImpl will automatically delete the Impl object.
 * The QSqlDatabase object within Impl will have its destructor called,
 * which closes the database connection if it's open and removes it
 * if it was added with a unique name not shared by other QSqlDatabase instances.
 * @note Marked noexcept as destructors should generally not throw exceptions.
 */
DbManager::~DbManager() noexcept = default;

/**
 * @brief Connects to the SQLite database specified by the given path.
 * @param dbPath The file system path to the SQLite database file.
 * @return std::error_code An error code indicating the result of the operation.
 * Returns a default-constructed std::error_code (evaluates to false) on success.
 * @note This function is marked noexcept; all errors are reported via std::error_code.
 * It uses QSqlDatabase::addDatabase() with the default connection name.
 * For robustness, especially if multiple DbManager instances or other Qt SQL
 * operations exist, using a unique connection name per instance is recommended.
 * (e.g., QSqlDatabase::addDatabase("QSQLITE", pImpl->connectionName)).
 */
std::error_code DbManager::connectDatabase(QStringView dbPath) noexcept {
    // If pImpl is null (should not happen if constructor didn't terminate due to bad_alloc)
    if (!pImpl) {
        return std::make_error_code(std::errc::state_not_recoverable);
    }

    auto& db = pImpl->db; // Convenience reference to the QSqlDatabase object in Impl.

    // If the database connection is already valid and open, consider it a success.
    if (db.isValid() && db.isOpen()) {
        // Note: If called with a different dbPath than the currently open one,
        // this implementation does not switch databases; it just returns success.
        return {}; // Default std::error_code signifies success.
    }

    // Add a new SQLite database connection using the default connection name.
    // If a connection with the default name already exists, it will be used.
    // Consider using a unique connection name:
    // db = QSqlDatabase::addDatabase("QSQLITE", pImpl->connectionName);
    db = QSqlDatabase::addDatabase("QSQLITE");
    if (!db.isValid()) { // Check if the driver was loaded correctly
        // qWarning() << "Failed to add QSQLITE database driver:" << db.lastError().text();
        return std::make_error_code(std::errc::operation_not_supported); // Or a more specific error
    }
    db.setDatabaseName(QString(dbPath)); // Set the database file path.

    // Attempt to open the database connection.
    if (!db.open()) {
        // qWarning() << "Failed to open database:" << dbPath << "Error:" << db.lastError().text();
        return std::make_error_code(std::errc::io_error); // I/O error or connection refused.
    }

    // qInfo() << "Database connected successfully:" << dbPath;
    return {}; // Success.
}

/**
 * @brief Checks if a connection to the database is currently established and valid.
 * @return True if the database is connected and valid, false otherwise.
 * @note Marked noexcept as this is expected to be a lightweight state check.
 */
bool DbManager::isDatabaseConnected() const noexcept {
    if (!pImpl) { // Should not happen in normal operation.
        return false;
    }
    return pImpl->db.isValid() && pImpl->db.isOpen();
}

/**
 * @brief Checks if a given SHA256 hash exists in the 'signatures' table.
 * @param sha256Hash The SHA256 hash string to search for.
 * @param[out] ec An std::error_code that will be set if an error occurs during the operation.
 * It is cleared if the operation is successful (even if hash is not found).
 * @return True if the SHA256 hash exists and no error occurred, false otherwise.
 * If an error occurs (ec is set), the return value should be considered unreliable.
 */
bool DbManager::isSha256Exists(QStringView sha256Hash, std::error_code& ec) {
    // Ensure the database is connected before proceeding.
    if (!isDatabaseConnected()) {
        ec = std::make_error_code(std::errc::not_connected);
        return false;
    }

    // Create a QSqlQuery object associated with the Pimpl's database connection.
    QSqlQuery query(pImpl->db);

    // Prepare the SQL query with a placeholder for the hash to prevent SQL injection.
    // "SELECT 1" is an optimization to just check for existence without retrieving data.
    // "LIMIT 1" stops the search after the first match is found.
    query.prepare("SELECT 1 FROM signatures WHERE sha256 = :h LIMIT 1");
    query.bindValue(":h", QString(sha256Hash)); // Bind the actual hash value.

    // Execute the query.
    if (!query.exec()) {
        // qWarning() << "Failed to execute isSha256Exists query:" << query.lastError().text();
        ec = std::make_error_code(std::errc::io_error); // Indicate a database query error.
        return false;
    }

    // query.next() attempts to move to the first record.
    // If it returns true, a matching record was found.
    bool exists = query.next();
    ec.clear(); // Clear any previous error code; operation itself was successful.
    return exists;
}

/**
 * @brief Retrieves the total count of records in the 'signatures' table.
 * @param[out] ec An std::error_code that will be set if an error occurs during the operation.
 * It is cleared if the operation is successful.
 * @return The total count of signatures if successful, or -1 (or 0) if an error occurs.
 * The value -1 is a common convention for "error" or "not found" for counts,
 * but `ec` should be the primary indicator of an error.
 */
long DbManager::getSignatureCount(std::error_code& ec) {
    // Ensure the database is connected.
    if (!isDatabaseConnected()) {
        ec = std::make_error_code(std::errc::not_connected);
        return -1; // Or 0, depending on desired error signaling convention for return value.
    }

    QSqlQuery query(pImpl->db);
    query.prepare("SELECT COUNT(*) FROM signatures");

    // Execute the query.
    if (!query.exec()) {
        // qWarning() << "Failed to execute getSignatureCount query:" << query.lastError().text();
        ec = std::make_error_code(std::errc::io_error);
        return -1; // Or 0.
    }

    // A COUNT(*) query should always return exactly one row, even if the count is 0.
    if (query.next()) {
        bool ok; // To check if the conversion to toLongLong was successful.
        long count = query.value(0).toLongLong(&ok);
        if (ok) {
            ec.clear(); // Successfully retrieved and converted the count.
            return count;
        } else {
            // qWarning() << "Failed to convert signature count to long:" << query.value(0).toString();
            ec = std::make_error_code(std::errc::invalid_argument); // Data conversion error.
            return -1; // Or 0.
        }
    }

    // This part should ideally not be reached for a successful COUNT(*) query.
    // If query.next() is false, it means no row was returned, which is unexpected.
    // qWarning() << "getSignatureCount query did not return a row. Error:" << query.lastError().text();
    ec = std::make_error_code(std::errc::protocol_error); // Or a more specific "unexpected result" error.
    return -1; // Or 0.
}