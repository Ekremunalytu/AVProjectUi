/**
 * @file DbManager.h
 * @brief Provides database management functionality for the application.
 * @author Ekrem Ünal
 * @date 9.05.2025
 */

#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <memory>         // For std::unique_ptr
#include <QStringView>    // For QStringView
#include "Interface/IDbManager.h" // For the IDbManager interface

// Add Qt String Literal namespace for Qt 6 compatibility
using namespace Qt::StringLiterals;

// Forward declaration for the Pimpl idiom
struct Impl;

/**
 * @class DbManager
 * @brief Concrete implementation of the IDbManager interface.
 *
 * This class provides the actual database management functionalities.
 * It uses the Pimpl (Pointer to Implementation) idiom to hide implementation
 * details, reduce compile-time dependencies, and improve ABI stability.
 */
class DbManager : public IDbManager {
public:
     /**
      * @brief Constructs a DbManager object.
      * Marked noexcept assuming the Pimpl object (pImpl) can be constructed without throwing,
      * or any exceptions during its construction are handled internally in a way that
      * doesn't propagate from this constructor.
      * Typically, `std::make_unique` for pImpl might throw `std::bad_alloc`.
      */
     explicit DbManager() noexcept;

     /**
      * @brief Destroys the DbManager object and its Pimpl.
      * Marked noexcept as destructors should generally not throw.
      * The std::unique_ptr (pImpl) will handle the deletion of the Impl object.
      */
     ~DbManager() noexcept override;

     /**
      * @brief Connects to the database.
      * @copydoc IDbManager::connectDatabase
      */
      std::error_code connectDatabase(QStringView dbPath) noexcept override;

     /**
      * @brief Checks if the database is connected.
      * @copydoc IDbManager::isDatabaseConnected
      */
      bool isDatabaseConnected() const noexcept override;

     /**
      * @brief Checks if a SHA256 hash exists in the database.
      * @copydoc IDbManager::isSha256Exists
      */
      bool isSha256Exists(QStringView sha256Hash, std::error_code& ec) const override;

     /**
      * @brief Gets the count of signatures from the database.
      * @copydoc IDbManager::getSignatureCount
      */
      long getSignatureCount(std::error_code& ec) const override;

private:
     /**
      * @struct Impl
      * @brief Forward declaration for the implementation struct (Pimpl).
      * The actual definition of this struct will be in the .cpp file.
      */
     struct Impl;
     std::unique_ptr<Impl> pImpl; ///< Pointer to the implementation details.
};

#endif //DBMANAGER_H