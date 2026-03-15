#pragma once

#include "access_point_info.h"
#include "channel_analyzer.h"
#include <array>
#include <vector>

namespace wifiscanner {

class DisplayManager {
public:
    void printBanner() const;
    void printScanResults(const std::vector<AccessPointInfo>& networks) const;
    void printChannelReport(
        const std::array<ChannelStats, ChannelAnalyzer::NUM_24GHZ_CHANNELS>& stats,
        uint8_t recommended_channel) const;
    void printSeparator() const;

private:
    String rssiBar(int32_t rssi) const;
};

}  // namespace wifiscanner
