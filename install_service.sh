#!/bin/bash

set -e  # Exit immediately if a command exits with non-zero status

# Get the current directory
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
USER=$(whoami)

echo "Creating systemd service file..."
cat > /tmp/binance-bot.service << EOL
[Unit]
Description=Binance Trading Bot
After=network.target

[Service]
Type=simple
User=${USER}
WorkingDirectory=${DIR}
ExecStart=${DIR}/run.sh
Restart=always
RestartSec=30
StandardOutput=append:${DIR}/logs/service.log
StandardError=append:${DIR}/logs/error.log

# Security measures
PrivateTmp=true
NoNewPrivileges=true
ProtectSystem=full
ProtectHome=read-only

[Install]
WantedBy=multi-user.target
EOL

echo "Installing service..."
sudo mv /tmp/binance-bot.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable binance-bot.service

echo "Starting service..."
sudo systemctl start binance-bot.service
sleep 2
sudo systemctl status binance-bot.service

echo "Service installation complete!"
echo "You can check service status with: sudo systemctl status binance-bot.service"
echo "View logs with: journalctl -u binance-bot.service"
