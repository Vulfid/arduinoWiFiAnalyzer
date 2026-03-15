#pragma once

#include "access_point_info.h"
#include <array>
#include <vector>

namespace wifiscanner {

struct ChannelStats {
    uint8_t channel = 0;
    uint8_t network_count = 0;
    float avg_rssi = 0.0f;
    float congestion_score = 0.0f;
};

class ChannelAnalyzer {
public:
    static constexpr uint8_t NUM_24GHZ_CHANNELS = 14;

    std::array<ChannelStats, NUM_24GHZ_CHANNELS> analyze(
        const std::vector<AccessPointInfo>& networks) const;

    uint8_t recommendChannel(
        const std::array<ChannelStats, NUM_24GHZ_CHANNELS>& stats) const;

    float overlapScore(
        uint8_t channel,
        const std::array<ChannelStats, NUM_24GHZ_CHANNELS>& stats) const;

private:
    // 2.4 GHz channels are 22 MHz wide on 5 MHz center spacing.
    // Channel N overlaps with N-4 through N+4.
    static constexpr int OVERLAP_RANGE = 4;
};

}  // namespace wifiscanner
