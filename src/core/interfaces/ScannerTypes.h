/**
 * @file ScannerTypes.h
 * @brief Common type definitions for scanner implementations
 * @author AVProjectUi Team
 * @version 1.0
 * @date 2024
 */

#ifndef SCANNER_TYPES_H
#define SCANNER_TYPES_H

/**
 * @enum ScannerType
 * @brief Enumeration of different scanner implementation types
 * 
 * Defines the various types of scanners available in the system,
 * each providing different analysis capabilities and methodologies.
 */
enum class ScannerType {
    Basic = 0,      ///< Basic hash-based scanning against local database
    CDR,            ///< Content Disarm and Reconstruction scanner
    VirusTotal,     ///< Cloud-based VirusTotal API scanner
    Sandbox,        ///< Dynamic behavioral analysis in isolated environment
    Unknown         ///< Unknown or unspecified scanner type
};

/**
 * @enum ScanStatus
 * @brief Enumeration of scanner operation status states
 * 
 * Tracks the current state of a scanning operation throughout
 * its lifecycle from initiation to completion.
 */
enum class ScanStatus {
    Idle = 0,       ///< Scanner is idle and ready for new operations
    Scanning,       ///< Scanner is actively processing a file
    Completed,      ///< Scanning operation completed successfully
    Error,          ///< An error occurred during scanning
    Cancelled       ///< Scanning operation was cancelled by user
};

#endif // SCANNER_TYPES_H
