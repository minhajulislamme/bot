#include "binance_utils.h"
#include <chrono>
#include <openssl/hmac.h>
#include <iomanip>
#include <sstream>

std::string getTimestamp() {
    using namespace std::chrono;
    auto now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    return std::to_string(now);
}

std::string hmac_sha256(const std::string& key, const std::string& data) {
    unsigned char* digest;
    digest = HMAC(EVP_sha256(), key.c_str(), key.length(), 
                  (unsigned char*)data.c_str(), data.length(), NULL, NULL);
    std::ostringstream result;
    for (int i = 0; i < 32; ++i)
        result << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
    return result.str();
}

std::string urlencode(const std::string& s) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (auto c : s) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        } else {
            escaped << '%' << std::setw(2) << int((unsigned char)c);
        }
    }
    return escaped.str();
}
