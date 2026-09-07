#pragma once

#include <atomic>
#include <cstddef>
#include <memory>
#include <vector>

// MRSW (multi-reader, single-writer) regular register.
//
// A value in [0, M] is encoded as M boolean toggles: toggles 0..value-1
// are true, everything at/above value is false. The reader returns the
// index of the first false toggle.
//
//   read()  – O(M) scan
//   write() – O(M): set 0..value-1 true, then value..M-1 false
//
// Two representations are provided: a plain bool vector (only valid when
// no writer runs concurrently) and a vector of std::atomic<bool> that
// preserves the register guarantees under a concurrent single writer.

class RegularRegister {
public:
    RegularRegister(std::size_t bound, bool atomic_mode)
        : M_(bound), use_atomic_(atomic_mode)
    {
        if (use_atomic_) {
            toggles_atomic_.reset(new std::atomic<bool>[M_]);
            for (std::size_t i = 0; i < M_; ++i) {
                toggles_atomic_[i].store(false, std::memory_order_seq_cst);
            }
        }
        else {
            toggles_bool_.assign(M_, false);
        }
    }

    void write(int value)
    {
        if (use_atomic_) {
            for (std::size_t i = 0; i < static_cast<std::size_t>(value); ++i) {
                toggles_atomic_[i].store(true, std::memory_order_seq_cst);
            }
            for (std::size_t i = static_cast<std::size_t>(value); i < M_; ++i) {
                toggles_atomic_[i].store(false, std::memory_order_seq_cst);
            }
        }
        else {
            for (std::size_t i = 0; i < static_cast<std::size_t>(value); ++i) {
                toggles_bool_[i] = true;
            }
            for (std::size_t i = static_cast<std::size_t>(value); i < M_; ++i) {
                toggles_bool_[i] = false;
            }
        }
    }

    int read()
    {
        if (use_atomic_) {
            for (std::size_t i = 0; i < M_; ++i) {
                if (!toggles_atomic_[i].load(std::memory_order_seq_cst)) {
                    return static_cast<int>(i);
                }
            }
        }
        else {
            for (std::size_t i = 0; i < M_; ++i) {
                if (!toggles_bool_[i]) return static_cast<int>(i);
            }
        }
        return static_cast<int>(M_);
    }

private:
    std::size_t                        M_;
    bool                               use_atomic_;
    std::vector<bool>                  toggles_bool_;
    std::unique_ptr<std::atomic<bool>[]> toggles_atomic_;
};