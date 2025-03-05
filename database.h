#ifndef DATABASE_H
#define DATABASE_H

#include <pqxx/pqxx>
#include <string>

class Database
{
public:
    static void init(const std::string &connStr);
    static void logTrade(const std::string &symbol,
                         const std::string &side,
                         double price,
                         double quantity);
    static void logError(const std::string &error);

private:
    static std::unique_ptr<pqxx::connection> conn;
};

#endif
