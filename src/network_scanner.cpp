#include "network_scanner.h"
#include <algorithm>

namespace wifiscanner {

std::vector<AccessPointInfo> NetworkScanner::scan() {
    Serial.println("Scanning...");

    int count = WiFi.scanNetworks();
    last_scan_count_ = (count > 0) ? static_cast<size_t>(count) : 0;

    std::vector<AccessPointInfo> results;
    results.reserve(last_scan_count_);

    for (int i = 0; i < count; ++i) {
        results.push_back(buildApInfo(i));
    }

    std::sort(results.begin(), results.end(),
              [](const AccessPointInfo& a, const AccessPointInfo& b) {
                  return a.rssi > b.rssi;
              });

    WiFi.scanDelete();
    return results;
}

AccessPointInfo NetworkScanner::buildApInfo(int index) const {
    AccessPointInfo info;
    info.ssid      = WiFi.SSID(index);
    info.rssi      = WiFi.RSSI(index);
    info.channel   = static_cast<uint8_t>(WiFi.channel(index));
    info.auth_mode = WiFi.encryptionType(index);

    const uint8_t* mac = WiFi.BSSID(index);
    if (mac) {
        std::copy(mac, mac + 6, info.bssid.begin());
    } else {
        info.bssid.fill(0);
    }

    return info;
}

}  // namespace wifiscanner
