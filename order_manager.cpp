#include "order_manager.h"
#include "binance_utils.h"
#include <curl/curl.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <json/json.h>     // Add this for Json::Value
#include <spdlog/spdlog.h> // Add this for spdlog

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp)
{
    userp->append((char *)contents, size * nmemb);
    return size * nmemb;
}

OrderManager::OrderManager(const std::string &apiKey, const std::string &apiSecret)
    : apiKey(apiKey), apiSecret(apiSecret) {}

bool OrderManager::placeMarketOrder(const std::string &symbol,
                                    const std::string &side,
                                    double quantity,
                                    double stopLoss,
                                    double takeProfit)
{
    // Place main market order
    std::string endpoint = "/fapi/v1/order";
    std::string timestamp = getTimestamp();

    // Format quantity with 3 decimal places
    std::stringstream ss;
    ss << std::fixed << std::setprecision(3) << quantity;

    std::string queryString = "symbol=" + symbol +
                              "&side=" + side +
                              "&type=MARKET" +
                              "&quantity=" + ss.str() +
                              "&timestamp=" + timestamp;

    std::string signature = hmac_sha256(apiSecret, queryString);
    queryString += "&signature=" + signature;

    CURL *curl = curl_easy_init();
    if (!curl)
        return false;

    std::string response;
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, ("X-MBX-APIKEY: " + apiKey).c_str());

    std::string url = "https://testnet.binancefuture.com" + endpoint + "?" + queryString;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    bool success = (res == CURLE_OK);

    if (success)
    {
        std::cout << "Market order placed: " << response << std::endl;

        // Place stop loss order
        if (stopLoss > 0)
        {
            placeStopOrder(symbol,
                           side == "BUY" ? "SELL" : "BUY",
                           quantity,
                           stopLoss,
                           "STOP_MARKET");
        }

        // Place take profit order
        if (takeProfit > 0)
        {
            placeStopOrder(symbol,
                           side == "BUY" ? "SELL" : "BUY",
                           quantity,
                           takeProfit,
                           "TAKE_PROFIT_MARKET");
        }
    }
    else
    {
        std::cerr << "Failed to place market order: " << curl_easy_strerror(res) << std::endl;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return success;
}

bool OrderManager::cancelOrder([[maybe_unused]] const std::string &symbol, [[maybe_unused]] const std::string &orderId)
{
    // TODO: Implement order cancellation
    return true;
}

double OrderManager::getAccountBalance()
{
    std::string endpoint = "/fapi/v2/balance";
    std::string timestamp = getTimestamp();

    std::string queryString = "timestamp=" + timestamp;
    std::string signature = hmac_sha256(apiSecret, queryString);
    queryString += "&signature=" + signature;

    CURL *curl = curl_easy_init();
    if (!curl)
        return 0.0;

    std::string response;
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, ("X-MBX-APIKEY: " + apiKey).c_str());

    std::string url = "https://testnet.binancefuture.com" + endpoint + "?" + queryString;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
    {
        spdlog::error("Failed to get account balance: {}", curl_easy_strerror(res));
        return 0.0;
    }

    try
    {
        Json::Value root;
        Json::Reader reader;
        if (reader.parse(response, root) && root.isArray())
        {
            for (const auto &asset : root)
            {
                if (asset["asset"].asString() == "USDT")
                {
                    return std::stod(asset["balance"].asString());
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        spdlog::error("Error parsing balance response: {}", e.what());
    }

    return 0.0;
}

void OrderManager::updateOpenOrders()
{
    // TODO: Implement open orders update
}

bool OrderManager::placeStopOrder(const std::string &symbol,
                                  const std::string &side,
                                  double quantity,
                                  double stopPrice,
                                  const std::string &orderType)
{
    std::string endpoint = "/fapi/v1/order";
    std::string timestamp = getTimestamp();

    std::stringstream ss, ps;
    ss << std::fixed << std::setprecision(3) << quantity;
    ps << std::fixed << std::setprecision(2) << stopPrice;

    std::string queryString = "symbol=" + symbol +
                              "&side=" + side +
                              "&type=" + orderType +
                              "&quantity=" + ss.str() +
                              "&stopPrice=" + ps.str() +
                              "&timestamp=" + timestamp;

    std::string signature = hmac_sha256(apiSecret, queryString);
    queryString += "&signature=" + signature;

    CURL *curl = curl_easy_init();
    if (!curl)
        return false;

    std::string response;
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, ("X-MBX-APIKEY: " + apiKey).c_str());

    std::string url = "https://testnet.binancefuture.com" + endpoint + "?" + queryString;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    bool success = (res == CURLE_OK);

    if (success)
    {
        std::cout << orderType << " order placed: " << response << std::endl;
    }
    else
    {
        std::cerr << "Failed to place " << orderType << " order: " << curl_easy_strerror(res) << std::endl;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return success;
}
