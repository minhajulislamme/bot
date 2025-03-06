#!/bin/bash

echo "=== Compiling System Test ==="
g++ -Wall -Wextra -I. -I/usr/include/openssl -I/usr/include/jsoncpp -I/usr/include/spdlog -I/usr/include/yaml-cpp \
    system_test.cpp \
    binance_utils.cpp \
    websocket_client.cpp \
    market_analyzer.cpp \
    trading_strategy.cpp \
    risk_manager.cpp \
    order_manager.cpp \
    config.cpp \
    telegram_notifier.cpp \
    data_health_monitor.cpp \
    status_checker.cpp \
    -o system_test \
    -lcurl -lwebsockets -lssl -lcrypto -lpthread -ljsoncpp -lspdlog -lyaml-cpp -lfmt

if [ $? -ne 0 ]; then
    echo "Compilation failed."
    exit 1
fi

echo ""
echo "=== Running System Test ==="
./system_test

echo ""
echo "System test completed. See logs/system_test.log for details."
