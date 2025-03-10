#ifndef TELEGRAM_NOTIFIER_H
#define TELEGRAM_NOTIFIER_H

#include <string>
#include <curl/curl.h>
#include <spdlog/spdlog.h>
#include <vector>

class TelegramNotifier
{
public:
    static void init(const std::string &token);
    static bool sendMessage(const std::string &message); // Changed return type to bool
    static void notifyTrade(const std::string &symbol, const std::string &side,
                            double price, double quantity);
    static void notifyBalance(double balance);
    static void notifyError(const std::string &error);
    static void notifyStartup(const std::vector<std::string> &tradingPairs);
    static void sendTestMessage();

private:
    static std::string botToken;
    static const std::string API_URL;
    static bool sendRequest(const std::string &method, const std::string &params);
    static size_t writeCallback(void *contents, size_t size, size_t nmemb, std::string *s);
};

#endif
