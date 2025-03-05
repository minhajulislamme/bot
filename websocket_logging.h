#ifndef WEBSOCKET_LOGGING_H
#define WEBSOCKET_LOGGING_H

#include <libwebsockets.h>
#include <iostream>

static void lws_log_emit_function(int level, const char *line)
{
    (void)level; // Unused parameter
    std::cerr << "WS: " << line << std::endl;
}

inline void setup_lws_logging(bool debug = false)
{
    int logs = LLL_ERR | LLL_WARN;
    if (debug)
    {
        logs |= LLL_DEBUG | LLL_INFO;
    }
    lws_set_log_level(logs, lws_log_emit_function);
}

#endif // WEBSOCKET_LOGGING_H
