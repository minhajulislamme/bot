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

# Make the testing scripts executable
echo "Setting up test scripts..."
chmod +x run_system_test.sh
chmod +x check_system_health.sh

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

# Run a quick health check to verify API connectivity
echo "Verifying API connectivity..."
if [ -f "check_system_health.sh" ]; then
    ./check_system_health.sh
fi

echo ""
echo "Setup completed successfully!"
echo "To run the bot, use: ./run.sh"
echo "To install as a service, use: sudo ./install_service.sh"
echo "To test Telegram notifications, use: ./test_telegram.sh"
echo "To run complete system tests, use: ./run_system_test.sh"
echo "To run a quick system health check, use: ./check_system_health.sh"

# Create a post-installation instruction file
cat > POST_INSTALL_STEPS.txt << EOL
=== Binance Trading Bot - Post Installation Steps ===

1. Verify your configuration:
   - Check config.yaml has correct Telegram token and chat_id
   - Ensure API keys are properly set for your Binance testnet account

2. Test the system:
   - Run './check_system_health.sh' to verify API connectivity
   - Run './run_system_test.sh' to run comprehensive tests

3. Monitor the bot:
   - View logs in the 'logs/' directory
   - Check Telegram for notifications
   
4. Start the bot:
   - Run './run.sh' for manual execution
   - Run 'sudo ./install_service.sh' to install as a system service

5. For troubleshooting:
   - Check the logs directory
   - Run the system tests again with './run_system_test.sh'
   - Verify Binance API connectivity with './check_system_health.sh'
EOL

echo "Post-installation instructions have been saved to POST_INSTALL_STEPS.txt"
