CXX = g++
CXXFLAGS = -Wall -Wextra -I. -I/usr/include/openssl -I/usr/include/jsoncpp
LDFLAGS = -lcurl -lwebsockets -lssl -lcrypto -lpthread -ljsoncpp

SRCS = binance_utils.cpp \
       websocket_client.cpp \
       market_analyzer.cpp \
       trading_strategy.cpp \
       risk_manager.cpp \
       order_manager.cpp \
       main.cpp

OBJS = $(SRCS:.cpp=.o)
TARGET = trading_bot

.PHONY: all clean debug

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

debug: CXXFLAGS += -g -DDEBUG
debug: all

clean:
	rm -f $(OBJS) $(TARGET)
