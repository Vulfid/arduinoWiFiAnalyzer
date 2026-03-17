#pragma once

#include "channel_analyzer.h"
#include <Adafruit_SSD1306.h>
#include <array>

namespace wifiscanner {

class OledDisplay {
public:
    static constexpr uint8_t SCREEN_WIDTH  = 128;
    static constexpr uint8_t SCREEN_HEIGHT = 64;
    static constexpr uint8_t I2C_ADDRESS   = 0x3C;

    bool begin();

    void update(
        const std::array<ChannelStats, ChannelAnalyzer::NUM_24GHZ_CHANNELS>& stats,
        uint8_t recommended_channel,
        size_t network_count);

    void showSplash();

private:
    Adafruit_SSD1306 oled_{SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1};

    void drawHeader(size_t network_count, uint8_t recommended_channel);
    void drawChannelChart(
        const std::array<ChannelStats, ChannelAnalyzer::NUM_24GHZ_CHANNELS>& stats,
        uint8_t recommended_channel);
    void drawChannelLabels();
};

}  // namespace wifiscanner
