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
    pkg-config

echo "Creating log directory..."
mkdir -p logs

echo "Building the project..."
make clean
make -j$(nproc)

echo "Testing SSL connection..."
./test_ssl.sh

echo "Setup completed successfully!"
echo "To run the bot, use: ./run.sh"
echo "To install as a service, use: sudo ./install_service.sh"
