#include "trading_strategy.h"
#include <numeric>
#include <cmath>

TradingStrategy::TradingStrategy(int periodRSI, int periodEMA, int periodMACD)
    : rsiPeriod(periodRSI), emaPeriod(periodEMA), macdPeriod(periodMACD) {}

void TradingStrategy::addIndicatorData(double price, double volume)
{
    prices.push_back(price);
    volumes.push_back(volume);

    // Keep only necessary history
    const size_t maxSize = static_cast<size_t>(macdPeriod * 2);
    while (prices.size() > maxSize)
    {
        prices.pop_front();
        volumes.pop_front();
    }
}

TradingSignal TradingStrategy::analyze(double currentPrice, [[maybe_unused]] double currentVolume)
{
    TradingSignal signal{TradingSignal::HOLD, currentPrice, 0, 0, 0};

    const size_t minSize = static_cast<size_t>(macdPeriod);
    if (prices.size() < minSize)
        return signal;

    double rsi = calculateRSI();
    double ema = calculateEMA(emaPeriod);
    auto [macd, signal_line] = calculateMACD();
    double atr = calculateATR();

    // Generate trading signals based on indicators
    if (rsi < 30 && macd > signal_line && currentPrice > ema)
    {
        signal.type = TradingSignal::BUY;
        signal.stopLoss = currentPrice - (2 * atr);
        signal.takeProfit = currentPrice + (3 * atr);
        signal.confidence = (30 - rsi) / 30.0;
    }
    else if (rsi > 70 && macd < signal_line && currentPrice < ema)
    {
        signal.type = TradingSignal::SELL;
        signal.stopLoss = currentPrice + (2 * atr);
        signal.takeProfit = currentPrice - (3 * atr);
        signal.confidence = (rsi - 70) / 30.0;
    }

    return signal;
}

void TradingStrategy::updateMarketAnalysis(const MarketAnalyzer &analyzer)
{
    marketSentiment = analyzer.getMarketSentiment();
    trendStrength = analyzer.getTrendStrength();
    isMarketUptrend = analyzer.isUptrend();
    supportLevel = analyzer.getSupportLevel();
    resistanceLevel = analyzer.getResistanceLevel();
}

TradingSignal TradingStrategy::analyzeWithMarketContext(double currentPrice, double currentVolume)
{
    TradingSignal signal = analyze(currentPrice, currentVolume);

    // Adjust signal based on market context
    if (signal.type != TradingSignal::HOLD)
    {
        // Reduce confidence in counter-trend trades
        if ((signal.type == TradingSignal::BUY && !isMarketUptrend) ||
            (signal.type == TradingSignal::SELL && isMarketUptrend))
        {
            signal.confidence *= (1.0 - trendStrength);
        }

        // Adjust stop loss and take profit based on support/resistance
        if (signal.type == TradingSignal::BUY)
        {
            signal.stopLoss = std::max(signal.stopLoss, supportLevel);
            signal.takeProfit = std::min(signal.takeProfit, resistanceLevel);
        }
        else
        {
            signal.stopLoss = std::min(signal.stopLoss, resistanceLevel);
            signal.takeProfit = std::max(signal.takeProfit, supportLevel);
        }
    }

    return signal;
}

double TradingStrategy::calculateRSI()
{
    const size_t minSize = static_cast<size_t>(rsiPeriod + 1);
    if (prices.size() < minSize)
        return 50.0;

    double gain = 0.0, loss = 0.0;
    for (size_t i = 1; i < prices.size(); ++i)
    {
        double diff = prices[i] - prices[i - 1];
        if (diff >= 0)
            gain += diff;
        else
            loss -= diff;
    }

    if (loss == 0)
        return 100.0;
    double rs = gain / loss;
    return 100.0 - (100.0 / (1.0 + rs));
}

double TradingStrategy::calculateEMA(int period)
{
    const size_t minSize = static_cast<size_t>(period);
    if (prices.size() < minSize)
        return prices.back();

    double multiplier = 2.0 / (period + 1.0);
    double ema = prices[prices.size() - period];

    for (size_t i = prices.size() - period + 1; i < prices.size(); ++i)
    {
        ema = (prices[i] - ema) * multiplier + ema;
    }

    return ema;
}

std::pair<double, double> TradingStrategy::calculateMACD()
{
    double ema12 = calculateEMA(12);
    double ema26 = calculateEMA(26);
    double macd = ema12 - ema26;
    double signal = calculateEMA(9); // 9-period EMA of MACD
    return {macd, signal};
}

double TradingStrategy::calculateATR()
{
    if (prices.size() < 2)
        return 0.0;

    double sum = 0.0;
    for (size_t i = 1; i < prices.size(); ++i)
    {
        sum += std::abs(prices[i] - prices[i - 1]);
    }
    return sum / (prices.size() - 1);
}
