#ifndef TRADING_STRATEGY_H
#define TRADING_STRATEGY_H

#include <deque>
#include <vector>
#include <string>
#include "market_analyzer.h" // Add this include

struct TradingSignal
{
    enum Type
    {
        BUY,
        SELL,
        HOLD
    } type;
    double price;
    double stopLoss;
    double takeProfit;
    double confidence; // 0.0 to 1.0
};

class TradingStrategy
{
public:
    TradingStrategy(int periodRSI = 14, int periodEMA = 20, int periodMACD = 26);
    TradingSignal analyze(double currentPrice, double currentVolume);
    void addIndicatorData(double price, double volume);

    // New methods
    void updateMarketAnalysis(const MarketAnalyzer &analyzer);
    TradingSignal analyzeWithMarketContext(double currentPrice, double currentVolume);

private:
    std::deque<double> prices;
    std::deque<double> volumes;
    int rsiPeriod;
    int emaPeriod;
    int macdPeriod;

    double calculateRSI();
    double calculateEMA(int period);
    std::pair<double, double> calculateMACD();
    double calculateBollingerBands(bool upper);
    double calculateATR();

    // New members
    double marketSentiment;
    double trendStrength;
    bool isMarketUptrend;
    double supportLevel;
    double resistanceLevel;
};

#endif
