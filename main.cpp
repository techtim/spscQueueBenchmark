#include <iostream>

#include "SpscQueueFixed.h"
#include <assert.h>
#include <syncstream>
#include <thread>
#include <vector>

struct TestStruct {
    long long a;
    long long b;
    long long c;
};

int main() {
    const std::vector<TestStruct> data(2048);
    auto queue = spsc_queue_fix<TestStruct, 1024>();
    auto pushThread = std::thread([&]() {
        try {
            for (auto it = data.begin(); it != data.end(); ++it) {
                queue.push(it.base());
            }
        } catch (const std::exception &e) {
            std::osyncstream(std::cout) << "ERRR: " << e.what() << std::endl;
        }
    });

    size_t cntr{0};
    auto popThread = std::thread([&]() {
        try {
            while (queue.pop()) {
                ++cntr;
                if (cntr == data.size())
                    break;
            }
        } catch (const std::exception &e) {
            std::osyncstream(std::cout) << "ERRR POP: " << e.what() << std::endl;
        }
        std::osyncstream(std::cout) << "POPED: " << cntr;
        queue.stop();
    });

    if (pushThread.joinable())
        pushThread.join();
    if (popThread.joinable())
        popThread.join();

    assert(cntr == data.size());

    return 0;
}
