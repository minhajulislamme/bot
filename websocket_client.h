#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include <string>
#include <map>
#include <mutex>

void runWebSocketClient(const std::string &symbol);
void onPriceUpdate(const std::string &symbol, double price);

extern std::map<std::string, double> currentPrices;
extern std::mutex pricesMutex;

#endif
