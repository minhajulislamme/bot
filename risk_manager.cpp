#include "risk_manager.h"
#include <cmath>

RiskManager::RiskManager(double maxPositionSize, double maxDailyLoss, double riskPerTrade)
    : accountBalance(10000.0), // Default initial balance
      maxPositionSize(maxPositionSize),
      maxDailyLoss(maxDailyLoss),
      riskPerTrade(riskPerTrade),
      dailyPnL(0.0)
{
}

bool RiskManager::canTrade([[maybe_unused]] const std::string &symbol, double price)
{
    // Check if we've hit daily loss limit
    if (dailyPnL < -(accountBalance * maxDailyLoss))
    {
        return false;
    }

    // Check if we already have too many positions
    double totalExposure = 0.0;
    for (const auto &pos : positions)
    {
        totalExposure += pos.second.quantity * price;
    }

    return (totalExposure / accountBalance) < maxPositionSize;
}

double RiskManager::getPositionSize(double price, double stopLoss)
{
    double riskAmount = accountBalance * riskPerTrade;
    double riskPerUnit = std::abs(price - stopLoss);
    if (riskPerUnit <= 0)
        return 0.0;

    double size = riskAmount / riskPerUnit;
    return std::min(size, accountBalance * maxPositionSize / price);
}

void RiskManager::updatePosition(const Position &position)
{
    positions[position.symbol] = position;
}

void RiskManager::closePosition(const std::string &symbol)
{
    auto it = positions.find(symbol);
    if (it != positions.end())
    {
        // Update daily PnL here if needed
        positions.erase(it);
    }
}

bool RiskManager::shouldClosePosition(const std::string &symbol, double currentPrice)
{
    auto it = positions.find(symbol);
    if (it == positions.end())
        return false;

    const Position &pos = it->second;
    if (pos.side == "BUY")
    {
        return currentPrice <= pos.stopLoss || currentPrice >= pos.takeProfit;
    }
    else
    {
        return currentPrice >= pos.stopLoss || currentPrice <= pos.takeProfit;
    }
}
