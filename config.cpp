#include "config.h"

// Initialize static members with default values
double Config::MAX_POSITION_SIZE = 0.01;
double Config::MAX_DAILY_LOSS = 0.02;
double Config::RISK_PER_TRADE = 0.01;
double Config::MIN_CONFIDENCE = 0.7;

int Config::RSI_PERIOD = 14;
int Config::EMA_PERIOD = 20;
int Config::MACD_FAST = 12;
int Config::MACD_SLOW = 26;
int Config::MACD_SIGNAL = 9;

double Config::STOP_LOSS_ATR_MULTIPLIER = 2.0;
double Config::TAKE_PROFIT_ATR_MULTIPLIER = 3.0;

int Config::MAX_CONCURRENT_TRADES = 5;
double Config::PORTFOLIO_RISK = 0.05;
double Config::CORRELATION_THRESHOLD = 0.7;

const std::vector<Config::SymbolRiskConfig> Config::SYMBOL_RISKS = {
    {"BTCUSDT", 1.0},
    {"ETHUSDT", 0.8},
    {"BNBUSDT", 0.7},
    {"DOGEUSDT", 0.5},
    {"ADAUSDT", 0.5}};
