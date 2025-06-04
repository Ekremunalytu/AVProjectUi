#!/bin/bash
# Setup Monitoring Script for Sandbox
# This script initializes monitoring infrastructure

MONITORING_LEVEL=1
NETWORK_MONITORING=false
FILESYSTEM_MONITORING=false
PROCESS_MONITORING=false
SYSCALL_MONITORING=false
SCREENSHOT_MONITORING=false
INTERVAL=5

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --level)
            MONITORING_LEVEL="$2"
            shift 2
            ;;
        --network)
            NETWORK_MONITORING=true
            shift
            ;;
        --filesystem)
            FILESYSTEM_MONITORING=true
            shift
            ;;
        --processes)
            PROCESS_MONITORING=true
            shift
            ;;
        --syscalls)
            SYSCALL_MONITORING=true
            shift
            ;;
        --screenshots)
            SCREENSHOT_MONITORING=true
            shift
            ;;
        --interval)
            INTERVAL="$2"
            shift 2
            ;;
        *)
            shift
            ;;
    esac
done

echo "Setting up sandbox monitoring with level: $MONITORING_LEVEL"

# Create monitoring directories
mkdir -p /sandbox/monitoring/network
mkdir -p /sandbox/monitoring/filesystem
mkdir -p /sandbox/monitoring/processes
mkdir -p /sandbox/monitoring/syscalls
mkdir -p /sandbox/logs

# Initialize monitoring state file
cat > /sandbox/monitoring/config << EOF
MONITORING_LEVEL=$MONITORING_LEVEL
NETWORK_MONITORING=$NETWORK_MONITORING
FILESYSTEM_MONITORING=$FILESYSTEM_MONITORING
PROCESS_MONITORING=$PROCESS_MONITORING
SYSCALL_MONITORING=$SYSCALL_MONITORING
SCREENSHOT_MONITORING=$SCREENSHOT_MONITORING
INTERVAL=$INTERVAL
START_TIME=$(date -u +"%Y-%m-%d %H:%M:%S")
EOF

# Setup network monitoring if enabled
if [ "$NETWORK_MONITORING" = true ]; then
    echo "Initializing network monitoring..."
    # Start background network monitoring
    (
        while true; do
            date >> /sandbox/monitoring/network/connections.log
            ss -tuln >> /sandbox/monitoring/network/connections.log 2>/dev/null
            netstat -i >> /sandbox/monitoring/network/interfaces.log 2>/dev/null
            sleep $INTERVAL
        done
    ) &
    echo $! > /sandbox/monitoring/network.pid
fi

# Setup filesystem monitoring if enabled
if [ "$FILESYSTEM_MONITORING" = true ]; then
    echo "Initializing filesystem monitoring..."
    if command -v inotifywait &> /dev/null; then
        (
            inotifywait -m -r -e create,modify,delete /tmp /var /home --format '%T %w%f %e' --timefmt '%Y-%m-%d %H:%M:%S' >> /sandbox/monitoring/filesystem/events.log 2>/dev/null
        ) &
        echo $! > /sandbox/monitoring/filesystem.pid
    fi
fi

# Setup process monitoring if enabled
if [ "$PROCESS_MONITORING" = true ]; then
    echo "Initializing process monitoring..."
    (
        while true; do
            date >> /sandbox/monitoring/processes/activity.log
            ps aux >> /sandbox/monitoring/processes/activity.log
            sleep $INTERVAL
        done
    ) &
    echo $! > /sandbox/monitoring/processes.pid
fi

# Setup system call monitoring if enabled
if [ "$SYSCALL_MONITORING" = true ]; then
    echo "Initializing system call monitoring..."
    # Note: This would require more complex setup in a real environment
    touch /sandbox/monitoring/syscalls/calls.log
fi

echo "Monitoring setup completed successfully"
echo "Monitoring level: $MONITORING_LEVEL"
echo "Interval: $INTERVAL seconds"
echo "Active monitors: Network=$NETWORK_MONITORING, FS=$FILESYSTEM_MONITORING, Proc=$PROCESS_MONITORING, Syscall=$SYSCALL_MONITORING"
