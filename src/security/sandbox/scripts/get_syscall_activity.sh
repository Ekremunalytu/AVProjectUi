#!/bin/bash
# System Call Activity Monitoring Script for Sandbox
# This script monitors system calls and security-relevant activities

echo "=== SYSTEM_CALLS ==="
# Monitor system calls using strace (if available)
if command -v strace &> /dev/null; then
    echo "SYSCALL|strace monitoring available"
    # Get recent processes and their system call patterns
    ps aux | grep -v "grep\|ps\|strace" | tail -10 | while read line; do
        pid=$(echo "$line" | awk '{print $2}')
        process=$(echo "$line" | awk '{print $11}')
        echo "SYSCALL|PID:$pid|PROCESS:$process"
    done
fi

echo "=== PRIVILEGE_ESCALATION ==="
# Monitor for privilege escalation attempts
recent_sudo=$(last -n 10 | grep sudo 2>/dev/null | head -5)
if [ ! -z "$recent_sudo" ]; then
    echo "$recent_sudo" | while read line; do
        echo "PRIV_ESC|SUDO|$line"
    done
fi

# Check for setuid/setgid file executions
find /tmp -perm -4000 -o -perm -2000 2>/dev/null | head -5 | while read file; do
    echo "PRIV_ESC|SETUID|$file"
done

echo "=== ANTI_DEBUGGING ==="
# Check for anti-debugging techniques
if command -v lsof &> /dev/null; then
    # Look for processes that might be checking for debuggers
    lsof | grep -i "debug\|trace\|gdb" | head -5 | while read line; do
        echo "ANTI_DEBUG|$line"
    done
fi

echo "=== CODE_INJECTION ==="
# Monitor for potential code injection
proc_maps="/proc/*/maps"
for maps_file in $proc_maps; do
    if [ -r "$maps_file" ] && [ -f "$maps_file" ]; then
        pid=$(echo "$maps_file" | sed 's/.*\/proc\/\([0-9]*\)\/.*/\1/')
        # Look for executable memory regions that might indicate injection
        grep -i "rwx\|heap.*x\|stack.*x" "$maps_file" 2>/dev/null | head -2 | while read line; do
            echo "CODE_INJ|PID:$pid|$line"
        done
    fi
done

echo "=== END_SYSCALL_ACTIVITY ==="
