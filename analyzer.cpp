
#include "analyzer.h"
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <sstream>

void TripAnalyzer::ingestFile(const std::string& csvPath) {
    std::ifstream file(csvPath);
    if (!file.is_open()) return;

    std::string oneLine;

    if (getline(file, oneLine)) {

    }

    while (getline(file, oneLine)) {


        size_t firstComma = oneLine.find(',');
        if (firstComma >= oneLine.size()) continue;

        size_t secondComma = oneLine.find(',', firstComma + 1);
        if (secondComma >= oneLine.size()) continue;

        std::string zone = oneLine.substr(firstComma + 1, secondComma - firstComma - 1);
        if (zone.empty()) continue;

        std::string dateTime = oneLine.substr(secondComma + 1);
        size_t spaceIndex = dateTime.find(' ');
        if (spaceIndex >= dateTime.size()) continue;

        size_t hourIndex = spaceIndex + 1;
        size_t minuteStartIndex = dateTime.find(':', hourIndex);
        if (minuteStartIndex >= dateTime.size()) continue;

        int hourInt = 0;
        bool validHour = true;
        for (size_t i = hourIndex; i < minuteStartIndex; ++i) {
            char currentChar = dateTime[i];
            if (currentChar < '0' || currentChar > '9') {
                validHour = false;
                break;
            }
            hourInt = hourInt * 10 + (currentChar - '0');
        }

        if (!validHour || hourInt < 0 || hourInt > 23) continue;

        zones[zone]++;
        slots[zone][hourInt]++;
    }
}

std::vector<ZoneCount> TripAnalyzer::topZones(int k) const {
    std::vector<ZoneCount> result;

    if (k < 20 && zones.size() > static_cast<size_t>(k)) {
        for (const auto& [zone, count] : zones) {
            result.push_back({zone, count});
        }
        std::partial_sort(result.begin(), result.begin() + std::min(static_cast<size_t>(k), result.size()), result.end(),
            [](const ZoneCount& a, const ZoneCount& b) {
                if (a.count != b.count) return a.count > b.count;
                return a.zone < b.zone;
            });
        if (static_cast<size_t>(k) < result.size()) result.resize(k);
        return result;
    }

    result.reserve(zones.size());
    for (const auto& [zone, count] : zones) {
        result.push_back({zone, count});
    }

    std::sort(result.begin(), result.end(), [](const ZoneCount& a, const ZoneCount& b) {
        if (a.count != b.count) return a.count > b.count;
        return a.zone < b.zone;
    });

    if (static_cast<size_t>(k) < result.size()) {
        result.resize(k);
    }
    return result;
}

std::vector<SlotCount> TripAnalyzer::topBusySlots(int k) const {
    std::vector<SlotCount> result;

    for (const auto& [zone, hours] : slots) {
        for (const auto& [hour, count] : hours) {
            result.push_back({zone, hour, count});
        }
    }

    std::sort(result.begin(), result.end(), [](const SlotCount& a, const SlotCount& b) {
        if (a.count != b.count) return a.count > b.count;
        if (a.zone != b.zone) return a.zone < b.zone;
        return a.hour < b.hour;
    });

    if (static_cast<size_t>(k) < result.size()) result.resize(k);
    return result;
}
