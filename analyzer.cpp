#include "analyzer.h"
#include <fstream>
#include <string>
#include <unordered_map>
#include <map>
#include <vector>
#include <algorithm>
#include <sstream>


void TripAnalyzer::ingestFile(const std::string& csvPath) {
    // - open file
    std::ifstream file(csvPath);
    if (!file.is_open()) return;

    // - skip header
    std::string line;
    std::getline(file, line);


    while (std::getline(file, line)) {
        // Find positions of commas to extract fields manually
        size_t c1 = line.find(',');
        if (c1 == std::string::npos) continue;

        size_t c2 = line.find(',', c1 + 1);
        if (c2 == std::string::npos) continue;

        size_t c3 = line.find(',', c2 + 1);
        if (c3 == std::string::npos) continue;

        size_t c4 = line.find(',', c3 + 1);
        if (c4 == std::string::npos) continue; // Need at least 5 fields

        std::string zone = line.substr(c1 + 1, c2 - c1 - 1);
        if (zone.empty()) continue;

        std::string dateTime = line.substr(c3 + 1, c4 - c3 - 1);
        size_t space_pos = dateTime.find(' ');
        if (space_pos == std::string::npos) continue;

        size_t hstart = space_pos + 1;
        size_t colon_pos = dateTime.find(':', hstart);
        if (colon_pos == std::string::npos) continue;

        std::string hour_str = dateTime.substr(hstart, colon_pos - hstart);
        int hr = 0;
        try {
            hr = std::stoi(hour_str);
            if (hr < 0 || hr > 23) continue;
        } catch (...) {
            continue; // Cannot parse hour
        }

        // Aggregate counts
        zones[zone]++;
        slots[zone][hr]++;
    }
}

std::vector<ZoneCount> TripAnalyzer::topZones(int k) const {
    // - sort by count desc, zone asc
    std::vector<ZoneCount> result;
    for (const auto& pair : zones) {
        result.push_back({pair.first, pair.second});
    }

    std::sort(result.begin(), result.end(), [](const ZoneCount& a, const ZoneCount& b) {
        if (a.count != b.count) {
            return a.count > b.count; // Higher counts first
        }
        return a.zone < b.zone; // Alphabetical order for ties
    });

    // - return first k
    if (static_cast<size_t>(k) < result.size()) {
        result.resize(k);
    }
    return result;
}

std::vector<SlotCount> TripAnalyzer::topBusySlots(int k) const {
    // - sort by count desc, zone asc, hour asc
    std::vector<SlotCount> result;
    for (const auto& zonePair : slots) {
        for (const auto& hourPair : zonePair.second) {
            result.push_back({zonePair.first, hourPair.first, hourPair.second});
        }
    }

    std::sort(result.begin(), result.end(), [](const SlotCount& a, const SlotCount& b) {
        if (a.count != b.count) {
            return a.count > b.count; // Higher counts first
        }
        if (a.zone != b.zone) {
            return a.zone < b.zone; // Alphabetical order for zones
        }
        return a.hour < b.hour; // Numerical order for hours
    });

    // - return first k
    if (static_cast<size_t>(k) < result.size()) {
        result.resize(k);
    }
    return result;
}
