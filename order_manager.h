#ifndef ORDER_MANAGER_H
#define ORDER_MANAGER_H

#include <string>
#include "risk_manager.h"
#include <map>

class OrderManager
{
public:
    OrderManager(const std::string &apiKey, const std::string &apiSecret);

    bool placeMarketOrder(const std::string &symbol,
                          const std::string &side,
                          double quantity,
                          double stopLoss = 0.0,
                          double takeProfit = 0.0);

    bool placeStopOrder(const std::string &symbol,
                        const std::string &side,
                        double quantity,
                        double stopPrice,
                        const std::string &orderType);

    bool cancelOrder(const std::string &symbol, const std::string &orderId);
    double getAccountBalance();
    void updateOpenOrders();

private:
    std::string apiKey;
    std::string apiSecret;
    std::map<std::string, std::string> openOrders;
};

#endif
