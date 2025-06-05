#ifndef YARACONFIG_H
#define YARACONFIG_H

#include <chrono>
#include <cstddef>

namespace YaraConfig {
    // Performance settings
    constexpr size_t DEFAULT_MAX_FILE_SIZE = 100 * 1024 * 1024; // 100MB
    constexpr std::chrono::seconds DEFAULT_SCAN_TIMEOUT{30};
    constexpr size_t DEFAULT_MAX_MEMORY_USAGE = 512 * 1024 * 1024; // 512MB
    
    // Rule management
    constexpr auto RULES_UPDATE_INTERVAL = std::chrono::hours{24};
    constexpr size_t MAX_RULES_PER_CATEGORY = 1000;
    
    // Scanning options
    constexpr int DEFAULT_SCAN_FLAGS = 0;
    constexpr bool ENABLE_RULE_PROFILING = true;
    constexpr bool ENABLE_DETAILED_LOGGING = false;
}

#endif // YARACONFIG_H
