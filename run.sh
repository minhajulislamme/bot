#!/bin/bash

LOG_DIR="logs"
mkdir -p $LOG_DIR

TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
LOG_FILE="$LOG_DIR/trading_bot_$TIMESTAMP.log"

echo "Starting Binance Trading Bot at $(date)"
echo "Logs will be saved to $LOG_FILE"

# Check if we have proper Telegram configuration before starting
if [ -f "config.yaml" ]; then
    TELEGRAM_TOKEN=$(grep "token:" config.yaml | cut -d'"' -f2)
    TELEGRAM_CHAT_ID=$(grep "chat_id:" config.yaml | cut -d'"' -f2)
    
    if [ -n "$TELEGRAM_TOKEN" ] && [ -n "$TELEGRAM_CHAT_ID" ] && \
       [ "$TELEGRAM_TOKEN" != "YOUR_TELEGRAM_TOKEN" ] && [ "$TELEGRAM_CHAT_ID" != "YOUR_CHAT_ID" ]; then
        echo "Telegram notifications configured correctly."
        
        # Send a startup notification via curl directly
        MESSAGE="🚀 Binance Trading Bot starting up at $(date)"
        curl -s -X POST "https://api.telegram.org/bot${TELEGRAM_TOKEN}/sendMessage" \
             -d chat_id="${TELEGRAM_CHAT_ID}" \
             -d text="${MESSAGE}" > /dev/null
    else
        echo "Warning: Telegram notifications not properly configured."
        echo "Check your token and chat_id in config.yaml."
    fi
else
    echo "Warning: config.yaml not found. Telegram notifications will not work."
fi

# Run the bot and log output
./trading_bot 2>&1 | tee -a "$LOG_FILE"

# Check if the bot exited with an error
if [ $? -ne 0 ]; then
    echo "Bot exited with an error. See log for details."
    
    # Send error notification via Telegram if configured
    if [ -n "$TELEGRAM_TOKEN" ] && [ -n "$TELEGRAM_CHAT_ID" ]; then
        ERROR_MESSAGE="⚠️ Trading bot exited with an error at $(date)"
        curl -s -X POST "https://api.telegram.org/bot${TELEGRAM_TOKEN}/sendMessage" \
             -d chat_id="${TELEGRAM_CHAT_ID}" \
             -d text="${ERROR_MESSAGE}" > /dev/null
    fi
    
    exit 1
fi
