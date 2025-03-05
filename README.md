# Binance Trading Bot

This is a simple algorithmic trading bot that connects to the Binance Futures Testnet. It uses WebSockets to receive real-time price updates and executes trades based on simple price movement strategies.

## Features

- Real-time price monitoring using WebSockets
- Simple trading strategy (buy when price increases by 0.1%, sell when price decreases by 0.1%)
- Connection to Binance Futures Testnet
- Automatic reconnection on WebSocket disconnect

## Setup

1. Replace the API_KEY and API_SECRET in main.cpp with your own Binance Futures Testnet API credentials
2. Build the project using the provided Makefile:

```bash
make
```

3. Run the trading bot:

```bash
./trading_bot
```

## Configuration

The bot is currently configured to trade BTCUSDT with a fixed quantity of 0.001 BTC. You can modify these parameters in the main.cpp file.

## Dependencies

- libcurl
- libwebsockets
- OpenSSL
- pthreads

## Disclaimer

This bot is for educational purposes only. Use at your own risk. The trading strategy is extremely simple and not recommended for real trading.
