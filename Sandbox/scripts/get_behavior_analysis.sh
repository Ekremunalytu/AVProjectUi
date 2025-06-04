#!/bin/bash
# Behavior Analysis Script for Sandbox
# This script collects and analyzes behavioral patterns

echo "=== BEHAVIOR_ANALYSIS ==="

# Monitor recently executed commands
echo "=== COMMAND_HISTORY ==="
if [ -f /root/.bash_history ]; then
    tail -n 20 /root/.bash_history | while read cmd; do
        echo "BEHAVIOR|command_executed|$cmd"
    done
fi

# Check for suspicious file operations
echo "=== SUSPICIOUS_FILE_OPS ==="
find /tmp -type f -executable -newer /sandbox/logs/init.log 2>/dev/null | while read file; do
    echo "BEHAVIOR|suspicious_executable_created|$file"
done

# Check for network activities
echo "=== NETWORK_BEHAVIORS ==="
if command -v ss &> /dev/null; then
    ss -tuln | grep LISTEN | while read line; do
        echo "BEHAVIOR|listening_service|$line"
    done
fi

# Check for privilege escalation attempts
echo "=== PRIVILEGE_ESCALATION ==="
grep -i "sudo\|su\|setuid" /var/log/syslog 2>/dev/null | tail -5 | while read line; do
    echo "BEHAVIOR|privilege_escalation_attempt|$line"
done

# Check for process injection patterns
echo "=== PROCESS_INJECTION ==="
for pid in $(ps -eo pid --no-headers | head -10); do
    if [ -r "/proc/$pid/maps" ]; then
        injection_count=$(grep -c "rwx" "/proc/$pid/maps" 2>/dev/null)
        if [ "$injection_count" -gt 0 ]; then
            proc_name=$(ps -p "$pid" -o comm= 2>/dev/null)
            echo "BEHAVIOR|potential_process_injection|PID:$pid|NAME:$proc_name|RWX_REGIONS:$injection_count"
        fi
    fi
done

# Check for anti-analysis techniques
echo "=== ANTI_ANALYSIS ==="
if pgrep -f "gdb\|strace\|ltrace" &> /dev/null; then
    echo "BEHAVIOR|debugger_detection|Debugging tools detected in process list"
fi

# Check for persistence mechanisms
echo "=== PERSISTENCE_CHECKS ==="
if [ -d "/etc/cron.d" ]; then
    find /etc/cron.d -newer /sandbox/logs/init.log 2>/dev/null | while read cronfile; do
        echo "BEHAVIOR|cron_persistence|$cronfile"
    done
fi

echo "=== END_BEHAVIOR_ANALYSIS ==="
