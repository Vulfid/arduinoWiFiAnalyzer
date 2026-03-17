# ESP32 Wi-Fi Scanner & Channel Analyzer

A handheld Wi-Fi network scanner and 2.4 GHz channel analyzer built on the ESP32 microcontroller. Scans the RF environment, reports every visible access point, and recommends the least-congested non-overlapping channel — the same core analysis performed by enterprise site survey tools, running on a $10 dev board. Includes a 128x64 OLED display that renders a real-time channel congestion bar chart.

<!-- TODO: Add a photo of the physical setup here -->
<!-- ![Hardware Setup](docs/hardware_photo.jpg) -->

---

## Why This Exists

I work with 802.11 access points professionally — validating firmware across Wi-Fi 6/6E/7 hardware, writing automated test suites, and debugging RF issues in shielded chambers. This project takes that domain knowledge and puts it into a self-contained embedded application, demonstrating wireless expertise implemented in C++. Also, just cause I thought it'd be neat.

---

## Features

- **Network Discovery** — Scans and lists every visible SSID with RSSI, channel, BSSID, encryption type, and signal quality rating
- **Channel Analysis** — Per-channel network count, average signal strength, and congestion scoring across all 14 channels (2.4 GHz)
- **Overlap-Aware Scoring** — Congestion calculation accounts for 802.11 channel overlap (22 MHz channel width on 5 MHz spacing), not just co-channel networks
- **Channel Recommendation** — Recommends the best non-overlapping channel (1, 6, or 11) based on weighted congestion analysis
- **Signal-Weighted Interference** — Strong nearby APs are weighted more heavily than weak distant ones, reflecting real-world co-channel interference behavior
- **OLED Visualization** — Real-time 2.4 GHz channel congestion bar chart on a 128x64 SSD1306 display, with the recommended channel highlighted
- **Formatted Serial Output** — Structured, readable reports over the serial monitor

---

## Architecture

```
┌─────────────────┐
│  NetworkScanner  │──── ESP32 WiFi.scanNetworks()
│                  │     Returns vector<AccessPointInfo>
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ ChannelAnalyzer  │──── Per-channel stats, overlap scoring,
│                  │     best channel recommendation
└────────┬────────┘
         │
    ┌────┴────┐
    ▼         ▼
┌──────────┐ ┌──────────┐
│ Display  │ │   OLED   │
│ Manager  │ │  Display │
│ (serial) │ │ (I2C)    │
└──────────┘ └──────────┘
```

| Class | Responsibility |
|-------|---------------|
| `AccessPointInfo` | Data struct for a single discovered AP (SSID, BSSID, RSSI, channel, auth) with formatting helpers |
| `NetworkScanner` | Drives ESP32 Wi-Fi scan, builds and sorts result set |
| `ChannelAnalyzer` | Aggregates scan data per channel, computes overlap-aware congestion scores, recommends best channel |
| `DisplayManager` | Renders scan results and channel reports to the serial monitor |
| `OledDisplay` | Drives the SSD1306 OLED over I2C — draws channel congestion bar chart and scan summary |

---

## Hardware

| Component | Purpose |
|-----------|---------|
| ESP32-S3 dev board | Wi-Fi scanning and computation |
| 0.96" OLED display (SSD1306, 128x64, I2C) | Real-time channel congestion bar chart |
| USB-C cable (data-capable) | Power and serial communication |
| 4 jumper wires (F-F) | I2C connection between ESP32 and OLED |

Total cost: ~$15-20

### OLED Wiring

Connect four pins from the SSD1306 OLED module to the ESP32-S3:

| OLED Pin | ESP32-S3 Pin | Wire Color (suggested) |
|----------|-------------|----------------------|
| VCC | 3.3V | Red |
| GND | GND | Black |
| SDA | GPIO 8 | Blue |
| SCL | GPIO 9 | Yellow |

<!-- TODO: Add a photo of the wired OLED display -->
<!-- ![OLED Wiring](docs/oled_wiring.jpg) -->

---

## Getting Started

### Prerequisites

- [PlatformIO](https://platformio.org/) (CLI or VS Code / Cursor extension)
- USB drivers for your ESP32 board (CP2102 or CH340, depending on the board)

### Build & Flash

```bash
# Clone the repo
git clone https://github.com/YOUR_USERNAME/esp32-wifi-scanner.git
cd esp32-wifi-scanner

# Build
pio run

# Flash to ESP32 (connect via USB first)
pio run --target upload

# Open serial monitor to see output
pio device monitor
```

### Quick Start (Arduino IDE)

If you prefer Arduino IDE over PlatformIO:
1. Install the ESP32 board package via Board Manager
2. Copy the contents of `src/` and `include/` into a single sketch folder
3. Select board: "ESP32 Dev Module"
4. Set baud rate to 115200
5. Upload and open Serial Monitor

---

## Sample Output

```
╔══════════════════════════════════════════════════╗
║      ESP32 Wi-Fi Scanner & Channel Analyzer     ║
║                Hunter R. Jamison                 ║
╚══════════════════════════════════════════════════╝

Scanning...

────────────────────────────────────────────────────────────────────────
  SCAN RESULTS  —  8 networks found
────────────────────────────────────────────────────────────────────────

  SSID                              RSSI   CH  Security    BSSID              Quality
────────────────────────────────────────────────────────────────────────
  MyNetwork-5G                      -42     6  WPA2/WPA3   A4:CF:12:8B:3E:01  Excellent [|||||]
  Neighbors_WiFi                    -58     1  WPA2        B8:27:EB:4A:D2:F3  Good      [||||]
  CoffeeShop                        -65    11  WPA2        DC:A6:32:10:CC:89  Fair      [||| ]
  DIRECT-printer                    -71     6  WPA2        00:1A:2B:3C:4D:5E  Fair      [|| ]
  xfinitywifi                       -78     1  OPEN        F4:F5:D8:AA:BB:CC  Weak      [|  ]
  ...

────────────────────────────────────────────────────────────────────────
  2.4 GHz CHANNEL ANALYSIS
────────────────────────────────────────────────────────────────────────

   CH  Networks   Avg RSSI  Congestion
  ---  --------   --------  --------------------
    1      2       -68 dBm   [####......] 40%
    6      3       -59 dBm   [######....] 60%
   11      1       -65 dBm   [##........] 20% << BEST

  >>> Recommended channel: 11

  Non-overlapping channels (1, 6, 11):
    Channel  1:  2 networks,  congestion 40%
    Channel  6:  3 networks,  congestion 60%
    Channel 11:  1 network,   congestion 20%
```

---

## How 802.11 Scanning Works

When an 802.11 station (like the ESP32) performs an **active scan**, it cycles through each channel and sends Probe Request frames, then listens for Probe Responses from access points. The ESP32's `WiFi.scanNetworks()` abstracts this process and returns metadata for each discovered BSS.

**Channel overlap** is the critical nuance in 2.4 GHz analysis. Each channel occupies approximately 22 MHz of bandwidth, but channel centers are spaced only 5 MHz apart. This means channel 6 doesn't just compete with other channel-6 networks — it receives interference from any network on channels 2 through 10. Only channels 1, 6, and 11 are fully non-overlapping in the North American channel plan.

This analyzer accounts for that overlap by computing a weighted congestion score where co-channel networks contribute full weight and adjacent-channel networks contribute proportionally less, with signal strength factored in to reflect real-world interference impact.

---

## Project Structure

```
├── platformio.ini              PlatformIO build configuration
├── include/
│   ├── access_point_info.h     AP data struct and formatting
│   ├── network_scanner.h       Wi-Fi scan driver
│   ├── channel_analyzer.h      Channel stats and congestion scoring
│   ├── display_manager.h       Serial output formatting
│   └── oled_display.h          SSD1306 OLED display driver
├── src/
│   ├── main.cpp                Entry point (setup/loop)
│   ├── access_point_info.cpp   AP struct method implementations
│   ├── network_scanner.cpp     Scan logic and result building
│   ├── channel_analyzer.cpp    Overlap-aware analysis engine
│   ├── display_manager.cpp     Formatted report rendering
│   └── oled_display.cpp        OLED bar chart and summary rendering
├── LICENSE                     MIT
└── README.md
```

---

## Future Enhancements

- [ ] Web UI served directly from the ESP32 (connect to device IP in a browser)
- [ ] JSON API endpoint for scan data consumption by external tools
- [x] OLED display output (SSD1306 via I2C) — channel congestion bar chart
- [ ] 5 GHz band scanning and per-band analysis
- [ ] Continuous mode with RSSI trending over time
- [ ] Hidden SSID detection (beacon frames with empty SSID field)
- [ ] Signal quality grading factoring in noise floor estimate

---

## License

MIT — see [LICENSE](LICENSE).
