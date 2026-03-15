#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <array>

namespace wifiscanner {

struct AccessPointInfo {
    String ssid;
    std::array<uint8_t, 6> bssid{};
    int32_t rssi = 0;
    uint8_t channel = 0;
    wifi_auth_mode_t auth_mode = WIFI_AUTH_OPEN;

    String bssidToString() const;
    String authModeToString() const;
    String signalQuality() const;
};

}  // namespace wifiscanner
