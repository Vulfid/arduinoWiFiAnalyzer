#include "channel_analyzer.h"
#include <algorithm>
#include <cmath>

namespace wifiscanner {

std::array<ChannelStats, ChannelAnalyzer::NUM_24GHZ_CHANNELS>
ChannelAnalyzer::analyze(const std::vector<AccessPointInfo>& networks) const {

    std::array<ChannelStats, NUM_24GHZ_CHANNELS> stats{};
    std::array<float, NUM_24GHZ_CHANNELS> rssi_sum{};

    for (uint8_t i = 0; i < NUM_24GHZ_CHANNELS; ++i) {
        stats[i].channel = i + 1;
    }

    for (const auto& ap : networks) {
        if (ap.channel < 1 || ap.channel > NUM_24GHZ_CHANNELS) continue;

        uint8_t idx = ap.channel - 1;
        stats[idx].network_count++;
        rssi_sum[idx] += static_cast<float>(ap.rssi);
    }

    for (uint8_t i = 0; i < NUM_24GHZ_CHANNELS; ++i) {
        if (stats[i].network_count > 0) {
            stats[i].avg_rssi = rssi_sum[i] / stats[i].network_count;
        }
        stats[i].congestion_score = overlapScore(i + 1, stats);
    }

    return stats;
}

uint8_t ChannelAnalyzer::recommendChannel(
    const std::array<ChannelStats, NUM_24GHZ_CHANNELS>& stats) const {

    // Only consider the three non-overlapping 2.4 GHz channels
    static constexpr uint8_t candidates[] = {1, 6, 11};

    uint8_t best = 1;
    float lowest = 2.0f;

    for (uint8_t ch : candidates) {
        float score = stats[ch - 1].congestion_score;
        if (score < lowest) {
            lowest = score;
            best = ch;
        }
    }

    return best;
}

float ChannelAnalyzer::overlapScore(
    uint8_t channel,
    const std::array<ChannelStats, NUM_24GHZ_CHANNELS>& stats) const {

    float score = 0.0f;

    for (int offset = -OVERLAP_RANGE; offset <= OVERLAP_RANGE; ++offset) {
        int neighbor = static_cast<int>(channel) + offset;
        if (neighbor < 1 || neighbor > NUM_24GHZ_CHANNELS) continue;

        uint8_t count = stats[neighbor - 1].network_count;
        if (count == 0) continue;

        // Weight decreases with channel distance — co-channel is worst,
        // adjacent-4 is minimal interference.
        float weight = 1.0f - (std::abs(offset) /
                               static_cast<float>(OVERLAP_RANGE + 1));

        // Nearby strong APs cause more CCI than distant weak ones
        float rssi_factor = 1.0f;
        if (stats[neighbor - 1].avg_rssi > -60.0f) {
            rssi_factor = 1.5f;
        } else if (stats[neighbor - 1].avg_rssi < -80.0f) {
            rssi_factor = 0.5f;
        }

        score += count * weight * rssi_factor;
    }

    // Normalize so 10+ weighted neighbor-networks = fully congested
    return std::min(score / 10.0f, 1.0f);
}

}  // namespace wifiscanner
