#include "display_manager.h"

namespace wifiscanner {

void DisplayManager::printBanner() const {
    Serial.println();
    Serial.println(F("╔══════════════════════════════════════════════════╗"));
    Serial.println(F("║      ESP32 Wi-Fi Scanner & Channel Analyzer     ║"));
    Serial.println(F("║                Hunter R. Jamison                 ║"));
    Serial.println(F("╚══════════════════════════════════════════════════╝"));
    Serial.println();
}

void DisplayManager::printSeparator() const {
    Serial.println(F("──────────────────────────────────────────────────"
                     "──────────────────────"));
}

void DisplayManager::printScanResults(
    const std::vector<AccessPointInfo>& networks) const {

    Serial.println();
    printSeparator();
    Serial.printf("  SCAN RESULTS  —  %d network%s found\n",
                  networks.size(),
                  networks.size() == 1 ? "" : "s");
    printSeparator();
    Serial.println();

    Serial.printf("  %-32s  %6s  %3s  %-10s  %-17s  %s\n",
                  "SSID", "RSSI", "CH", "Security", "BSSID", "Quality");
    printSeparator();

    for (const auto& ap : networks) {
        String ssid = ap.ssid.length() > 0 ? ap.ssid : "(hidden)";
        if (ssid.length() > 32) {
            ssid = ssid.substring(0, 29) + "...";
        }

        Serial.printf("  %-32s  %4d  %3d  %-10s  %s  %s %s\n",
                      ssid.c_str(),
                      ap.rssi,
                      ap.channel,
                      ap.authModeToString().c_str(),
                      ap.bssidToString().c_str(),
                      ap.signalQuality().c_str(),
                      rssiBar(ap.rssi).c_str());
    }
    Serial.println();
}

void DisplayManager::printChannelReport(
    const std::array<ChannelStats, ChannelAnalyzer::NUM_24GHZ_CHANNELS>& stats,
    uint8_t recommended_channel) const {

    printSeparator();
    Serial.println(F("  2.4 GHz CHANNEL ANALYSIS"));
    printSeparator();
    Serial.println();

    Serial.printf("  %3s  %8s  %10s  %s\n",
                  "CH", "Networks", "Avg RSSI", "Congestion");
    Serial.printf("  %3s  %8s  %10s  %s\n",
                  "---", "--------", "--------", "--------------------");

    for (uint8_t i = 0; i < 13; ++i) {
        const auto& ch = stats[i];

        int bar_filled = static_cast<int>(ch.congestion_score * 10);
        String bar;
        for (int j = 0; j < 10; ++j) {
            bar += (j < bar_filled) ? "#" : ".";
        }

        String marker = (ch.channel == recommended_channel) ? " << BEST" : "";

        if (ch.network_count > 0) {
            Serial.printf("  %3d  %5d     %5.0f dBm   [%s] %3.0f%%%s\n",
                          ch.channel,
                          ch.network_count,
                          ch.avg_rssi,
                          bar.c_str(),
                          ch.congestion_score * 100.0f,
                          marker.c_str());
        } else {
            Serial.printf("  %3d  %5d     %8s   [%s] %3.0f%%%s\n",
                          ch.channel,
                          ch.network_count,
                          "--",
                          bar.c_str(),
                          ch.congestion_score * 100.0f,
                          marker.c_str());
        }
    }

    Serial.println();
    Serial.printf("  >>> Recommended channel: %d\n\n", recommended_channel);

    Serial.println(F("  Non-overlapping channels (1, 6, 11):"));
    for (uint8_t ch : {1, 6, 11}) {
        const auto& s = stats[ch - 1];
        Serial.printf("    Channel %2d:  %d network%s,  congestion %3.0f%%\n",
                      ch,
                      s.network_count,
                      s.network_count == 1 ? "" : "s",
                      s.congestion_score * 100.0f);
    }
    Serial.println();
}

String DisplayManager::rssiBar(int32_t rssi) const {
    int strength = constrain(map(rssi, -100, -30, 0, 5), 0, 5);
    String bar = "[";
    for (int i = 0; i < 5; ++i) {
        bar += (i < strength) ? "|" : " ";
    }
    bar += "]";
    return bar;
}

}  // namespace wifiscanner
