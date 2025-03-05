#ifndef HEALTH_MONITOR_H
#define HEALTH_MONITOR_H

#include <prometheus/counter.h>
#include <prometheus/exposer.h>

class HealthMonitor
{
public:
    static void init();
    static void recordTrade(const std::string &symbol);
    static void recordError(const std::string &type);
    static void updateMetrics();

private:
    static std::unique_ptr<prometheus::Exposer> exposer;
    static std::shared_ptr<prometheus::Registry> registry;
};

#endif
