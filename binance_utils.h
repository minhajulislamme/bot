#ifndef BINANCE_UTILS_H
#define BINANCE_UTILS_H

#include <string>

std::string getTimestamp();
std::string hmac_sha256(const std::string& key, const std::string& data);
std::string urlencode(const std::string& s);

#endif
