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
       main.cpp

OBJS = $(SRCS:.cpp=.o)
TARGET = trading_bot

# Add production flags
PROD_FLAGS = -O3 -DNDEBUG -march=native

.PHONY: all clean debug test install

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

debug: CXXFLAGS += -g -DDEBUG
debug: all

test: $(TEST_OBJS)
	$(CXX) $(TEST_OBJS) -o run_tests $(LDFLAGS) -lgtest -lgtest_main

install: $(TARGET)
	install -d $(DESTDIR)/usr/local/bin
	install $(TARGET) $(DESTDIR)/usr/local/bin/

clean:
	rm -f $(OBJS) $(TARGET)
