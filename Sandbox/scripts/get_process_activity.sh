#!/bin/bash
# Process Activity Monitoring Script for Sandbox
# This script monitors process creation, injection, and suspicious behaviors

echo "=== PROCESS_CREATION ==="
# Monitor recent process creations
ps -eo pid,ppid,cmd,etime --sort=etime | head -20 | while read line; do
    echo "PROC_CREATE|$line"
done

echo "=== PROCESS_INJECTION ==="
# Look for signs of process injection
for pid in $(ps -eo pid --no-headers | head -10); do
    if [ -d "/proc/$pid" ]; then
        # Check memory maps for suspicious patterns
        if [ -r "/proc/$pid/maps" ]; then
            # Look for memory regions that might indicate injection
            injection_signs=$(grep -c "rwx\|heap.*x" "/proc/$pid/maps" 2>/dev/null)
            if [ "$injection_signs" -gt 0 ]; then
                proc_name=$(ps -p "$pid" -o comm= 2>/dev/null)
                echo "PROC_INJ|PID:$pid|NAME:$proc_name|SIGNS:$injection_signs"
            fi
        fi
    fi
done

echo "=== PROCESS_HOLLOWING ==="
# Monitor for process hollowing indicators
for pid in $(ps -eo pid --no-headers | head -10); do
    if [ -d "/proc/$pid" ]; then
        # Check if process executable path matches what's expected
        exe_link="/proc/$pid/exe"
        if [ -L "$exe_link" ]; then
            original_exe=$(readlink "$exe_link" 2>/dev/null)
            if [ $? -eq 0 ] && [[ "$original_exe" == *"(deleted)"* ]]; then
                proc_name=$(ps -p "$pid" -o comm= 2>/dev/null)
                echo "PROC_HOLLOW|PID:$pid|NAME:$proc_name|EXE:$original_exe"
            fi
        fi
    fi
done

echo "=== PERSISTENCE_MECHANISMS ==="
# Check for persistence mechanisms
persistence_locations="/etc/crontab /var/spool/cron /etc/init.d /etc/systemd/system"
for location in $persistence_locations; do
    if [ -e "$location" ]; then
        # Check for recently modified files
        find "$location" -type f -mmin -60 2>/dev/null | head -5 | while read file; do
            echo "PERSIST|$file|$(stat -c %Y "$file" 2>/dev/null)"
        done
    fi
done

# Check for autostart entries
autostart_dirs="/home/*/.config/autostart /etc/xdg/autostart"
for dir in $autostart_dirs; do
    if [ -d "$dir" ]; then
        find "$dir" -name "*.desktop" -mmin -60 2>/dev/null | while read file; do
            echo "PERSIST|AUTOSTART|$file"
        done
    fi
done

echo "=== PARENT_CHILD_RELATIONSHIPS ==="
# Show process tree for analysis
if command -v pstree &> /dev/null; then
    pstree -p | head -15 | while read line; do
        echo "PROC_TREE|$line"
    done
else
    # Fallback: show parent-child relationships
    ps -eo pid,ppid,cmd | head -15 | while read line; do
        echo "PROC_REL|$line"
    done
fi

echo "=== END_PROCESS_ACTIVITY ==="
