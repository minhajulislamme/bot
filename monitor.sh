#!/bin/bash

# Configuration
TELEGRAM_TOKEN=$(grep "token:" config.yaml | cut -d'"' -f2)
TELEGRAM_CHAT_ID=$(grep "chat_id:" config.yaml | cut -d'"' -f2)

# Check if the process is running
check_process() {
    if ! pgrep -f "./trading_bot" > /dev/null; then
        send_notification "⚠️ Alert: Trading bot is not running! Attempting to restart..."
        sudo systemctl restart binance-bot.service
        sleep 10
        
        if ! pgrep -f "./trading_bot" > /dev/null; then
            send_notification "🚨 Critical: Failed to restart trading bot. Manual intervention required."
        else
            send_notification "✅ Trading bot successfully restarted."
        fi
    fi
}

# Send Telegram notification
send_notification() {
    local message="$1"
    curl -s -X POST "https://api.telegram.org/bot${TELEGRAM_TOKEN}/sendMessage" \
        -d chat_id="${TELEGRAM_CHAT_ID}" \
        -d text="${message}" > /dev/null
}

# Check system resources
check_resources() {
    local cpu_usage=$(top -bn1 | grep "Cpu(s)" | sed "s/.*, *\([0-9.]*\)%* id.*/\1/" | awk '{print 100 - $1}')
    local memory_usage=$(free -m | awk '/Mem:/ {print int($3/$2 * 100)}')
    local disk_usage=$(df -h / | awk 'NR==2 {print $5}' | tr -d '%')
    
    if (( $(echo "$cpu_usage > 90" | bc -l) )); then
        send_notification "⚠️ High CPU usage: ${cpu_usage}%"
    fi
    
    if [ "$memory_usage" -gt 90 ]; then
        send_notification "⚠️ High memory usage: ${memory_usage}%"
    fi
    
    if [ "$disk_usage" -gt 90 ]; then
        send_notification "⚠️ High disk usage: ${disk_usage}%"
    fi
}

# Main monitoring function
monitor() {
    check_process
    check_resources
    
    # Check log for errors
    if [ -f logs/trading_bot.log ]; then
        error_count=$(grep -c "ERROR" logs/trading_bot.log)
        if [ "$error_count" -gt 0 ]; then
            recent_errors=$(grep "ERROR" logs/trading_bot.log | tail -5)
            send_notification "⚠️ ${error_count} errors found in log:\n${recent_errors}"
            # Rotate logs when errors found
            mv logs/trading_bot.log logs/trading_bot_$(date +"%Y%m%d_%H%M%S").log
        fi
    fi
}

# Run monitoring
monitor
