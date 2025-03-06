#ifndef RISK_MANAGER_H
#define RISK_MANAGER_H

#include <string>
#include <map>
#include <vector>

class RiskManager
{
public:
    RiskManager(double maxPositionSize = 0.01,
                double maxDailyLoss = 0.02,
                double riskPerTrade = 0.01);

    struct Position
    {
        std::string symbol;
        std::string side;
        double entryPrice;
        double quantity;
        double stopLoss;
        double takeProfit;
    };

    bool canTrade(const std::string &symbol, double price);
    double getPositionSize(double price, double stopLoss);
    void updatePosition(const Position &position);
    void closePosition(const std::string &symbol);
    bool shouldClosePosition(const std::string &symbol, double currentPrice);
    double getDailyPnL() const { return dailyPnL; }

    // Add getter for position quantity
    double getPositionQuantity(const std::string &symbol) const
    {
        auto it = positions.find(symbol);
        return it != positions.end() ? it->second.quantity : 0.0;
    }

    // New methods for multi-pair trading
    double getTotalExposure() const;
    int getActivePositionsCount() const;
    bool isHighlyCorrelated(const std::string &symbol1, const std::string &symbol2) const;
    double getSymbolRiskMultiplier(const std::string &symbol) const;

private:
    double accountBalance;
    double maxPositionSize;
    double maxDailyLoss;
    double riskPerTrade;
    double dailyPnL;
    std::map<std::string, Position> positions;
    std::map<std::string, std::vector<double>> priceHistory;
    std::map<std::pair<std::string, std::string>, double> correlationMatrix;

    static constexpr int MAX_POSITIONS = 5;
    static constexpr double MAX_TOTAL_RISK = 0.15; // 15% max total risk
    static constexpr double MAX_CORRELATION = 0.7;

    void updateCorrelationMatrix();
    double calculatePairCorrelation(const std::string &symbol1, const std::string &symbol2) const;
    bool checkPositionLimits(const std::string &symbol) const;
    double calculateCorrelation(const std::vector<double> &prices1,
                                const std::vector<double> &prices2) const;
};

#endif
