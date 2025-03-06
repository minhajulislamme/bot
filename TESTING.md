# Binance Trading Bot - Testing Documentation

This document describes the testing infrastructure for the Binance Trading Bot.

## System Test

The system test (`./run_system_test.sh`) provides comprehensive testing of all critical components of the bot. It verifies:

- **Config Loading**: Ensures configuration files are properly loaded
- **Telegram Integration**: Verifies notification system works correctly
- **WebSocket Communication**: Tests real-time data feeds
- **Market Data Reception**: Confirms price data is being received
- **Market Analysis**: Validates trend detection and analysis
- **Account Balance**: Checks Binance API connection for balance retrieval
- **Order Management**: Tests order placement functionality (simulated)
- **Risk Management**: Verifies position sizing and risk calculations
- **Data Health Monitoring**: Tests data freshness monitoring
- **Status Checking**: Validates status reporting functionality

### Running the System Test

```bash
./run_system_test.sh
```

Test results are displayed in the console and also sent to Telegram. A detailed log is saved to `logs/system_test.log`.

## Quick Health Check

For a faster verification of the bot's core functionality, run the health check script:

```bash
./check_system_health.sh
```

This performs basic checks:

- Verifies if the bot process is running
- Checks recent log entries for errors
- Tests Binance API connectivity
- Verifies WebSocket server accessibility

## Automated Testing

The bot also includes automated monitoring:

- Data health monitoring (checks every 6 hours)
- Status reports (every 5 hours)
- Continuous price feed verification

## When to Run Tests

- After installation: Run full system test to verify setup
- Before trading: Run quick health check
- After API changes: Run full system test
- When experiencing issues: Run health check for diagnostics

## Troubleshooting

If tests fail:

1. Check your API keys in the configuration
2. Verify network connectivity to Binance
3. Ensure all dependencies are properly installed
4. Check the log files for detailed error messages
