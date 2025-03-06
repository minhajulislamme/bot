#include "websocket_client.h"
#include <iostream>
#include <libwebsockets.h>
#include <string>
#include <chrono>
#include <thread>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <algorithm>
#include <cctype>
#include <unistd.h>
#include <spdlog/spdlog.h>
#include "data_health_monitor.h"

double currentPrice = 0.0;

// List of fallback WebSocket servers to try
const std::vector<std::string> WS_SERVERS = {
    "stream.binancefuture.com", // Primary server
    "fstream.binancefuture.com" // Fallback server
};

// Connection state tracking
static struct connection_data
{
    bool interrupted;
    bool should_reconnect;
    std::string symbol;
    std::string current_server;
    int server_index;
    int connection_attempts;
} conn_data;

class WebSocketException : public std::runtime_error
{
public:
    explicit WebSocketException(const std::string &msg) : std::runtime_error(msg) {}
};

static int websocketCallback(struct lws *wsi, enum lws_callback_reasons reason,
                             void *user, void *in, size_t len)
{
    (void)wsi;  // Mark as unused to suppress warning
    (void)user; // Mark as unused to suppress warning

    switch (reason)
    {
    case LWS_CALLBACK_CLIENT_ESTABLISHED:
        std::cout << "WebSocket connection established!" << std::endl;
        conn_data.should_reconnect = false;
        break;

    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
        std::cerr << "WebSocket connection error" << std::endl;
        conn_data.interrupted = true;
        conn_data.should_reconnect = true;
        break;

    case LWS_CALLBACK_CLIENT_RECEIVE:
    {
        std::string msg((char *)in, len);
        auto pricePos = msg.find("\"p\":\"");
        if (pricePos != std::string::npos)
        {
            auto start = pricePos + 5;
            auto end = msg.find("\"", start);
            std::string priceStr = msg.substr(start, end - start);
            currentPrice = std::stod(priceStr);

            // Record data point for health monitoring
            DataHealthMonitor::recordDataPoint(conn_data.symbol, currentPrice);

            std::cout << "Current Price: " << currentPrice << std::endl;
        }
    }
    break;

    case LWS_CALLBACK_CLOSED:
        std::cout << "WebSocket connection closed" << std::endl;
        conn_data.interrupted = true;
        conn_data.should_reconnect = true;
        break;

    default:
        break;
    }
    return 0;
}

// Define protocol list with all required fields
static const struct lws_protocols protocols[] = {
    {
        "ws",              // name
        websocketCallback, // callback
        0,                 // per_session_data_size
        65536,             // rx_buffer_size
        0,                 // id
        NULL,              // user
        0                  // tx_packet_size
    },
    {NULL, NULL, 0, 0, 0, NULL, 0} // End of list with all fields
};

struct lws *connect_websocket(struct lws_context *context, const std::string &symbol)
{
    struct lws_client_connect_info ccinfo = {};

    // For Binance Futures testnet
    std::string lowercaseSymbol = symbol;
    std::transform(lowercaseSymbol.begin(), lowercaseSymbol.end(), lowercaseSymbol.begin(),
                   [](unsigned char c)
                   { return std::tolower(c); });
    std::string path = "/ws/" + lowercaseSymbol + "@trade";

    // Use the current server from the connection data
    ccinfo.context = context;
    ccinfo.address = conn_data.current_server.c_str();
    ccinfo.port = 443;
    ccinfo.path = path.c_str();
    ccinfo.host = ccinfo.address;
    ccinfo.origin = ccinfo.address;
    ccinfo.protocol = protocols[0].name;
    ccinfo.ietf_version_or_minus_one = -1;
    ccinfo.userdata = nullptr;
    ccinfo.ssl_connection = LCCSCF_USE_SSL;

    spdlog::info("Connecting to WebSocket server: {}, path: {}", conn_data.current_server, path);
    return lws_client_connect_via_info(&ccinfo);
}

void runWebSocketClient(const std::string &symbol)
{
    try
    {
        // Initialize OpenSSL
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();

        conn_data.symbol = symbol;
        conn_data.interrupted = false;
        conn_data.should_reconnect = false;
        conn_data.server_index = 0;
        conn_data.current_server = WS_SERVERS[0];
        conn_data.connection_attempts = 0;

        struct lws_context_creation_info info = {};
        struct lws_context *context;

        info.port = CONTEXT_PORT_NO_LISTEN;
        info.protocols = protocols;
        info.gid = -1;
        info.uid = -1;
        info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;

        // Try standard locations for CA certificates
        const char *ca_filepath = "/etc/ssl/certs/ca-certificates.crt";
        if (access(ca_filepath, R_OK) != -1)
        {
            info.ssl_ca_filepath = ca_filepath;
        }
        else
        {
            // Try alternative locations
            ca_filepath = "/etc/pki/tls/certs/ca-bundle.crt";
            if (access(ca_filepath, R_OK) != -1)
            {
                info.ssl_ca_filepath = ca_filepath;
            }
        }

        context = lws_create_context(&info);
        if (!context)
        {
            throw WebSocketException("Failed to create websocket context");
        }

        struct lws *wsi = connect_websocket(context, symbol);
        if (!wsi)
        {
            std::cerr << "Failed to create websocket connection" << std::endl;
            conn_data.should_reconnect = true;
            conn_data.interrupted = true;
        }

        // Main event loop with reconnection logic
        while (true)
        {
            lws_service(context, 100);

            if (conn_data.interrupted)
            {
                if (conn_data.should_reconnect)
                {
                    conn_data.connection_attempts++;

                    // If we've tried the current server too many times, try the next one
                    if (conn_data.connection_attempts >= 3)
                    {
                        conn_data.server_index = (conn_data.server_index + 1) % WS_SERVERS.size();
                        conn_data.current_server = WS_SERVERS[conn_data.server_index];
                        conn_data.connection_attempts = 0;
                        spdlog::warn("Switching to WebSocket server: {}", conn_data.current_server);
                    }

                    std::cout << "Attempting to reconnect WebSocket... (attempt "
                              << conn_data.connection_attempts + 1 << " to "
                              << conn_data.current_server << ")" << std::endl;

                    std::this_thread::sleep_for(std::chrono::seconds(5)); // Wait before reconnecting

                    wsi = connect_websocket(context, symbol);
                    if (wsi)
                    {
                        conn_data.interrupted = false;
                        spdlog::info("Successfully reconnected to {}", conn_data.current_server);
                    }
                    else
                    {
                        std::cerr << "Reconnection attempt failed" << std::endl;
                    }
                }
                else
                {
                    break; // Exit if we don't want to reconnect
                }
            }
        }

        lws_context_destroy(context);

        // Cleanup OpenSSL
        EVP_cleanup();
        ERR_free_strings();
    }
    catch (const std::exception &e)
    {
        spdlog::error("WebSocket error for {}: {}", symbol, e.what());
        throw;
    }
}
