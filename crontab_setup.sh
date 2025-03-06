#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

# Make sure all scripts are executable
chmod +x ${DIR}/*.sh

# Setup crontab entries for monitoring
(crontab -l 2>/dev/null; echo "*/10 * * * * ${DIR}/monitor.sh >> ${DIR}/logs/monitor.log 2>&1") | crontab -
(crontab -l 2>/dev/null; echo "0 0 * * * find ${DIR}/logs -name \"*.log\" -type f -mtime +7 -exec rm {} \;") | crontab -

echo "Crontab setup complete:"
echo "- Monitoring every 10 minutes"
echo "- Log cleanup every day (removing logs older than 7 days)"
crontab -l
