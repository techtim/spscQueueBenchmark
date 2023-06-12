#pragma once

#include <array>
#include <emmintrin.h>
#include <utility>

#define FORCE_INLINE inline __attribute__((always_inline))
#define UNLIKELY(condition) __builtin_expect(static_cast<bool>(condition), 0)

template<class T, size_t size>
class spsc_queue {
private:
    /// Volatile has nothing to do with memory access guaranties in multi-threaded environment.
    volatile __attribute__((aligned(64))) uint64_t stopFlag = 0;
    volatile __attribute__((aligned(64))) uint64_t pread = 0;
    volatile __attribute__((aligned(64))) uint64_t pwrite = 0;
    __attribute__((aligned(64))) std::array<const T *, size> buffer;

public:
    spsc_queue() {
        std::fill(buffer.begin(), buffer.end(), nullptr);
    }

    /// Both producer and consumer make reads at same addresses in the buffer without any ordering
    /// that lead to race conditions confirmed by TSAN.
    FORCE_INLINE bool empty() const { return buffer[pread] == nullptr; }

    FORCE_INLINE bool available() const {
        return buffer[pwrite] == nullptr;
    }

    FORCE_INLINE void push(const T *data) {
        while (!stopFlag && !available()) {
            _mm_pause();
        }
        if (UNLIKELY(stopFlag)) {
            return;
        }
        buffer[pwrite] = data;
        pwrite = (pwrite + 1) % size;
    }

    FORCE_INLINE const T *pop() {
        while (!stopFlag && empty()) {
            _mm_pause();
        }

        const T *result = nullptr;
        if (UNLIKELY(stopFlag)) {
            return result;
        }

        /// can use swap
        result = buffer[pread];
        buffer[pread] = nullptr;
        /// mod is pricey can check: pwrite+1 < size ? pwrite + 1  : (pwrite+1) % size;
        pread = (pread + 1) % size;
        return result;
    }
    FORCE_INLINE void stop() {
        stopFlag = 1;
    }
    FORCE_INLINE void restart() {
        stopFlag = 1;
        pread = 0;
        pwrite = 0;
    }
};//spsc queue
