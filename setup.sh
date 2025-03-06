#!/bin/bash

set -e  # Exit immediately if a command exits with non-zero status

echo "==== Setting up Binance Trading Bot ===="
echo "Updating system packages..."
sudo apt-get update
sudo apt-get upgrade -y

echo "Installing required dependencies..."
sudo apt-get install -y build-essential git cmake \
    libcurl4-openssl-dev libssl-dev libwebsockets-dev \
    libjsoncpp-dev libspdlog-dev libyaml-cpp-dev libfmt-dev \
    pkg-config jq

echo "Creating log directory..."
mkdir -p logs

echo "Making scripts executable..."
chmod +x *.sh

echo "Building the project..."
make clean
make -j$(nproc)

echo "Testing SSL connection..."
./test_ssl.sh

# Test Telegram notifications if config file exists
if [ -f "config.yaml" ]; then
    echo "Testing Telegram notifications..."
    ./test_telegram.sh
else
    echo "Config file not found, skipping Telegram test."
    echo "Please create a config.yaml file with your Telegram token and chat_id."
fi

# Check for common configuration issues
echo "Checking configuration..."
if [ -f "config.yaml" ]; then
    TELEGRAM_TOKEN=$(grep "token:" config.yaml | cut -d'"' -f2)
    TELEGRAM_CHAT_ID=$(grep "chat_id:" config.yaml | cut -d'"' -f2)
    
    if [ -z "$TELEGRAM_TOKEN" ] || [ "$TELEGRAM_TOKEN" = "YOUR_TELEGRAM_TOKEN" ]; then
        echo "⚠️ Warning: Telegram token not properly configured in config.yaml"
    fi
    
    if [ -z "$TELEGRAM_CHAT_ID" ] || [ "$TELEGRAM_CHAT_ID" = "YOUR_CHAT_ID" ]; then
        echo "⚠️ Warning: Telegram chat ID not properly configured in config.yaml"
    fi
else
    echo "⚠️ Warning: config.yaml not found. Please create it before running the bot."
fi

echo ""
echo "Setup completed successfully!"
echo "To run the bot, use: ./run.sh"
echo "To install as a service, use: sudo ./install_service.sh"
echo "To test Telegram notifications, use: ./test_telegram.sh"
