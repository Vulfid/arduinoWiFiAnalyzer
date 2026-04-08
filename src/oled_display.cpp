#include "oled_display.h"
#include <Wire.h>

namespace wifiscanner {

static constexpr uint8_t HEADER_HEIGHT    = 12;
static constexpr uint8_t DIVIDER_Y        = 13;
static constexpr uint8_t CHART_TOP        = 15;
static constexpr uint8_t LABEL_HEIGHT     = 8;
static constexpr uint8_t CHART_BOTTOM     = OledDisplay::SCREEN_HEIGHT - LABEL_HEIGHT - 1;
static constexpr uint8_t CHART_HEIGHT     = CHART_BOTTOM - CHART_TOP;
static constexpr uint8_t NUM_BARS         = 13;
static constexpr uint8_t BAR_SPACING      = 1;
static constexpr uint8_t BAR_WIDTH        =
    (OledDisplay::SCREEN_WIDTH - (NUM_BARS - 1) * BAR_SPACING) / NUM_BARS;

bool OledDisplay::begin() {
    Wire.begin(8, 9);
    if (!oled_.begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS)) {
        return false;
    }
    oled_.clearDisplay();
    oled_.display();
    return true;
}

void OledDisplay::showSplash() {
    oled_.clearDisplay();
    oled_.setTextSize(1);
    oled_.setTextColor(SSD1306_WHITE);
    oled_.setCursor(16, 20);
    oled_.print(F("WiFi Scanner"));
    oled_.setCursor(16, 36);
    oled_.print(F("Scanning..."));
    oled_.display();
}

void OledDisplay::update(
    const std::array<ChannelStats, ChannelAnalyzer::NUM_24GHZ_CHANNELS>& stats,
    uint8_t recommended_channel,
    size_t network_count) {

    oled_.clearDisplay();
    drawHeader(network_count, recommended_channel);
    oled_.drawLine(0, DIVIDER_Y, SCREEN_WIDTH - 1, DIVIDER_Y, SSD1306_WHITE);
    drawChannelChart(stats, recommended_channel);
    drawChannelLabels();
    oled_.display();
}

void OledDisplay::drawHeader(size_t network_count, uint8_t recommended_channel) {
    oled_.setTextSize(1);
    oled_.setTextColor(SSD1306_WHITE);

    oled_.setCursor(0, 2);
    oled_.print(network_count);
    oled_.print(F(" nets"));

    oled_.setCursor(72, 2);
    oled_.print(F("Best:"));
    oled_.print(recommended_channel);
}

void OledDisplay::drawChannelChart(
    const std::array<ChannelStats, ChannelAnalyzer::NUM_24GHZ_CHANNELS>& stats,
    uint8_t recommended_channel) {

    for (uint8_t i = 0; i < NUM_BARS; ++i) {
        float score = stats[i].congestion_score;
        uint8_t bar_h = static_cast<uint8_t>(score * CHART_HEIGHT);
        if (stats[i].network_count > 0 && bar_h < 2) {
            bar_h = 2;
        }

        uint8_t x = i * (BAR_WIDTH + BAR_SPACING);
        uint8_t y = CHART_BOTTOM - bar_h;

        if (stats[i].channel == recommended_channel) {
            if (bar_h > 0) {
                oled_.fillRect(x, y, BAR_WIDTH, bar_h, SSD1306_WHITE);
            }
            uint8_t cx = x + BAR_WIDTH / 2;
            uint8_t cy = CHART_TOP + 4;
            // Clear interior so heart is visible against a full bar
            static const int8_t fill[][2] = {
                {-1,-2},{0,-2},{1,-2},           {3,-2},{4,-2},{5,-2},
                {-2, -1},{-1,-1},{0,-1},{1,-1},{2,-1},{3,-1},{4,-1},{5,-1},{6,-1},
                {-2, 0},{-1, 0},{0, 0},{1, 0},{2, 0},{3, 0},{4, 0},{5, 0},{6, 0},
                {-1, 1},{0, 1},{1, 1},{2, 1},{3, 1},{4, 1},{5, 1},
                { 0, 2},{1, 2},{2, 2},{3, 2},{4, 2},
                { 1, 3},{2, 3},{3, 3},
                { 2, 4},
            };
            for (auto& p : fill) {
                oled_.drawPixel(cx + p[0] - 2, cy + p[1], SSD1306_BLACK);
            }
            static const int8_t heart[][2] = {
                {-1,-3},{0,-3},{1,-3},           {3,-3},{4,-3},{5,-3},
                {-2,-2},                  {2,-2},                  {6,-2},
                {-3,-1},                                           {7,-1},
                {-3, 0},                                           {7, 0},
                {-2, 1},                                    {6, 1},
                {-1, 2},                             {5, 2},
                { 0, 3},                      {4, 3},
                { 1, 4},               {3, 4},
                { 2, 5},
            };
            for (auto& p : heart) {
                oled_.drawPixel(cx + p[0] - 2, cy + p[1], SSD1306_WHITE);
            }
        } else if (bar_h > 0) {
            oled_.fillRect(x, y, BAR_WIDTH, bar_h, SSD1306_WHITE);
        }
    }
}

void OledDisplay::drawChannelLabels() {
    oled_.setTextSize(1);
    oled_.setTextColor(SSD1306_WHITE);

    // Label the three non-overlapping channels plus endpoints
    static constexpr uint8_t labels[] = {1, 6, 11};
    for (uint8_t ch : labels) {
        uint8_t idx = ch - 1;
        uint8_t x = idx * (BAR_WIDTH + BAR_SPACING);
        oled_.setCursor(x, SCREEN_HEIGHT - LABEL_HEIGHT);
        oled_.print(ch);
    }
}

}  // namespace wifiscanner
