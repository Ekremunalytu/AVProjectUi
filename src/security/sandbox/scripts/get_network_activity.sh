#!/bin/bash
# Network Activity Monitoring Script for Sandbox
# This script monitors network connections and DNS queries

echo "=== NETWORK_CONNECTIONS ==="
# Monitor active network connections
netstat -tulpn 2>/dev/null | grep ESTABLISHED | while read line; do
    echo "CONN|$line"
done

echo "=== DNS_QUERIES ==="
# Monitor DNS queries from system logs (simplified)
if [ -f /var/log/syslog ]; then
    tail -n 100 /var/log/syslog | grep -i dns | tail -n 10 | while read line; do
        echo "DNS|$line"
    done
fi

echo "=== NETWORK_TRAFFIC ==="
# Monitor network interface statistics
if command -v ss &> /dev/null; then
    ss -tuln | while read line; do
        echo "TRAFFIC|$line"
    done
fi

echo "=== END_NETWORK_ACTIVITY ==="
