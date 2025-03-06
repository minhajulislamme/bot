#ifndef DATA_HEALTH_MONITOR_H
#define DATA_HEALTH_MONITOR_H

#include <map>
#include <string>
#include <chrono>
#include <mutex>

class DataHealthMonitor
{
public:
    static void recordDataPoint(const std::string &symbol, double price);
    static bool isDataFresh(const std::string &symbol, int maxAgeSeconds = 60);
    static int getDataPointCount(const std::string &symbol);
    static void reportDataHealth();

    // Initialize monitoring
    static void init();

private:
    static std::map<std::string, std::chrono::system_clock::time_point> lastUpdates;
    static std::map<std::string, int> dataPointCounts;
    static std::map<std::string, double> lastPrices;
    static std::chrono::system_clock::time_point startTime;
    static std::mutex dataMutex;
};

#endif
