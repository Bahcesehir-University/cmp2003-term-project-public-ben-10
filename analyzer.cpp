#include "analyzer.h"
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <algorithm>


static std::unordered_map<std::string, long long> zoneCounts;
static std::unordered_map<std::string, std::unordered_map<int, long long>> slotCounts;

void TripAnalyzer::ingestFile(const std::string& csvPath) {
    std::ifstream file(csvPath);
    if (!file.is_open()) {
        return;
    }

    std::string line;
    std::getline(file, line);

    
    zoneCounts.clear();
    slotCounts.clear();

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string tripId, zoneId, timeStr;

        if (!std::getline(iss, tripId, ',') || !std::getline(iss, zoneId, ',')) {
            continue;
        }

        if (!std::getline(iss, timeStr)) {
            continue; 
        }

       
        if (zoneId.empty()) {
            continue;
        }

        int hour = -1;
        size_t spacePos = timeStr.find(' ');
        if (spacePos != std::string::npos && timeStr.length() >= spacePos + 6) {

            std::string timePart = timeStr.substr(spacePos + 1);
            if (timePart.length() >= 5 && std::isdigit(timePart[0]) && std::isdigit(timePart[1]) &&
                timePart[2] == ':' && std::isdigit(timePart[3]) && std::isdigit(timePart[4])) {
                hour = (timePart[0] - '0') * 10 + (timePart[1] - '0');
                if (hour < 0 || hour > 23) {
                    hour = -1; 
                }
            }
        }

        
        if (hour < 0) {
            continue;
        }

        zoneCounts[zoneId]++;
        slotCounts[zoneId][hour]++;
    }
}

std::vector<ZoneCount> TripAnalyzer::topZones(int k) const {
    std::vector<ZoneCount> result;
    result.reserve(zoneCounts.size());

    for (const auto& pair : zoneCounts) {
        result.push_back({pair.first, pair.second});
    }
    
    std::sort(result.begin(), result.end(), [](const ZoneCount& a, const ZoneCount& b) {
        if (a.count != b.count) {
            return a.count > b.count; 
        }
        return a.zone < b.zone;
    });

    if (result.size() > static_cast<size_t>(k)) {
        result.resize(k);
    }

    return result;
}

std::vector<SlotCount> TripAnalyzer::topBusySlots(int k) const {
    std::vector<SlotCount> result;
    result.reserve(10000);

    for (const auto& zonePair : slotCounts) {
        for (const auto& hourPair : zonePair.second) {
            result.push_back({zonePair.first, hourPair.first, hourPair.second});
        }
    }

    std::sort(result.begin(), result.end(),
        [](const SlotCount& a, const SlotCount& b) {
            if (a.count != b.count) {
                return a.count > b.count;
            }
            if (a.zone != b.zone) {
                return a.zone < b.zone;
            }
            return a.hour < b.hour;
        });

    if (result.size() > static_cast<size_t>(k)) {
        result.resize(k);
    }

    return result;
}


