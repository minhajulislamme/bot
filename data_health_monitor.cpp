#include "data_health_monitor.h"
#include "telegram_notifier.h"
#include <spdlog/spdlog.h>
#include <sstream>
#include <iomanip>

std::map<std::string, std::chrono::system_clock::time_point> DataHealthMonitor::lastUpdates;
std::map<std::string, int> DataHealthMonitor::dataPointCounts;
std::map<std::string, double> DataHealthMonitor::lastPrices;
std::chrono::system_clock::time_point DataHealthMonitor::startTime;
std::mutex DataHealthMonitor::dataMutex;

void DataHealthMonitor::init()
{
    startTime = std::chrono::system_clock::now();
}

void DataHealthMonitor::recordDataPoint(const std::string &symbol, double price)
{
    std::lock_guard<std::mutex> lock(dataMutex);
    lastUpdates[symbol] = std::chrono::system_clock::now();
    dataPointCounts[symbol]++;
    lastPrices[symbol] = price;
}

bool DataHealthMonitor::isDataFresh(const std::string &symbol, int maxAgeSeconds)
{
    std::lock_guard<std::mutex> lock(dataMutex);
    auto it = lastUpdates.find(symbol);
    if (it == lastUpdates.end())
    {
        return false;
    }

    auto now = std::chrono::system_clock::now();
    auto age = std::chrono::duration_cast<std::chrono::seconds>(now - it->second).count();
    return age < maxAgeSeconds;
}

int DataHealthMonitor::getDataPointCount(const std::string &symbol)
{
    std::lock_guard<std::mutex> lock(dataMutex);
    return dataPointCounts[symbol];
}

void DataHealthMonitor::reportDataHealth()
{
    std::lock_guard<std::mutex> lock(dataMutex);
    std::stringstream report;
    auto now = std::chrono::system_clock::now();

    report << "🔍 Market Data Health Report\n";
    report << "━━━━━━━━━━━━━━━━━━━━━\n";

    bool hasStaleData = false;

    for (const auto &pair : lastUpdates)
    {
        auto age = std::chrono::duration_cast<std::chrono::seconds>(now - pair.second).count();
        std::string status = (age < 60) ? "✅" : "⚠️";

        if (age >= 60)
            hasStaleData = true;

        report << status << " " << pair.first << ": ";
        report << dataPointCounts[pair.first] << " points, ";
        report << "Last price: " << std::fixed << std::setprecision(2) << lastPrices[pair.first];
        report << ", Updated " << age << "s ago\n";
    }

    auto uptime = std::chrono::duration_cast<std::chrono::hours>(now - startTime).count();
    report << "\n⏱️ Uptime: " << uptime << " hours\n";

    if (hasStaleData)
    {
        report << "\n⚠️ Warning: Some data streams may be stale!";
    }

    spdlog::info("Data health report generated");
    TelegramNotifier::sendMessage(report.str());
}
