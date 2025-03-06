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
#include "logger.h"
#include "config.h"
#include "telegram_notifier.h"
#include "data_health_monitor.h" // Add this include
#include "status_checker.h"      // Add this include
#include <map>
#include <mutex>
#include <iomanip> // Add this for std::setprecision

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
    std::shared_ptr<RiskManager> riskManager;
    OrderManager orderManager;
    std::string symbol;
    double lastPrice;
    bool inPosition;
    MarketAnalyzer marketAnalyzer;
    std::chrono::system_clock::time_point lastAnalysisTime;
    double lastKnownBalance;
    std::chrono::system_clock::time_point lastBalanceCheck;
    static constexpr int BALANCE_CHECK_INTERVAL = 60;       // seconds
    std::map<std::string, RiskManager::Position> positions; // Add this member

    bool checkAndUpdateBalance()
    {
        double currentBalance = orderManager.getAccountBalance();
        if (currentBalance <= 0)
        {
            TelegramNotifier::notifyError("Failed to fetch account balance");
            return false;
        }

        if (currentBalance != lastKnownBalance)
        {
            std::stringstream ss;
            ss << "💰 Balance Change Detected\n"
               << "Previous: $" << std::fixed << std::setprecision(2) << lastKnownBalance << "\n"
               << "Current: $" << currentBalance << "\n"
               << "Change: " << (currentBalance > lastKnownBalance ? "+" : "")
               << std::setprecision(2) << (currentBalance - lastKnownBalance);
            TelegramNotifier::sendMessage(ss.str());

            lastKnownBalance = currentBalance;
            riskManager->updateAccountBalance(currentBalance);
        }
        return true;
    }

    bool canPlaceTrade(double price, double quantity)
    {
        if (std::chrono::system_clock::now() - lastBalanceCheck >
            std::chrono::seconds(BALANCE_CHECK_INTERVAL))
        {
            if (!checkAndUpdateBalance())
            {
                return false;
            }
            lastBalanceCheck = std::chrono::system_clock::now();
        }

        double requiredBalance = price * quantity * (1.0 + 0.01); // Include margin
        return riskManager->isBalanceSufficient(requiredBalance);
    }

public:
    TradingBot(const std::string &sym, const std::string &apiKey, const std::string &apiSecret, std::shared_ptr<RiskManager> globalRiskManager)
        : strategy(14, 20, 26), // RSI, EMA, MACD periods
          riskManager(globalRiskManager),
          orderManager(apiKey, apiSecret),
          symbol(sym),
          lastPrice(0),
          inPosition(false),
          lastKnownBalance(0)
    {
        // Initialize with current balance
        lastKnownBalance = orderManager.getAccountBalance();
        lastBalanceCheck = std::chrono::system_clock::now();
        riskManager->updateAccountBalance(lastKnownBalance);

        // Notify initial balance
        TelegramNotifier::notifyBalance(lastKnownBalance);

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

    void updateAccountBalance()
    {
        double balance = orderManager.getAccountBalance();
        if (balance > 0)
        {
            riskManager->updateAccountBalance(balance);
            spdlog::info("Account balance updated: ${:.2f}", balance);

            // Adjust position sizes based on new balance
            double maxPositionSize = riskManager->getMaxPositionSizeForBalance();
            spdlog::info("Max position size: ${:.2f}", maxPositionSize);
        }
    }

    void processPrice(double price, double volume)
    {
        // Update balance every hour
        auto now = std::chrono::system_clock::now();
        static auto lastBalanceCheck = now;
        if (now - lastBalanceCheck > std::chrono::hours(1))
        {
            updateAccountBalance();
            lastBalanceCheck = now;
        }

        // Update market analysis every 4 hours
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
        if (inPosition && riskManager->shouldClosePosition(symbol, price))
        {
            closeAllPositions();
            return;
        }

        // Process trading signal
        if (signal.type != TradingSignal::HOLD && signal.confidence > 0.7)
        {
            if (signal.type == TradingSignal::BUY && !inPosition)
            {
                if (riskManager->canTrade(symbol, price))
                {
                    double quantity = riskManager->getPositionSize(price, signal.stopLoss);

                    if (canPlaceTrade(price, quantity))
                    {
                        double requiredBalance = price * quantity * (1.0 + 0.01);

                        if (riskManager->isBalanceSufficient(requiredBalance))
                        {
                            if (placeTradeWithNotification("BUY", quantity, price,
                                                           signal.stopLoss, signal.takeProfit))
                            {
                                inPosition = true;
                                RiskManager::Position pos{
                                    symbol, "BUY", price, quantity,
                                    signal.stopLoss, signal.takeProfit,
                                    std::chrono::system_clock::now() // Add entry time
                                };
                                riskManager->updatePosition(pos);
                            }
                        }
                        else
                        {
                            spdlog::warn("Insufficient balance for trade on {}", symbol);
                        }
                    }
                    else
                    {
                        spdlog::warn("Insufficient balance or balance check failed for {} trade", symbol);
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

    bool hasOpenPosition() const
    {
        return inPosition;
    }

    MarketAnalyzer &getMarketAnalyzer()
    {
        return marketAnalyzer;
    }

private:
    void closeAllPositions()
    {
        if (inPosition)
        {
            double quantity = riskManager->getPositionQuantity(symbol);
            double currentPrice = lastPrice; // Use latest price

            if (placeTradeWithNotification("SELL", quantity, currentPrice, 0, 0))
            {
                auto it = positions.find(symbol);
                if (it != positions.end())
                {
                    double pnl = (currentPrice - it->second.entryPrice) * quantity;

                    std::stringstream ss;
                    ss << "💰 Position Closed\n"
                       << "PnL: " << (pnl >= 0 ? "+" : "") << std::fixed
                       << std::setprecision(2) << "$" << pnl << "\n"
                       << "Hold Time: " << calculateHoldTime(it->second.entryTime);
                    TelegramNotifier::sendMessage(ss.str());
                }
                riskManager->closePosition(symbol);
                inPosition = false;
            }
        }
    }

    bool placeTradeWithNotification(const std::string &side, double quantity, double price,
                                    double stopLoss, double takeProfit)
    {
        if (orderManager.placeMarketOrder(symbol, side, quantity, stopLoss, takeProfit))
        {
            // Log the trade execution
            spdlog::info("Trade executed: {} {} {} at ${:.2f}", side, quantity, symbol, price);

            // Send trade notification
            TelegramNotifier::notifyTrade(symbol, side, price, quantity);

            // Calculate risk metrics
            double potentialLoss = abs(price - stopLoss) * quantity;
            double potentialProfit = abs(price - takeProfit) * quantity;
            double riskRewardRatio = potentialProfit / (potentialLoss > 0 ? potentialLoss : 1);

            // Send trade details notification
            std::stringstream ss;
            ss << "📊 Trade Details:\n"
               << "━━━━━━━━━━━━━━━━━━━━━\n"
               << "Symbol: " << symbol << "\n"
               << "Stop Loss: $" << std::fixed << std::setprecision(2) << stopLoss << "\n"
               << "Take Profit: $" << takeProfit << "\n"
               << "Potential Loss: $" << potentialLoss << "\n"
               << "Potential Profit: $" << potentialProfit << "\n"
               << "Risk/Reward: " << std::setprecision(2) << riskRewardRatio << "\n"
               << "Account Balance: $" << std::setprecision(2) << lastKnownBalance;
            TelegramNotifier::sendMessage(ss.str());

            return true;
        }

        // Log and notify failure
        spdlog::error("Failed to place {} order for {} {}", side, quantity, symbol);
        TelegramNotifier::notifyError("Failed to place " + side + " order for " + symbol);
        return false;
    }

    std::string calculateHoldTime(const std::chrono::system_clock::time_point &entryTime)
    {
        auto now = std::chrono::system_clock::now();
        auto duration = now - entryTime;
        auto hours = std::chrono::duration_cast<std::chrono::hours>(duration).count();
        auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration).count() % 60;

        std::stringstream ss;
        ss << hours << "h " << minutes << "m";
        return ss.str();
    }
};

class MultiPairTradingBot
{
private:
    std::map<std::string, std::unique_ptr<TradingBot>> bots;
    std::vector<std::string> symbols;
    std::shared_ptr<RiskManager> globalRiskManager;
    std::mutex tradingMutex;
    static constexpr int MAX_SIMULTANEOUS_TRADES = 5;
    std::map<std::string, std::chrono::system_clock::time_point> lastTradeTime;

    void updateGlobalMarketContext()
    {
        std::map<std::string, std::vector<MarketData>> allData;
        for (const auto &bot : bots)
        {
            allData[bot.first] = bot.second->getMarketAnalyzer().getHistoricalData();
        }

        for (auto &bot : bots)
        {
            bot.second->getMarketAnalyzer().updateCrossMarketCorrelations(allData);
        }
    }

    int countActivePositions() const
    {
        int count = 0;
        for (const auto &bot : bots)
        {
            if (bot.second->hasOpenPosition())
            {
                count++;
            }
        }
        return count;
    }

public:
    MultiPairTradingBot(const std::vector<std::string> &tradingPairs,
                        const std::string &apiKey,
                        const std::string &apiSecret)
        : globalRiskManager(std::make_shared<RiskManager>(0.01, 0.02, 0.01))
    {
        for (const auto &pair : tradingPairs)
        {
            symbols.push_back(pair);
            bots[pair] = std::make_unique<TradingBot>(pair, apiKey, apiSecret, globalRiskManager);
            lastTradeTime[pair] = std::chrono::system_clock::now();
        }
    }

    const std::vector<std::string> &getSymbols() const
    {
        return symbols;
    }

    void processAllPrices()
    {
        std::lock_guard<std::mutex> lock(tradingMutex);

        updateGlobalMarketContext();

        int activePositions = countActivePositions();

        if (activePositions >= MAX_SIMULTANEOUS_TRADES)
        {
            spdlog::info("Maximum concurrent positions ({}) reached", MAX_SIMULTANEOUS_TRADES);
            return;
        }

        std::lock_guard<std::mutex> priceLock(pricesMutex);
        for (const auto &pair : currentPrices)
        {
            if (bots.find(pair.first) != bots.end())
            {
                // Check cooldown period
                auto now = std::chrono::system_clock::now();
                auto lastTrade = lastTradeTime[pair.first];

                // Add minimum time between trades (5 minutes)
                if (now - lastTrade > std::chrono::minutes(5))
                {
                    bots[pair.first]->processPrice(pair.second, 0);
                    lastTradeTime[pair.first] = now;
                }
            }
        }
    }
};

int main()
{
    try
    {
        Logger::init();
        Config::loadFromFile("config.yaml");

        // Initialize health monitoring systems
        DataHealthMonitor::init();
        StatusChecker::init();

        // Initialize OrderManager to get initial balance
        OrderManager initialBalanceCheck(API_KEY, API_SECRET);
        double totalBalance = initialBalanceCheck.getInitialBalance();

        if (totalBalance <= 0)
        {
            spdlog::error("Failed to get initial balance from Binance");
            return 1;
        }

        // Set initial balance and calculate trading limits
        Config::INITIAL_BALANCE = totalBalance;
        Config::TRADING_BALANCE = totalBalance * 0.1; // Use 10% for trading

        spdlog::info("Total Balance: ${:.2f}", totalBalance);
        spdlog::info("Trading Balance (10%): ${:.2f}", Config::TRADING_BALANCE);

        // Initialize Telegram and rest of the code...
        TelegramNotifier::init(Config::TELEGRAM_TOKEN);

        // Send test notification
        TelegramNotifier::sendTestMessage();

        curl_global_init(CURL_GLOBAL_ALL);
        setup_lws_logging(false);

        std::vector<std::string> tradingPairs;
        for (const auto &pair : TRADING_PAIRS)
        {
            tradingPairs.push_back(pair.symbol);
        }

        // Configure status checker with trading pairs
        StatusChecker::setSupportedSymbols(tradingPairs);

        // Send startup notification
        TelegramNotifier::notifyStartup(tradingPairs);

        MultiPairTradingBot trader(tradingPairs, API_KEY, API_SECRET);
        std::vector<std::thread> wsThreads;

        for (const auto &symbol : trader.getSymbols())
        {
            wsThreads.push_back(std::thread(runWebSocketClient, symbol));
        }

        // Schedule data health reporting after 30 minutes
        auto nextDataHealthReport = std::chrono::system_clock::now() + std::chrono::minutes(30);

        while (true)
        {
            trader.processAllPrices();

            // Check if it's time for periodic status update
            StatusChecker::checkAndSendPeriodicStatus();

            // Check if it's time for data health report
            auto now = std::chrono::system_clock::now();
            if (now >= nextDataHealthReport)
            {
                DataHealthMonitor::reportDataHealth();
                // Schedule next report in 6 hours
                nextDataHealthReport = now + std::chrono::hours(6);
            }

            std::this_thread::sleep_for(std::chrono::seconds(5));
        }

        for (auto &thread : wsThreads)
        {
            thread.join();
        }

        curl_global_cleanup();
        return 0;
    }
    catch (const std::exception &e)
    {
        spdlog::error("Fatal error: {}", e.what());
        TelegramNotifier::notifyError(e.what());
        return 1;
    }
}
