#include "market_analyzer.h"
#include "binance_utils.h"
#include <curl/curl.h>
#include <json/json.h>
#include <numeric>
#include <algorithm>
#include <cmath>

MarketAnalyzer::MarketAnalyzer()
{
    // Initialize empty containers and default values
    historicalData.clear();
    marketStats.clear();
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp)
{
    userp->append((char *)contents, size * nmemb);
    return size * nmemb;
}

bool MarketAnalyzer::loadHistoricalData(const std::string &symbol, const std::string &interval, int limit)
{
    std::string endpoint = "https://testnet.binancefuture.com/fapi/v1/klines";
    std::string query = "symbol=" + symbol + "&interval=" + interval + "&limit=" + std::to_string(limit);

    CURL *curl = curl_easy_init();
    if (!curl)
        return false;

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, (endpoint + "?" + query).c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
        return false;

    // Parse JSON response
    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(response, root))
        return false;

    historicalData.clear();
    for (const auto &kline : root)
    {
        MarketData data;
        data.timestamp = std::stol(kline[0].asString());
        data.open = std::stod(kline[1].asString());
        data.high = std::stod(kline[2].asString());
        data.low = std::stod(kline[3].asString());
        data.close = std::stod(kline[4].asString());
        data.volume = std::stod(kline[5].asString());
        historicalData.push_back(data);
    }

    calculateStatistics();
    return true;
}

void MarketAnalyzer::calculateStatistics()
{
    if (historicalData.empty())
        return;

    // Calculate average volume
    double totalVolume = 0;
    for (const auto &data : historicalData)
    {
        totalVolume += data.volume;
    }
    marketStats["avgVolume"] = totalVolume / historicalData.size();

    // Calculate volatility (standard deviation of returns)
    std::vector<double> returns;
    for (size_t i = 1; i < historicalData.size(); i++)
    {
        double ret = (historicalData[i].close - historicalData[i - 1].close) / historicalData[i - 1].close;
        returns.push_back(ret);
    }

    double mean = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();
    double variance = 0;
    for (double ret : returns)
    {
        variance += (ret - mean) * (ret - mean);
    }
    marketStats["volatility"] = std::sqrt(variance / returns.size());

    // Identify support and resistance levels
    std::vector<double> lows, highs;
    for (const auto &data : historicalData)
    {
        lows.push_back(data.low);
        highs.push_back(data.high);
    }

    std::sort(lows.begin(), lows.end());
    std::sort(highs.begin(), highs.end());

    marketStats["support"] = lows[lows.size() * 0.2];      // 20th percentile
    marketStats["resistance"] = highs[highs.size() * 0.8]; // 80th percentile

    analyzeTrend();
}

void MarketAnalyzer::analyzeTrend()
{
    if (historicalData.size() < 2)
        return;

    // Simple trend analysis using linear regression
    double sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;
    int n = historicalData.size();

    for (int i = 0; i < n; i++)
    {
        sumX += i;
        sumY += historicalData[i].close;
        sumXY += i * historicalData[i].close;
        sumX2 += i * i;
    }

    double slope = (n * sumXY - sumX * sumY) / (n * sumX2 - sumX * sumX);
    marketStats["trendStrength"] = std::abs(slope);
    marketStats["trendDirection"] = slope > 0 ? 1.0 : -1.0;
}

double MarketAnalyzer::getTrendStrength() const
{
    auto it = marketStats.find("trendStrength");
    return it != marketStats.end() ? it->second : 0.0;
}

bool MarketAnalyzer::isUptrend() const
{
    auto it = marketStats.find("trendDirection");
    return it != marketStats.end() && it->second > 0;
}

bool MarketAnalyzer::isDowntrend() const
{
    auto it = marketStats.find("trendDirection");
    return it != marketStats.end() && it->second < 0;
}

double MarketAnalyzer::getMarketSentiment() const
{
    auto it = marketStats.find("trendDirection");
    return it != marketStats.end() ? it->second : 0.0;
}

double MarketAnalyzer::getSupportLevel() const
{
    auto it = marketStats.find("support");
    return it != marketStats.end() ? it->second : 0.0;
}

double MarketAnalyzer::getResistanceLevel() const
{
    auto it = marketStats.find("resistance");
    return it != marketStats.end() ? it->second : 0.0;
}

void MarketAnalyzer::updateCrossMarketCorrelations(const std::map<std::string, std::vector<MarketData>> &allData)
{
    // Clear existing correlations
    crossMarketCorrelations.clear();

    // Calculate correlations between all pairs
    for (auto it1 = allData.begin(); it1 != allData.end(); ++it1)
    {
        for (auto it2 = std::next(it1); it2 != allData.end(); ++it2)
        {
            // Extract closing prices for correlation calculation
            std::vector<double> prices1, prices2;
            size_t minSize = std::min(it1->second.size(), it2->second.size());

            for (size_t i = 0; i < minSize; ++i)
            {
                prices1.push_back(it1->second[i].close);
                prices2.push_back(it2->second[i].close);
            }

            // Calculate correlation if we have enough data points
            if (prices1.size() >= 2)
            {
                double correlation = calculateCorrelation(prices1, prices2);
                crossMarketCorrelations[{it1->first, it2->first}] = correlation;
                crossMarketCorrelations[{it2->first, it1->first}] = correlation;
            }
        }
    }
}

// Add missing helper method for correlation calculation
double MarketAnalyzer::calculateCorrelation(const std::vector<double> &x, const std::vector<double> &y) const
{
    double sum_x = 0.0, sum_y = 0.0;
    double sum_xy = 0.0;
    double sum_x2 = 0.0, sum_y2 = 0.0;
    size_t n = x.size();

    for (size_t i = 0; i < n; ++i)
    {
        sum_x += x[i];
        sum_y += y[i];
        sum_xy += x[i] * y[i];
        sum_x2 += x[i] * x[i];
        sum_y2 += y[i] * y[i];
    }

    double denominator = sqrt((n * sum_x2 - sum_x * sum_x) * (n * sum_y2 - sum_y * sum_y));
    if (denominator == 0)
        return 0;

    return (n * sum_xy - sum_x * sum_y) / denominator;
}

double MarketAnalyzer::getAverageVolume() const
{
    auto it = marketStats.find("avgVolume");
    return it != marketStats.end() ? it->second : 0.0;
}

double MarketAnalyzer::getAverageRange() const
{
    if (historicalData.empty())
        return 0.0;
    double sum = 0.0;
    for (const auto &data : historicalData)
    {
        sum += (data.high - data.low);
    }
    return sum / historicalData.size();
}

double MarketAnalyzer::getVolatility() const
{
    auto it = marketStats.find("volatility");
    return it != marketStats.end() ? it->second : 0.0;
}

bool MarketAnalyzer::isOverbought() const
{
    auto it = marketStats.find("rsi");
    return it != marketStats.end() && it->second > 70.0;
}

bool MarketAnalyzer::isOversold() const
{
    auto it = marketStats.find("rsi");
    return it != marketStats.end() && it->second < 30.0;
}

bool MarketAnalyzer::isMarketOverextended() const
{
    return getVolatility() > OVEREXTENDED_THRESHOLD;
}

double MarketAnalyzer::getPairCorrelation(const std::string &pair1, const std::string &pair2) const
{
    auto it = crossMarketCorrelations.find({pair1, pair2});
    return it != crossMarketCorrelations.end() ? it->second : 0.0;
}
