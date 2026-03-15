#pragma once

#include "access_point_info.h"
#include <vector>

namespace wifiscanner {

class NetworkScanner {
public:
    NetworkScanner() = default;

    std::vector<AccessPointInfo> scan();
    size_t lastScanCount() const { return last_scan_count_; }

private:
    size_t last_scan_count_ = 0;

    AccessPointInfo buildApInfo(int index) const;
};

}  // namespace wifiscanner
