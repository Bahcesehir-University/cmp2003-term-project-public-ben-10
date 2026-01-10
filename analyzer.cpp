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
        size_t firstComma = line.find(',');
        if (firstComma >= line.size()) continue;

        size_t secondComma = line.find(',', firstComma + 1);
        if (secondComma >= line.size()) continue;

        size_t thirdComma = line.find(',', secondComma + 1);
        if (thirdComma >= line.size()) continue;

        size_t fourthComma = line.find(',', thirdComma + 1);
        if (fourthComma >= line.size()) continue; // Need at least 5 fields

        std::string zone = line.substr(firstComma + 1, secondComma - firstComma - 1);
        if (zone.empty()) continue;

        std::string dateTime = line.substr(thirdComma + 1, fourthComma - thirdComma - 1);
        size_t spaceIndex = dateTime.find(' ');
        if  (spaceIndex >=dateTime.size())  continue;

        size_t hStartIndex = spaceIndex + 1;
        size_t minuteStartIndex = dateTime.find(':', hStartIndex);
        if (minuteStartIndex >= dateTime.size()) continue;

        std::string hour_str = dateTime.substr(hStartIndex, minuteStartIndex - hStartIndex);
        int hr = 0;
        try {
            hr = std::stoi(hour_str);
            if (hr < 0 || hr > 23) continue;
        } catch (...) {
            continue; 
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
