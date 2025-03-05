#ifndef CONFIG_H
#define CONFIG_H

#include <yaml-cpp/yaml.h>
#include <spdlog/spdlog.h>

class Config
{
public:
    // Trading Parameters
    static double MAX_POSITION_SIZE; // 1% of account
    static double MAX_DAILY_LOSS;    // 2% max daily loss
    static double RISK_PER_TRADE;    // 1% risk per trade
    static double MIN_CONFIDENCE;    // Minimum signal confidence

    // Technical Indicators
    static int RSI_PERIOD;
    static int EMA_PERIOD;
    static int MACD_FAST;
    static int MACD_SLOW;
    static int MACD_SIGNAL;

    // Risk Management
    static double STOP_LOSS_ATR_MULTIPLIER;
    static double TAKE_PROFIT_ATR_MULTIPLIER;

    // Multi-pair Trading Settings
    static int MAX_CONCURRENT_TRADES;
    static double PORTFOLIO_RISK;
    static double CORRELATION_THRESHOLD;

    // Per-Symbol Risk Adjustments
    struct SymbolRiskConfig
    {
        std::string symbol;
        double riskMultiplier;
    };

    static const std::vector<SymbolRiskConfig> SYMBOL_RISKS;

    static void loadFromFile(const std::string &configPath)
    {
        try
        {
            YAML::Node config = YAML::LoadFile(configPath);
            MAX_POSITION_SIZE = config["trading"]["max_position_size"].as<double>();
            MAX_DAILY_LOSS = config["trading"]["max_daily_loss"].as<double>();
            RISK_PER_TRADE = config["trading"]["risk_per_trade"].as<double>();
            MIN_CONFIDENCE = config["trading"]["min_confidence"].as<double>();

            // Load indicator settings
            RSI_PERIOD = config["indicators"]["rsi_period"].as<int>();
            EMA_PERIOD = config["indicators"]["ema_period"].as<int>();
            MACD_FAST = config["indicators"]["macd_fast"].as<int>();
            MACD_SLOW = config["indicators"]["macd_slow"].as<int>();
            MACD_SIGNAL = config["indicators"]["macd_signal"].as<int>();
        }
        catch (const YAML::Exception &e)
        {
            spdlog::error("Failed to load config: {}", e.what());
            throw;
        }
    }
};

#endif
