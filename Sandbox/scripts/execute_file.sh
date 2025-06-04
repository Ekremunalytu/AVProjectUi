#!/bin/bash
# Execute File Script for Sandbox
# This script safely executes files in the sandbox environment

TARGET_FILE="$1"
MONITOR_NETWORK=false
MONITOR_FILESYSTEM=false
MONITOR_PROCESSES=false
RECORD_SCREENSHOTS=false

# Parse additional arguments
shift
while [[ $# -gt 0 ]]; do
    case $1 in
        --monitor-network)
            MONITOR_NETWORK=true
            shift
            ;;
        --monitor-filesystem)
            MONITOR_FILESYSTEM=true
            shift
            ;;
        --monitor-processes)
            MONITOR_PROCESSES=true
            shift
            ;;
        --record-screenshots)
            RECORD_SCREENSHOTS=true
            shift
            ;;
        *)
            shift
            ;;
    esac
done

if [ -z "$TARGET_FILE" ]; then
    echo "ERROR: No target file specified"
    exit 1
fi

if [ ! -f "$TARGET_FILE" ]; then
    echo "ERROR: Target file does not exist: $TARGET_FILE"
    exit 1
fi

echo "Starting execution of: $TARGET_FILE"
echo "Monitoring - Network: $MONITOR_NETWORK, FS: $MONITOR_FILESYSTEM, Processes: $MONITOR_PROCESSES"

# Create execution log
EXEC_LOG="/sandbox/logs/execution_$(date +%s).log"
echo "Execution started at: $(date)" > "$EXEC_LOG"
echo "Target file: $TARGET_FILE" >> "$EXEC_LOG"

# Start pre-execution monitoring snapshot
if [ "$MONITOR_PROCESSES" = true ]; then
    echo "=== PRE_EXECUTION_PROCESSES ===" >> "$EXEC_LOG"
    ps aux >> "$EXEC_LOG"
fi

if [ "$MONITOR_NETWORK" = true ]; then
    echo "=== PRE_EXECUTION_NETWORK ===" >> "$EXEC_LOG"
    ss -tuln >> "$EXEC_LOG" 2>/dev/null
    netstat -rn >> "$EXEC_LOG" 2>/dev/null
fi

# Determine file type and execution method
FILE_TYPE=$(file "$TARGET_FILE" | cut -d: -f2)
echo "File type detected: $FILE_TYPE" >> "$EXEC_LOG"

# Execute based on file type
if [[ "$FILE_TYPE" == *"shell script"* ]]; then
    echo "Executing as shell script..." >> "$EXEC_LOG"
    timeout 60 bash "$TARGET_FILE" >> "$EXEC_LOG" 2>&1
    EXEC_RESULT=$?
elif [[ "$FILE_TYPE" == *"Python"* ]]; then
    echo "Executing as Python script..." >> "$EXEC_LOG"
    timeout 60 python3 "$TARGET_FILE" >> "$EXEC_LOG" 2>&1
    EXEC_RESULT=$?
elif [[ "$FILE_TYPE" == *"ELF"* ]]; then
    echo "Executing as Linux binary..." >> "$EXEC_LOG"
    chmod +x "$TARGET_FILE"
    timeout 60 "$TARGET_FILE" >> "$EXEC_LOG" 2>&1
    EXEC_RESULT=$?
elif [[ "$FILE_TYPE" == *"JavaScript"* ]]; then
    echo "Executing as JavaScript..." >> "$EXEC_LOG"
    timeout 60 node "$TARGET_FILE" >> "$EXEC_LOG" 2>&1
    EXEC_RESULT=$?
else
    echo "Unknown file type, attempting direct execution..." >> "$EXEC_LOG"
    chmod +x "$TARGET_FILE"
    timeout 60 "$TARGET_FILE" >> "$EXEC_LOG" 2>&1
    EXEC_RESULT=$?
fi

echo "Execution completed with exit code: $EXEC_RESULT" >> "$EXEC_LOG"

# Post-execution monitoring snapshot
if [ "$MONITOR_PROCESSES" = true ]; then
    echo "=== POST_EXECUTION_PROCESSES ===" >> "$EXEC_LOG"
    ps aux >> "$EXEC_LOG"
fi

if [ "$MONITOR_NETWORK" = true ]; then
    echo "=== POST_EXECUTION_NETWORK ===" >> "$EXEC_LOG"
    ss -tuln >> "$EXEC_LOG" 2>/dev/null
fi

if [ "$MONITOR_FILESYSTEM" = true ]; then
    echo "=== FILESYSTEM_CHANGES ===" >> "$EXEC_LOG"
    find /tmp -newer "$EXEC_LOG" -type f 2>/dev/null | head -20 >> "$EXEC_LOG"
fi

echo "Execution completed at: $(date)" >> "$EXEC_LOG"
echo "Execution completed successfully. Log: $EXEC_LOG"

exit $EXEC_RESULT
