#ifndef IDBMANAGER_H
#define IDBMANAGER_H

#include <system_error> // For std::error_code
#include <QStringView>    // For QStringView

/**
 * @interface IDbManager
 * @brief Defines an interface for database management operations.
 *
 * This interface provides a contract for classes that manage database
 * connections, queries, and other related functionalities. It is designed
 * to be non-copyable and non-movable.
 */
class IDbManager {
public:
  /**
   * @brief Default constructor.
   */
  IDbManager() = default;

  /**
   * @brief Pure virtual destructor.
   * Ensures proper cleanup of derived class resources.
   * Marked noexcept as destructors should not throw exceptions.
   */
  virtual ~IDbManager() noexcept = 0;

  // Prevention of copying or moving interfaces
  IDbManager(const IDbManager&) = delete;
  IDbManager& operator=(const IDbManager&) = delete;
  IDbManager(IDbManager&&) = delete;
  IDbManager& operator=(IDbManager&&) = delete;

  // Interface functions

  /**
   * @brief Connects to the database specified by the given path.
   * For local databases, this path typically refers to a file.
   * @param dbPath The path to the database file or connection string.
   * @return std::error_code() (default-constructed, indicating success) if the connection is successful,
   * otherwise an error_code पानीtaining the failure reason.
   * @note This function is marked noexcept, assuming all connection errors
   * (e.g., file not found, permissions issues) are reported via std::error_code
   * and no C++ exceptions are thrown.
   */
  virtual  std::error_code connectDatabase(QStringView dbPath) noexcept = 0;

  /**
   * @brief Checks if a connection to the database is currently established.
   * @return True if the database is connected, false otherwise.
   * @note This function is marked noexcept as it's expected to be a lightweight check.
   */
  virtual  bool isDatabaseConnected() const noexcept = 0;

  /**
   * @brief Checks if a given SHA256 hash exists in the database.
   * @param sha256Hash The SHA256 hash string to search for.
   * @param[out] ec A std::error_code reference that will be set if an error occurs during
   * the database operation. On success, it will be cleared (or hold a
   * default-constructed error_code).
   * @return True if the SHA256 hash exists and no error occurred, false otherwise.
   * If an error occurs (ec is set), the return value may not be reliable.
   * @note This function is marked noexcept, implying all database query errors
   * are reported via std::error_code and no C++ exceptions are propagated.
   */
  virtual  bool isSha256Exists(QStringView sha256Hash, std::error_code& ec) = 0;

  /**
   * @brief Retrieves the total count of signatures (or a relevant metric) from the database.
   * @param[out] ec A std::error_code reference that will be set if an error occurs during
   * the database operation. On success, it will be cleared (or hold a
   * default-constructed error_code).
   * @return The count of signatures if the operation is successful and no error occurred.
   * If an error occurs (ec is set), the return value (e.g., 0 or -1) may indicate
   * failure, but `ec` should be checked for the definitive error status.
   * @note This function is marked noexcept, implying all database query errors
   * are reported via std::error_code and no C++ exceptions are propagated.
   */
  virtual  long getSignatureCount(std::error_code& ec) = 0;
};

// Pure virtual Destructor definition
/**
 * @brief Default implementation for the pure virtual destructor of IDbManager.
 * This is required for the linker even if the destructor is pure virtual and has no explicit body in the class.
 */
inline IDbManager::~IDbManager() noexcept = default;

#endif // IDBMANAGER_H