#!/bin/bash

# A simplified script to quickly check if the bot is working properly
# This runs a subset of tests from the full system test to verify core functionality

echo "=== Quick System Health Check ==="
mkdir -p logs

# Check if trading_bot process is running
if pgrep -f "trading_bot" > /dev/null; then
    echo "✅ Trading bot process is running"
else
    echo "❌ Trading bot process is NOT running!"
fi

# Check for recent log entries
LATEST_LOG=$(find logs -name "*.log" -type f -printf '%T@ %p\n' | sort -n | tail -1 | cut -d' ' -f2-)
echo "Latest log file: $LATEST_LOG"

if [ -f "$LATEST_LOG" ]; then
    LAST_LOG_TIME=$(date -r "$LATEST_LOG" "+%Y-%m-%d %H:%M:%S")
    echo "Last log update: $LAST_LOG_TIME"
    
    # Check recent errors
    ERROR_COUNT=$(grep -c "ERROR" "$LATEST_LOG")
    if [ $ERROR_COUNT -gt 0 ]; then
        echo "⚠️ Found $ERROR_COUNT errors in log"
        echo "Recent errors:"
        grep "ERROR" "$LATEST_LOG" | tail -5
    else
        echo "✅ No errors found in log"
    fi
else
    echo "❌ No log files found!"
fi

# Check Binance API connectivity
echo -e "\nTesting Binance API connection..."
curl -s "https://testnet.binancefuture.com/fapi/v1/ping" > /dev/null
if [ $? -eq 0 ]; then
    echo "✅ Binance API is accessible"
else
    echo "❌ Cannot access Binance API!"
fi

# Check WebSocket connectivity
echo -e "\nTesting WebSocket server..."
timeout 5 bash -c "curl -s --include --no-buffer -N \
    -H 'Connection: Upgrade' \
    -H 'Upgrade: websocket' \
    -H 'Host: stream.binancefuture.com' \
    -H 'Origin: https://stream.binancefuture.com' \
    -H 'Sec-WebSocket-Key: SGVsbG8sIHdvcmxkIQ==' \
    -H 'Sec-WebSocket-Version: 13' \
    'wss://stream.binancefuture.com/ws' 2>&1 | grep -q 'HTTP/1.1 101'"

if [ $? -eq 0 ]; then
    echo "✅ WebSocket server is accessible"
else
    echo "❌ Cannot access WebSocket server!"
fi

echo -e "\nSystem health check completed."
