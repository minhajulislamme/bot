#ifndef CONFIG_H
#define CONFIG_H

// Trading Parameters
constexpr double MAX_POSITION_SIZE = 0.01; // 1% of account
constexpr double MAX_DAILY_LOSS = 0.02;    // 2% max daily loss
constexpr double RISK_PER_TRADE = 0.01;    // 1% risk per trade
constexpr double MIN_CONFIDENCE = 0.7;     // Minimum signal confidence

// Technical Indicators
constexpr int RSI_PERIOD = 14;
constexpr int EMA_PERIOD = 20;
constexpr int MACD_FAST = 12;
constexpr int MACD_SLOW = 26;
constexpr int MACD_SIGNAL = 9;

// Risk Management
constexpr double STOP_LOSS_ATR_MULTIPLIER = 2.0;
constexpr double TAKE_PROFIT_ATR_MULTIPLIER = 3.0;

// Multi-pair Trading Settings
constexpr int MAX_CONCURRENT_TRADES = 5;      // Maximum number of simultaneous positions
constexpr double PORTFOLIO_RISK = 0.05;       // Maximum 5% portfolio risk across all positions
constexpr double CORRELATION_THRESHOLD = 0.7; // Avoid trading highly correlated pairs

// Per-Symbol Risk Adjustments
struct SymbolRiskConfig
{
    std::string symbol;
    double riskMultiplier; // Adjust risk per symbol (1.0 is standard)
};

const SymbolRiskConfig SYMBOL_RISKS[] = {
    {"BTCUSDT", 1.0},
    {"ETHUSDT", 0.8},
    {"BNBUSDT", 0.7},
    {"DOGEUSDT", 0.5},
    {"ADAUSDT", 0.5}};

#endif
