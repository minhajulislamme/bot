#!/bin/bash

# Test script for Telegram notifications

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
CONFIG_FILE="${DIR}/config.yaml"

# Extract Telegram token and chat ID from config
TELEGRAM_TOKEN=$(grep "token:" ${CONFIG_FILE} | cut -d'"' -f2)
TELEGRAM_CHAT_ID=$(grep "chat_id:" ${CONFIG_FILE} | cut -d'"' -f2)

echo "Testing Telegram notifications..."
echo "Token: ${TELEGRAM_TOKEN}"
echo "Chat ID: ${TELEGRAM_CHAT_ID}"

# Send a test message to verify configuration
MESSAGE="🧪 This is a test message from Binance Trading Bot at $(date)"
curl -s -X POST "https://api.telegram.org/bot${TELEGRAM_TOKEN}/sendMessage" \
     -d chat_id="${TELEGRAM_CHAT_ID}" \
     -d text="${MESSAGE}" > /dev/null

# Check if the API call was successful
if [ $? -eq 0 ]; then
    echo "✅ Test message sent successfully!"
else
    echo "❌ Failed to send test message"
    echo "Check your Telegram token and chat ID in config.yaml"
fi

echo "Done."
