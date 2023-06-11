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

static auto queue = spsc_queue_fix<TestStruct, 1000>();
static std::vector<TestStruct> data(2000);
static void init(const benchmark::State &) {
    std::generate_n(data.begin(), data.size(), []() { return TestStruct{
                                                              rand(), rand(), rand()}; });
}

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

BENCHMARK_TEMPLATE(BM_PushPop, spsc_queue_fix<TestStruct, 600>)->Setup(init)->Iterations(1000);
BENCHMARK_TEMPLATE(BM_PushPop, spsc_queue<TestStruct, 600>)->Setup(init)->Iterations(1000);

static void BM_MultiThreaded(benchmark::State &state) {
    for (auto _: state) {
        if (state.thread_index() == 0) {
            for (auto it = data.begin(); it != data.end(); ++it) {
                queue.push(it.base());
                benchmark::ClobberMemory();
            }

        } else {
            size_t cntr = 0;
            while (queue.pop() != nullptr) {
                benchmark::DoNotOptimize(++cntr);
                if (cntr == data.size())
                    break;
            }
            std::cout << cntr << std::endl;
        }
    }
}

//BENCHMARK(BM_MultiThreaded)->Setup(init)->Threads(2)->Iterations(1);
//        Teardown([](const benchmark::State &) { std::cout << "STOP" << std::endl; queue.stop(); });

BENCHMARK_MAIN();
