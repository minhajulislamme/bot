#include "status_checker.h"
#include "telegram_notifier.h"
#include "data_health_monitor.h"
#include <sstream>
#include <iomanip>
#include <chrono>
#include <spdlog/spdlog.h>

std::chrono::system_clock::time_point StatusChecker::startTime;
std::chrono::system_clock::time_point StatusChecker::lastStatusUpdate;
std::vector<std::string> StatusChecker::tradingSymbols;

void StatusChecker::init()
{
    startTime = std::chrono::system_clock::now();
    lastStatusUpdate = startTime;
}

void StatusChecker::setSupportedSymbols(const std::vector<std::string> &symbols)
{
    tradingSymbols = symbols;
}

void StatusChecker::checkAndSendPeriodicStatus()
{
    auto now = std::chrono::system_clock::now();
    auto hoursSinceLastUpdate = std::chrono::duration_cast<std::chrono::hours>(
                                    now - lastStatusUpdate)
                                    .count();

    if (hoursSinceLastUpdate >= STATUS_UPDATE_INTERVAL_HOURS)
    {
        // Create status report
        std::stringstream ss;
        auto totalUptime = std::chrono::duration_cast<std::chrono::hours>(now - startTime).count();

        std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

        ss << "🤖 Bot Status Report\n"
           << "━━━━━━━━━━━━━━━━━━━━━\n"
           << "🕒 Current time: " << std::put_time(std::localtime(&currentTime), "%Y-%m-%d %H:%M:%S") << "\n"
           << "⏱️ Uptime: " << totalUptime << " hours\n\n"
           << "📊 Market Data Status:\n";

        // Check data freshness for each symbol
        bool allDataFresh = true;
        for (const auto &symbol : tradingSymbols)
        {
            bool isFresh = DataHealthMonitor::isDataFresh(symbol);
            ss << (isFresh ? "✅" : "⚠️") << " " << symbol << ": ";
            ss << (isFresh ? "Active" : "Stale") << " (";
            ss << DataHealthMonitor::getDataPointCount(symbol) << " data points)\n";

            if (!isFresh)
                allDataFresh = false;
        }

        ss << "\n📡 Connection Status: " << (allDataFresh ? "✅ All connections active" : "⚠️ Some connections may be down") << "\n";
        ss << "💹 Trading Status: Active\n";

        // Send the status report
        TelegramNotifier::sendMessage(ss.str());
        spdlog::info("Sent periodic status update (after {} hours)", STATUS_UPDATE_INTERVAL_HOURS);

        // Update the last status update time
        lastStatusUpdate = now;
    }
}
