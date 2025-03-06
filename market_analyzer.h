#ifndef MARKET_ANALYZER_H
#define MARKET_ANALYZER_H

#include <vector>
#include <string>
#include <map>

struct MarketData
{
    long timestamp;
    double open;
    double high;
    double low;
    double close;
    double volume;
};

class MarketAnalyzer
{
public:
    MarketAnalyzer();

    // Load and analyze historical data
    bool loadHistoricalData(const std::string &symbol, const std::string &interval = "1d", int limit = 30);

    // Get market statistics
    double getAverageVolume() const;
    double getAverageRange() const;
    double getVolatility() const;
    double getSupportLevel() const;
    double getResistanceLevel() const;

    // Market trend analysis
    bool isUptrend() const;
    bool isDowntrend() const;
    double getTrendStrength() const;

    // Market conditions
    bool isOverbought() const;
    bool isOversold() const;
    double getMarketSentiment() const; // -1 to 1 (bearish to bullish)

    const std::vector<MarketData> &getHistoricalData() const { return historicalData; }
    void updateCrossMarketCorrelations(const std::map<std::string, std::vector<MarketData>> &allData);
    double getPairCorrelation(const std::string &pair1, const std::string &pair2) const;
    bool isMarketOverextended() const;

private:
    std::vector<MarketData> historicalData;
    std::map<std::string, double> marketStats;
    std::map<std::pair<std::string, std::string>, double> crossMarketCorrelations;
    static constexpr double OVEREXTENDED_THRESHOLD = 2.0;

    void calculateStatistics();
    void identifyPatterns();
    void analyzeTrend();
    double calculateCorrelation(const std::vector<double> &x, const std::vector<double> &y) const;
};

#endif
