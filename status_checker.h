#ifndef STATUS_CHECKER_H
#define STATUS_CHECKER_H

#include <string>
#include <chrono>
#include <vector>

class StatusChecker
{
public:
    static void init();
    static void checkAndSendPeriodicStatus();
    static void setSupportedSymbols(const std::vector<std::string> &symbols);
    static std::chrono::system_clock::time_point getStartTime() { return startTime; }

private:
    static std::chrono::system_clock::time_point startTime;
    static std::chrono::system_clock::time_point lastStatusUpdate;
    static constexpr int STATUS_UPDATE_INTERVAL_HOURS = 5;
    static std::vector<std::string> tradingSymbols;
};

#endif
