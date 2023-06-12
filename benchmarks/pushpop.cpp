#include <benchmark/benchmark.h>

#include "SpscQueue.h"
#include "SpscQueueFixed.h"

#include <iostream>
#include <thread>

// clang-format off
static void escape(void *p) {
    asm volatile("" : : "g"(p) : "memory");
}
static void clobber() {
    asm volatile("" : : : "memory");
}
// clang-format on

struct TestStruct {
    long long a;
    long long b;
    long long c;
};

static auto queueFix = spsc_queue_fix<TestStruct, 1000>();
static auto queueOrig = spsc_queue<TestStruct, 1000>();
static std::vector<TestStruct> data(2000);
static void init(const benchmark::State &) {
    std::generate_n(data.begin(), data.size(), []() { return TestStruct{
                                                              rand(), rand(), rand()}; });
}

void BM_PushPopOrig(benchmark::State &state) {
    for (auto _: state) {
        queueOrig.restart();
        auto pushThread = std::jthread([&]() {
            for (auto it = data.begin(); it != data.end(); ++it) {
                queueOrig.push(it.base());
                benchmark::ClobberMemory();
            }
        });

        size_t cntr{0};
        auto popThread = std::jthread([&]() {
            while (queueOrig.pop()) {
                ++cntr;
                if (cntr == data.size())
                    queueOrig.stop();
            }
            assert(cntr == data.size());
        });
    }
}


void BM_PushPopFix(benchmark::State &state) {
    for (auto _: state) {
        queueFix.restart();
        auto pushThread = std::jthread([&]() {
            for (auto it = data.begin(); it != data.end(); ++it) {
                queueFix.push(it.base());
                benchmark::ClobberMemory();
            }
        });

        size_t cntr{0};
        auto popThread = std::jthread([&]() {
            while (queueFix.pop()) {
                ++cntr;
                if (cntr == data.size())
                    queueFix.stop();
            }
            assert(cntr == data.size());
        });
    }
}

BENCHMARK(BM_PushPopOrig)->Setup(init)->Iterations(1000);
BENCHMARK(BM_PushPopFix)->Setup(init)->Iterations(1000);

template<class Queue>
void BM_PushPop(benchmark::State &state) {
    for (auto _: state) {
        auto queue = Queue();
        auto pushThread = std::jthread([&]() {
            for (auto it = data.begin(); it != data.end(); ++it) {
                queue.push(it.base());
                benchmark::ClobberMemory();
            }
        });

        size_t cntr{0};
        auto popThread = std::jthread([&]() {
            while (queue.pop()) {
                benchmark::DoNotOptimize(++cntr);
                if (cntr == data.size())
                    queue.stop();
            }
            assert(cntr == data.size());
        });
    }
}

BENCHMARK_TEMPLATE(BM_PushPop, spsc_queue<TestStruct, 64>)->Setup(init)->Iterations(1000);
BENCHMARK_TEMPLATE(BM_PushPop, spsc_queue_fix<TestStruct, 64>)->Setup(init)->Iterations(1000);
BENCHMARK_TEMPLATE(BM_PushPop, spsc_queue<TestStruct, 666>)->Setup(init)->Iterations(1000);
BENCHMARK_TEMPLATE(BM_PushPop, spsc_queue_fix<TestStruct, 666>)->Setup(init)->Iterations(1000);
BENCHMARK_TEMPLATE(BM_PushPop, spsc_queue<TestStruct, 2000>)->Setup(init)->Iterations(1000);
BENCHMARK_TEMPLATE(BM_PushPop, spsc_queue_fix<TestStruct, 2000>)->Setup(init)->Iterations(1000);

BENCHMARK_MAIN();
