#ifndef SCANNER_TYPES_H
#define SCANNER_TYPES_H

/**
 * @brief Scanner types for different scanner implementations
 */
enum class ScannerType {
    Basic = 0,
    CDR,
    VirusTotal,
    Sandbox,
    Unknown
};

/**
 * @brief Scanner status for tracking scan progress
 */
enum class ScanStatus {
    Idle = 0,
    Scanning,
    Completed,
    Error,
    Cancelled
};

/**
 * @brief Scanner error codes
 */
enum class ScannerErrorCode {
    NoError = 0,
    FileNotFound,
    AccessDenied,
    ScanFailed,
    NetworkError,
    ApiError,
    DatabaseNotConnected,
    FileNotReadable,
    ScanInProgress,
    HashCalculationFailed,
    DatabaseQueryFailed,
    InvalidInput,
    MaliciousFileDetected,
    UnknownError
};

#endif // SCANNER_TYPES_H
