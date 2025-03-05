#include <gtest/gtest.h>
#include "../trading_strategy.h"

TEST(TradingStrategyTest, BasicSignalGeneration)
{
    TradingStrategy strategy;

    // Test with uptrend data
    for (int i = 0; i < 20; i++)
    {
        strategy.addIndicatorData(100 + i, 1000);
    }

    auto signal = strategy.analyze(120, 1000);
    EXPECT_EQ(signal.type, TradingSignal::BUY);
}
