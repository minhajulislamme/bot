#include <iostream>
#include <thread>
#include <chrono>
#include <curl/curl.h>
#include "binance_utils.h"
#include "websocket_client.h"
#include "trading_strategy.h"
#include "risk_manager.h"
#include "order_manager.h"
#include "websocket_logging.h"
#include "market_analyzer.h"
#include "trading_pairs.h"
#include <map>
#include <mutex>

// Binance Futures Testnet
const std::string API_HOST = "https://testnet.binancefuture.com";

// Testnet API keys (replace these with your actual keys)
const std::string API_KEY = "bb0ba32b12f6188db14096d2b2e4c1bc43592b2e265b5fd2ca81d5df56316884";
const std::string API_SECRET = "7d95dcd173e0e24eef369713fa716ff04a23cf3de5ec42e0470365ab32fac237";

// Add new global variable for price tracking
std::map<std::string, double> currentPrices;
std::mutex pricesMutex;

size_t writeCallback(void *contents, size_t size, size_t nmemb, std::string *output)
{
    size_t totalSize = size * nmemb;
    output->append((char *)contents, totalSize);
    return totalSize;
}

class TradingBot
{
private:
    TradingStrategy strategy;
    RiskManager riskManager;
    OrderManager orderManager;
    std::string symbol;
    double lastPrice;
    bool inPosition;
    MarketAnalyzer marketAnalyzer;
    std::chrono::system_clock::time_point lastAnalysisTime;

public:
    TradingBot(const std::string &sym, const std::string &apiKey, const std::string &apiSecret)
        : strategy(14, 20, 26),          // RSI, EMA, MACD periods
          riskManager(0.01, 0.02, 0.01), // maxPosition, maxDailyLoss, riskPerTrade
          orderManager(apiKey, apiSecret),
          symbol(sym),
          lastPrice(0),
          inPosition(false)
    {
        // Initial market analysis
        updateMarketAnalysis();
    }

    void updateMarketAnalysis()
    {
        if (marketAnalyzer.loadHistoricalData(symbol))
        {
            strategy.updateMarketAnalysis(marketAnalyzer);
            std::cout << "Market Analysis Updated - Trend: "
                      << (marketAnalyzer.isUptrend() ? "UP" : "DOWN")
                      << " Strength: " << marketAnalyzer.getTrendStrength() << std::endl;
        }
    }

    void processPrice(double price, double volume)
    {
        // Update market analysis every 4 hours
        auto now = std::chrono::system_clock::now();
        if (now - lastAnalysisTime > std::chrono::hours(4))
        {
            updateMarketAnalysis();
            lastAnalysisTime = now;
        }

        if (lastPrice == 0)
        {
            lastPrice = price;
            return;
        }

        strategy.addIndicatorData(price, volume);
        TradingSignal signal = strategy.analyzeWithMarketContext(price, volume);

        // Check if we need to close any positions
        if (inPosition && riskManager.shouldClosePosition(symbol, price))
        {
            closeAllPositions();
            return;
        }

        // Process trading signal
        if (signal.type != TradingSignal::HOLD && signal.confidence > 0.7)
        {
            if (signal.type == TradingSignal::BUY && !inPosition)
            {
                if (riskManager.canTrade(symbol, price))
                {
                    double quantity = riskManager.getPositionSize(price, signal.stopLoss);
                    if (orderManager.placeMarketOrder(symbol, "BUY", quantity,
                                                      signal.stopLoss, signal.takeProfit))
                    {
                        inPosition = true;
                        RiskManager::Position pos{
                            symbol, "BUY", price, quantity,
                            signal.stopLoss, signal.takeProfit};
                        riskManager.updatePosition(pos);
                    }
                }
            }
            else if (signal.type == TradingSignal::SELL && inPosition)
            {
                closeAllPositions();
            }
        }

        lastPrice = price;
    }

private:
    void closeAllPositions()
    {
        if (inPosition)
        {
            orderManager.placeMarketOrder(symbol, "SELL",
                                          riskManager.getPositionQuantity(symbol), 0, 0);
            riskManager.closePosition(symbol);
            inPosition = false;
        }
    }
};

class MultiPairTradingBot
{
private:
    std::map<std::string, std::unique_ptr<TradingBot>> bots;
    std::vector<std::string> symbols;

public:
    MultiPairTradingBot(const std::vector<std::string> &tradingPairs,
                        const std::string &apiKey,
                        const std::string &apiSecret)
    {
        for (const auto &pair : tradingPairs)
        {
            symbols.push_back(pair);
            bots[pair] = std::make_unique<TradingBot>(pair, apiKey, apiSecret);
        }
    }

    void processAllPrices()
    {
        std::lock_guard<std::mutex> lock(pricesMutex);
        for (const auto &pair : currentPrices)
        {
            if (bots.find(pair.first) != bots.end())
            {
                bots[pair.first]->processPrice(pair.second, 0);
            }
        }
    }

    const std::vector<std::string> &getSymbols() const
    {
        return symbols;
    }
};

// Update WebSocket callback to handle multiple symbols
void onPriceUpdate(const std::string &symbol, double price)
{
    std::lock_guard<std::mutex> lock(pricesMutex);
    currentPrices[symbol] = price;
}

int main()
{
    // Initialize curl globally
    curl_global_init(CURL_GLOBAL_ALL);

    // Setup WebSocket logging with debug mode off
    setup_lws_logging(false);

    // Create vector of symbols from trading pairs
    std::vector<std::string> tradingPairs;
    for (const auto &pair : TRADING_PAIRS)
    {
        tradingPairs.push_back(pair.symbol);
    }

    // Initialize multi-pair trading bot
    MultiPairTradingBot trader(tradingPairs, API_KEY, API_SECRET);

    // Start WebSocket connections for all pairs
    std::vector<std::thread> wsThreads;
    for (const auto &symbol : trader.getSymbols())
    {
        wsThreads.push_back(std::thread(runWebSocketClient, symbol));
    }

    // Trading loop
    while (true)
    {
        trader.processAllPrices();
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    // Join all threads
    for (auto &thread : wsThreads)
    {
        thread.join();
    }

    // Clean up curl
    curl_global_cleanup();

    return 0;
}
