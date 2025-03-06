#!/bin/bash

set -e  # Exit immediately if a command exits with non-zero status

echo "==== Server Setup for Binance Trading Bot ===="

# Ensure all scripts are executable
chmod +x *.sh

# Run detailed WebSocket tests
echo "Running WebSocket connectivity tests..."
./test_websocket.sh

# Check system requirements
echo -e "\nChecking system requirements..."
CPU_CORES=$(nproc)
TOTAL_MEM=$(free -m | awk '/^Mem:/{print $2}')
FREE_DISK=$(df -m / | awk 'NR==2 {print $4}')

echo "CPU Cores: $CPU_CORES"
echo "Total Memory: $TOTAL_MEM MB"
echo "Free Disk Space: $FREE_DISK MB"

if [ $CPU_CORES -lt 2 ]; then
    echo "⚠️ Warning: Low CPU core count. The bot may run slower than expected."
fi
if [ $TOTAL_MEM -lt 2000 ]; then
    echo "⚠️ Warning: Low memory. At least 2GB RAM is recommended."
fi
if [ $FREE_DISK -lt 5000 ]; then
    echo "⚠️ Warning: Low disk space. At least 5GB free space is recommended."
fi

# Check for required packages
echo -e "\nChecking for required system packages..."
MISSING_PKGS=""
for pkg in build-essential git cmake libcurl4-openssl-dev libssl-dev libwebsockets-dev libjsoncpp-dev libspdlog-dev libyaml-cpp-dev libfmt-dev pkg-config jq netcat-openbsd; do
    if ! dpkg -s $pkg >/dev/null 2>&1; then
        MISSING_PKGS="$MISSING_PKGS $pkg"
    fi
done

if [ -n "$MISSING_PKGS" ]; then
    echo "Installing missing packages:$MISSING_PKGS"
    apt-get update
    apt-get install -y $MISSING_PKGS
fi

# Check network connectivity
echo -e "\nVerifying network connectivity..."
echo "Testing connectivity to Binance APIs..."
curl -s "https://testnet.binancefuture.com/fapi/v1/ping" > /dev/null
if [ $? -eq 0 ]; then
    echo "✅ Binance API is accessible"
else
    echo "❌ Cannot access Binance API. Check your network configuration."
    exit 1
fi

# Setup firewall rules if needed
echo -e "\nEnsuring firewall allows necessary connections..."
if command -v ufw >/dev/null 2>&1; then
    ufw status | grep -q "Status: active"
    if [ $? -eq 0 ]; then
        echo "Firewall is active, ensuring outbound connections are allowed..."
        ufw allow out 443/tcp
    fi
fi

echo -e "\nServer setup complete! You can now run:"
echo "  - './run.sh' to start the bot manually"
echo "  - './install_service.sh' to install as a system service"
echo "  - './check_system_health.sh' to verify everything is working properly"
