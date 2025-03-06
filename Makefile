CXX = g++
CXXFLAGS = -Wall -Wextra -I. -I/usr/include/openssl -I/usr/include/jsoncpp -I/usr/include/spdlog -I/usr/include/yaml-cpp
LDFLAGS = -lcurl -lwebsockets -lssl -lcrypto -lpthread -ljsoncpp -lspdlog -lyaml-cpp -lfmt

SRCS = binance_utils.cpp \
       websocket_client.cpp \
       market_analyzer.cpp \
       trading_strategy.cpp \
       risk_manager.cpp \
       order_manager.cpp \
       config.cpp \
       telegram_notifier.cpp \
       data_health_monitor.cpp \
       status_checker.cpp \
       main.cpp

OBJS = $(SRCS:.cpp=.o)
TARGET = trading_bot

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
