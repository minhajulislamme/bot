#!/bin/bash

LOG_DIR="logs"
mkdir -p $LOG_DIR

TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
LOG_FILE="$LOG_DIR/trading_bot_$TIMESTAMP.log"

echo "Starting Binance Trading Bot at $(date)"
echo "Logs will be saved to $LOG_FILE"

# Run the bot and log output
./trading_bot 2>&1 | tee -a "$LOG_FILE"

# Check if the bot exited with an error
if [ $? -ne 0 ]; then
    echo "Bot exited with an error. See log for details."
    exit 1
fi
