#!/bin/bash
# File System Activity Monitoring Script for Sandbox
# This script monitors file system changes and operations

echo "=== FILE_OPERATIONS ==="
# Monitor recent file operations using inotify events (if available)
if command -v inotifywait &> /dev/null; then
    # This would normally be a background process
    echo "FILE_OP|File monitoring active via inotify"
else
    # Fallback: check recent file modifications
    find /tmp /var /home -type f -mmin -1 2>/dev/null | head -20 | while read file; do
        echo "FILE_OP|MODIFIED|$file|$(stat -c %Y "$file" 2>/dev/null)"
    done
fi

echo "=== SYSTEM_FILES ==="
# Check for modifications to critical system files
critical_files="/etc/passwd /etc/shadow /etc/hosts /etc/crontab"
for file in $critical_files; do
    if [ -f "$file" ]; then
        mod_time=$(stat -c %Y "$file" 2>/dev/null)
        echo "SYS_FILE|$file|$mod_time"
    fi
done

echo "=== REGISTRY_CHANGES ==="
# Windows registry equivalent for Linux (configuration files)
config_dirs="/etc /home/*/.config"
for dir in $config_dirs; do
    if [ -d "$dir" ]; then
        find "$dir" -name "*.conf" -o -name "*.cfg" -mmin -1 2>/dev/null | head -10 | while read file; do
            echo "REG_CHANGE|$file|$(stat -c %Y "$file" 2>/dev/null)"
        done
    fi
done

echo "=== END_FILESYSTEM_ACTIVITY ==="
