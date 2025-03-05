#ifndef TRADING_PAIRS_H
#define TRADING_PAIRS_H

#include <vector>
#include <string>

struct TradingPairConfig
{
    std::string symbol;
    double minQuantity;
    double maxQuantity;
    double tickSize;
    double minNotional;
    int leverage;
};

// List of trading pairs with their configurations
const std::vector<TradingPairConfig> TRADING_PAIRS = {
    {"BTCUSDT", 0.001, 100.0, 0.00001, 5.0, 20},
    {"ETHUSDT", 0.01, 1000.0, 0.00001, 5.0, 20},
    {"BNBUSDT", 0.01, 1000.0, 0.00001, 5.0, 20},
    {"DOGEUSDT", 1.0, 1000000.0, 0.00001, 5.0, 20},
    {"ADAUSDT", 1.0, 1000000.0, 0.00001, 5.0, 20}};

#endif
