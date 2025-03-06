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

// Changed return type to bool
bool TelegramNotifier::sendMessage(const std::string &message)
{
    CURL *curl = curl_easy_init();
    if (!curl)
        return false;

    // Use proper chat ID from config
    char *escaped_text = curl_easy_escape(curl, message.c_str(), 0);
    if (!escaped_text)
    {
        curl_easy_cleanup(curl);
        return false;
    }

    std::string params = "chat_id=" + Config::TELEGRAM_CHAT_ID + "&text=" + escaped_text;
    bool success = sendRequest("sendMessage", params);

    if (success)
    {
        spdlog::debug("Message sent to chat ID: {}", Config::TELEGRAM_CHAT_ID);
    }

    curl_free(escaped_text);
    curl_easy_cleanup(curl);
    return success; // Return success status
}

void TelegramNotifier::notifyTrade(const std::string &symbol, const std::string &side,
                                   double price, double quantity)
{
    try
    {
        std::stringstream ss;
        double totalValue = price * quantity;
        std::time_t now = std::time(nullptr);

        ss << "🤖 Trade Executed\n"
           << "━━━━━━━━━━━━━━━━━━━━━\n"
           << "Symbol: " << symbol << "\n"
           << "Action: " << (side == "BUY" ? "🟢 " : "🔴 ") << side << "\n"
           << "Price: $" << std::fixed << std::setprecision(2) << price << "\n"
           << "Quantity: " << std::setprecision(6) << quantity << "\n"
           << "Total Value: $" << std::setprecision(2) << totalValue << "\n"
           << "Time: " << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S") << "\n"
           << "Strategy: RSI, MACD, EMA";

        spdlog::info("Sending trade notification for {}: {} @ ${:.2f}", symbol, side, price);
        bool sent = sendMessage(ss.str());

        if (sent)
        {
            spdlog::info("Trade notification sent successfully");
        }
        else
        {
            spdlog::error("Failed to send trade notification");
        }
    }
    catch (const std::exception &e)
    {
        spdlog::error("Error while sending trade notification: {}", e.what());
    }
}

void TelegramNotifier::notifyBalance(double balance)
{
    std::stringstream ss;
    ss << "💰 Balance Update\n"
       << "━━━━━━━━━━━━━━━━━━━━━\n"
       << "Total Balance: $" << std::fixed << std::setprecision(2) << balance << "\n"
       << "Trading Balance (10%): $" << (balance * 0.1) << "\n"
       << "Reserved (90%): $" << (balance * 0.9) << "\n"
       << "Min Required: $" << (balance * 0.01) << " (1%)\n\n"
       << "Trading Capacity:\n"
       << "• Max Position: $" << (balance * 0.1 * Config::MAX_POSITION_SIZE) << "\n"
       << "• Risk Per Trade: $" << (balance * 0.1 * Config::RISK_PER_TRADE) << "\n"
       << "• Daily Loss Limit: $" << (balance * Config::MAX_DAILY_LOSS);
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
       << "📅 " << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S") << "\n\n"
       << "💰 Account Balance\n"
       << "Total: $" << std::fixed << std::setprecision(2) << Config::INITIAL_BALANCE << "\n"
       << "Trading (10%): $" << (Config::INITIAL_BALANCE * 0.1) << "\n"
       << "Reserved (90%): $" << (Config::INITIAL_BALANCE * 0.9) << "\n"
       << "Min Required: $" << (Config::INITIAL_BALANCE * 0.01) << "\n\n"
       << "📊 Risk Parameters\n"
       << "• Max Position: " << (Config::MAX_POSITION_SIZE * 100) << "% ($"
       << (Config::INITIAL_BALANCE * 0.1 * Config::MAX_POSITION_SIZE) << ")\n"
       << "• Risk per Trade: " << (Config::RISK_PER_TRADE * 100) << "% ($"
       << (Config::INITIAL_BALANCE * 0.1 * Config::RISK_PER_TRADE) << ")\n"
       << "• Max Daily Loss: " << (Config::MAX_DAILY_LOSS * 100) << "% ($"
       << (Config::INITIAL_BALANCE * Config::MAX_DAILY_LOSS) << ")\n\n"
       << "💱 Trading Pairs:\n";

    for (const auto &pair : tradingPairs)
    {
        ss << "• " << pair << "\n";
    }

    ss << "\n🔄 Trading Strategy\n"
       << "• Time Frame: 1h\n"
       << "• Indicators: RSI, MACD, EMA\n"
       << "• Max Concurrent Trades: " << Config::MAX_CONCURRENT_TRADES << "\n"
       << "\n💡 Status: Online and monitoring markets\n"
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

    std::string response;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
    {
        spdlog::error("Failed to send Telegram notification: {} (URL: {})",
                      curl_easy_strerror(res), url);
        return false;
    }

    spdlog::debug("Telegram API response: {}", response);
    return true;
}

// Add callback for curl response
size_t TelegramNotifier::writeCallback(void *contents, size_t size, size_t nmemb, std::string *s)
{
    size_t newLength = size * nmemb;
    try
    {
        s->append((char *)contents, newLength);
        return newLength;
    }
    catch (std::bad_alloc &e)
    {
        return 0;
    }
}
