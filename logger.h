#ifndef LOGGER_H
#define LOGGER_H

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>

class Logger
{
public:
    static void init(const std::string &logPath = "logs/trading_bot.log")
    {
        try
        {
            auto logger = spdlog::rotating_logger_mt("trading_bot",
                                                     logPath, 1024 * 1024 * 5, 3);
            spdlog::set_default_logger(logger);
            spdlog::set_level(spdlog::level::info);
            spdlog::flush_on(spdlog::level::warn);
        }
        catch (const spdlog::spdlog_ex &ex)
        {
            std::cerr << "Logger initialization failed: " << ex.what() << std::endl;
        }
    }
};

#endif
