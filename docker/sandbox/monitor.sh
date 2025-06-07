#!/bin/bash

# Sandbox Monitoring Script
# Monitors file execution and system behavior

LOGFILE="/sandbox/logs/execution.log"
PIDFILE="/sandbox/logs/monitor.pid"

# Function to log with timestamp
log_message() {
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] $1" >> "$LOGFILE"
}

# Function to start monitoring
start_monitoring() {
    echo $$ > "$PIDFILE"
    log_message "Sandbox monitoring started"
    
    # Monitor process creation
    log_message "Starting process monitoring..."
    
    # Monitor network activity
    log_message "Starting network monitoring..."
    
    # Monitor file system changes
    log_message "Starting file system monitoring..."
    
    # Keep monitoring running
    while [ -f "$PIDFILE" ]; do
        sleep 1
    done
    
    log_message "Sandbox monitoring stopped"
}

# Function to stop monitoring
stop_monitoring() {
    if [ -f "$PIDFILE" ]; then
        rm -f "$PIDFILE"
        log_message "Monitoring stop signal received"
    fi
}

# Function to execute file with monitoring
execute_file() {
    local FILE_PATH="$1"
    
    if [ ! -f "$FILE_PATH" ]; then
        log_message "ERROR: File not found: $FILE_PATH"
        return 1
    fi
    
    log_message "Executing file: $FILE_PATH"
    log_message "File size: $(stat -f%z "$FILE_PATH" 2>/dev/null || stat -c%s "$FILE_PATH")"
    log_message "File type: $(file "$FILE_PATH")"
    
    # Start monitoring in background
    start_monitoring &
    MONITOR_PID=$!
    
    # Execute the file with timeout and strace
    timeout 300s strace -f -e trace=all -o "/sandbox/logs/strace.log" "$FILE_PATH" >> "$LOGFILE" 2>&1
    EXEC_STATUS=$?
    
    # Stop monitoring
    stop_monitoring
    wait $MONITOR_PID
    
    log_message "File execution completed with status: $EXEC_STATUS"
    return $EXEC_STATUS
}

# Main script logic
case "$1" in
    "monitor")
        start_monitoring
        ;;
    "execute")
        execute_file "$2"
        ;;
    "stop")
        stop_monitoring
        ;;
    *)
        echo "Usage: $0 {monitor|execute <file>|stop}"
        exit 1
        ;;
esac
