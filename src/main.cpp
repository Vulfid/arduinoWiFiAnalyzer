#include <Arduino.h>
#include "network_scanner.h"
#include "channel_analyzer.h"
#include "display_manager.h"
#include "oled_display.h"

using namespace wifiscanner;

static constexpr unsigned long SCAN_INTERVAL_MS = 15000;

NetworkScanner scanner;
ChannelAnalyzer analyzer;
DisplayManager display;
OledDisplay oled;

void setup() {
    Serial.begin(115200);
    delay(1000);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    if (!oled.begin()) {
        Serial.println(F("[WARN] OLED display not found — continuing without it"));
    } else {
        oled.showSplash();
    }

    display.printBanner();
}

void loop() {
    auto networks = scanner.scan();

    if (networks.empty()) {
        Serial.println(F("No networks found.\n"));
    } else {
        display.printScanResults(networks);

        auto stats = analyzer.analyze(networks);
        uint8_t best = analyzer.recommendChannel(stats);
        display.printChannelReport(stats, best);
        oled.update(stats, best, networks.size());
    }

    delay(SCAN_INTERVAL_MS);
}
