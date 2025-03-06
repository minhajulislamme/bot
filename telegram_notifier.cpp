#include "telegram_notifier.h"
#include "config.h" // Add this include
#include <sstream>
#include <iomanip>

std::string TelegramNotifier::botToken;
const std::string TelegramNotifier::API_URL = "https://api.telegram.org/bot";

void TelegramNotifier::init(const std::string &token)
{
    botToken = token;
}

void TelegramNotifier::sendMessage(const std::string &message)
{
    CURL *curl = curl_easy_init();
    if (!curl)
        return;

    // Use proper chat ID from config
    char *escaped_text = curl_easy_escape(curl, message.c_str(), 0);
    if (!escaped_text)
    {
        curl_easy_cleanup(curl);
        return;
    }

    std::string params = "chat_id=" + Config::TELEGRAM_CHAT_ID + "&text=" + escaped_text;
    bool success = sendRequest("sendMessage", params);

    if (success)
    {
        spdlog::debug("Message sent to chat ID: {}", Config::TELEGRAM_CHAT_ID);
    }

    curl_free(escaped_text);
    curl_easy_cleanup(curl);
}

void TelegramNotifier::notifyTrade(const std::string &symbol, const std::string &side,
                                   double price, double quantity)
{
    std::stringstream ss;
    ss << "🤖 Trade Alert\n"
       << "Symbol: " << symbol << "\n"
       << "Side: " << side << "\n"
       << "Price: $" << std::fixed << std::setprecision(2) << price << "\n"
       << "Quantity: " << quantity;
    sendMessage(ss.str());
}

void TelegramNotifier::notifyBalance(double balance)
{
    std::stringstream ss;
    ss << "💰 Balance Update\n"
       << "Current Balance: $" << std::fixed << std::setprecision(2) << balance;
    sendMessage(ss.str());
}

void TelegramNotifier::notifyError(const std::string &error)
{
    std::string message = "⚠️ Error: " + error;
    sendMessage(message);
}

void TelegramNotifier::notifyStartup(const std::vector<std::string> &tradingPairs)
{
    std::stringstream ss;
    std::time_t now = std::time(nullptr);
    ss << "🚀 Binance Trading Bot Started\n"
       << "━━━━━━━━━━━━━━━━━━━━━\n"
       << "📅 Date: " << std::put_time(std::localtime(&now), "%Y-%m-%d") << "\n"
       << "⏰ Time: " << std::put_time(std::localtime(&now), "%H:%M:%S") << "\n"
       << "💱 Active Trading Pairs:\n";

    for (const auto &pair : tradingPairs)
    {
        ss << "• " << pair << "\n";
    }

    ss << "\n📊 Trading Settings:\n"
       << "• Max Positions: " << Config::MAX_CONCURRENT_TRADES << "\n"
       << "• Risk per Trade: " << (Config::RISK_PER_TRADE * 100) << "%\n"
       << "• Min Balance: $" << Config::MIN_CONFIDENCE << "\n"
       << "\n💡 Bot Status: Online and monitoring markets\n"
       << "━━━━━━━━━━━━━━━━━━━━━";

    sendMessage(ss.str());
}

void TelegramNotifier::sendTestMessage()
{
    std::stringstream ss;
    std::time_t now = std::time(nullptr);
    ss << "🔔 Test Notification\n"
       << "Bot is successfully connected!\n"
       << "Time: " << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S") << "\n"
       << "Bot Name: trading1332bot\n"
       << "Status: Ready to trade";
    sendMessage(ss.str());
}

bool TelegramNotifier::sendRequest(const std::string &method, const std::string &params)
{
    CURL *curl = curl_easy_init();
    if (!curl)
        return false;

    std::string url = API_URL + botToken + "/" + method + "?" + params;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); // Add timeout

    // Add debug logging
    spdlog::debug("Sending Telegram request: {}", url);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
    {
        spdlog::error("Failed to send Telegram notification: {} (URL: {})",
                      curl_easy_strerror(res), url);
        return false;
    }

    spdlog::debug("Telegram notification sent successfully");
    return true;
}
