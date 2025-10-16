#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <map>
#include <unistd.h>

typedef char *Address;
typedef std::chrono::duration<long long, std::ratio<1, 1000000000>> Duration;

using namespace std::chrono_literals;

Duration bench(const long long spots, const Address *currentP) {
    const auto timeBefore = std::chrono::high_resolution_clock::now();
    for (long long spot = 0; spot < spots; ++spot) {
        currentP = reinterpret_cast<Address *>(*currentP);
    }
    const auto timeAfter = std::chrono::high_resolution_clock::now();
    const auto finalTime = timeAfter - timeBefore;
    return finalTime;
}

Duration benchMedian(const long long spots, const Address *currentP, const int numTries = 5, const Duration minViableTime = 10ns) {
    std::vector<Duration> results;
    results.reserve(numTries);
    for (int i = 0; i < numTries; ++i) {
        if (const Duration result = bench(spots, currentP); result > 10ns) {
            results.push_back(result);
        }
    }
    if (results.empty()) {
        throw std::runtime_error(std::format("No results for spots={}", spots));
    }
    const unsigned long medianIndex = results.size() / 2;
    const auto median = results.begin() + medianIndex;
    std::ranges::nth_element(results, median);
    return results[medianIndex];
}

bool withinErrorMargin(const Duration &a, const Duration &b, const double errorMargin) {
    return a * (1. - errorMargin) <= b && b <= a * (1. + errorMargin);
}

std::map<int, Duration> spotsWithJumps(const std::vector<Duration> &data, const double errorMargin) {
    std::map<int, Duration> result;
    Duration lastComparisonPoint = *data.rbegin();
    for (int i = data.size() - 2; i >= 0; --i) {
        if (!withinErrorMargin(lastComparisonPoint, data[i], errorMargin) && data[i] < lastComparisonPoint) {
            result[i + 1] = data[i + 1];
            // result[i + 1] = data[i];
        }
        lastComparisonPoint = std::min(lastComparisonPoint, data[i]);
    }
    result[1] = data[0];
    return result;
}

bool isOptimized()
{
#ifdef __OPTIMIZE__
    return true;
#else
    return false;
#endif
}

// stride at which jumps stopped and the biggest number of spots which didn't cause a jump
int smallestPowerOfTwo(int x) {
    // Find the smallest 2^p such that x <= 2^p
    int power = ceil(log2(x)); // using logarithm to calculate the required power
    return pow(2, power);
}

std::pair<unsigned long long, int> findLastGroup(const std::map<unsigned long long, int>& values, int errorMargin = 3) {
    int groupStartKey = -1; // Starting key of the current group
    int lastKey = -1; // Key of the last element in the group
    int maxElementInTheLastGroup = -1;
    int lastValue = -1; // Value of the last element in the group
    int firstElement = true; // Flag to handle the first iteration

    // Iterate through the map
    for (const auto& [key, value] : values) {
        if (firstElement) {
            // Initialize the first group
            groupStartKey = key;
            lastKey = key;
            lastValue = value;
            firstElement = false;
            continue;
        }

        // Check if the current value continues the group
        if (abs(value - lastValue) <= errorMargin) {
            // Continue the group
            lastKey = key;
            lastValue = value;
            maxElementInTheLastGroup = std::max(maxElementInTheLastGroup, value);
        } else {
            // Start a new group
            groupStartKey = key;
            lastKey = key;
            lastValue = value;
            maxElementInTheLastGroup = value;
        }
    }

    // Return the starting key of the last group and the num of spots at which the jump didn't occur
    return {groupStartKey, maxElementInTheLastGroup - 1};
}

int main() {
    std::cout << std::boolalpha;
    const auto pagesize = getpagesize();
    std::cout << "Optimized: " << isOptimized() << std::endl;
    constexpr unsigned long long maxData = 1 << 26;
    std::cout << "Max data: " << maxData << std::endl;
    const long maxSpotsSeiling = 100;
    std::map<unsigned long long, int> firstJumpRecords;

    for (unsigned int pow = 1; pow < 64; ++pow) {
        const unsigned long long stride = 1 << pow;
        if (stride < sizeof(char *)) continue;

        // Generate data
        long maxSpots = 0;
        long long i = stride * (maxData / stride);
        if (i == 0) break;
        const auto dataPtr = std::unique_ptr<Address>(new Address[maxData + pagesize]);
        auto data = dataPtr.get();
        // Align test data to the start of a page
        while (reinterpret_cast<long>(data) % pagesize != 0) {
            ++data;
        }
        Address *firstPtr = &data[i];
        for (; i >= 0; i -= stride) {
            ++maxSpots;
            Address next;

            Address *p = &data[i];
            if (i < stride) {
                next = reinterpret_cast<char *>(&data[maxData - stride]);
                assert(next == *firstPtr);
            } else {
                next = reinterpret_cast<char *>(&data[i - stride]);
            }
            *p = next;
        }
        maxSpots = maxSpots > maxSpotsSeiling ? maxSpotsSeiling : maxSpots;
        const Address *currentP = firstPtr;
        std::vector<std::chrono::duration<long long, std::ratio<1, 1000000000>>> results(maxSpots + 1);
        for (long long spots = 1; spots <= maxSpots; ++spots) {
            results[spots] = benchMedian(spots, currentP, 50);
        }
        auto spotsWithJumpsValues = spotsWithJumps(results, 0.2);
        if (spotsWithJumpsValues.size() >= 2) {
            firstJumpRecords[stride] = (++spotsWithJumpsValues.begin())->first;
        }
    }

    for (auto [i, result] : firstJumpRecords) {
        std::cout << "\t" << i << ':' << result;
    }
    std::cout << std::endl;
    auto [largeStride, maxSpotsThatFitIntoSet] = findLastGroup(firstJumpRecords);
    std::cout << "Large stride: " << largeStride << ", max spots that fit into set: " << maxSpotsThatFitIntoSet << std::endl;
    auto cacheAssociativity = maxSpotsThatFitIntoSet;
    auto cacheSizeBytes = largeStride * sizeof(char *) * cacheAssociativity;
    std::cout << "Cache associativity: " << cacheAssociativity << std::endl;
    std::cout << "Cache size (bytes): " << cacheSizeBytes << std::endl;
    std::cout << "Cache size (KB): " << static_cast<double>(cacheSizeBytes) / 1024. << std::endl;
    std::cout << "Cache size (MB): " << static_cast<double>(cacheSizeBytes) / 1024. / 1024. << std::endl;

    return 0;
}
