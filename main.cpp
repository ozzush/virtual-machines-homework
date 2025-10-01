#include <chrono>
#include <iostream>

typedef char *Address;

int main() {
    constexpr long maxData = 1 << 15;
    std::cout << "Max data: " << maxData << std::endl;
    const auto dataPtr = std::unique_ptr<Address>(new Address[maxData]);
    const auto data = dataPtr.get();
    for (unsigned int pow = 1; pow < 64; ++pow) {
        const long stride = 1 << pow;

        // Generate data
        long maxSpots = 0;
        long long i = stride * (maxData / stride);
        Address *firstPtr = &data[i];
        for (; i >= 0; i -= stride) {
            ++maxSpots;
            Address next;

            Address *p = &data[i];
            if (i < stride) {
                next = reinterpret_cast<char *>(&data[maxData - stride]);
            } else {
                next = reinterpret_cast<char *>(&data[i - stride]);
            }
            *p = next;
        }
        maxSpots = maxSpots > 50 ? 50 : maxSpots;
        const Address *currentP = firstPtr;
        std::vector<std::chrono::duration<long long, std::ratio<1, 1000000000>>> results(maxSpots);
        std::cout << "stride\\spots";
        for (int j = 0; j < maxSpots; ++j) {
            std::cout << "\t" << j + 1;
        }
        std::cout << std::endl;
        for (long long spots = 1; spots <= maxSpots; ++spots) {
            auto timeBefore = std::chrono::high_resolution_clock::now();
            for (long long spot = 0; spot < spots; ++spot) {
                currentP = reinterpret_cast<Address *>(*currentP);
            }
            auto timeAfter = std::chrono::high_resolution_clock::now();
            auto finalTime = timeAfter - timeBefore;
            results[spots - 1] = finalTime;
        }
        std::cout << stride;
        for (auto res : results) {
            std::cout << "\t" << res;
        }
        std::cout << std::endl;
    }

    return 0;
    // TIP See CLion help at <a href="https://www.jetbrains.com/help/clion/">jetbrains.com/help/clion/</a>. Also, you can try interactive lessons for CLion by selecting 'Help | Learn IDE Features' from the main menu.
}
