/*
 * Kernel.hpp — Kernel Driver Abstraction Layer
 * ============================================
 * Wraps paradise_api.h untuk DecryptEngine
 * Menyediakan interface ke hardware breakpoint & memory operations
 */

#ifndef KERNEL_HPP
#define KERNEL_HPP

#include "paradise_api.h"
#include <cstdint>

class Kernel {
public:
    // Hardware Breakpoint Flags
    static constexpr uint32_t HWBP_ENABLED        = 0x01;
    static constexpr uint32_t HWBP_CAPTURE        = 0x02;
    static constexpr uint32_t HWBP_TIMING_BYPASS  = 0x04;
    static constexpr uint32_t HWBP_INTERCEPT      = 0x08;
    static constexpr uint32_t HWBP_DIAGNOSTIC     = 0x10;
    static constexpr uint32_t HWBP_BAIT_GUARD     = 0x20;

    // Hardware Breakpoint Types
    static constexpr uint32_t HWBP_TYPE_X = PARADISE_HWBP_EXECUTE;
    static constexpr uint32_t HWBP_TYPE_W = PARADISE_HWBP_WRITE;
    static constexpr uint32_t HWBP_TYPE_R = PARADISE_HWBP_READ;
    static constexpr uint32_t HWBP_TYPE_RW = PARADISE_HWBP_READ_WRITE;

    // Hardware Breakpoint Info Structure
    struct hwbp_bp_info {
        uint64_t hit_count;
        uint64_t external_clear_count;
        uint64_t dfi_restore_count;
        uint64_t mdscr_restore_count;
        bool has_snapshot;
        uint64_t x[32];  // ARM64 X0-X31 registers dari snapshot
    };

    // Static methods (to be implemented by driver wrapper)
    static int hwbp_attach(pid_t pid);
    static int hwbp_bp_set(uint64_t addr, uint32_t flags, uint32_t type, int size);
    static int hwbp_bp_enable(int bp_id);
    static int hwbp_bp_get_info(int bp_id, hwbp_bp_info& info);
    static int hwbp_bp_remove(int bp_id);
    static int hwbp_reset();
    
    static pid_t get_pid();
    
    template <typename T>
    static T read(uint64_t addr) {
        T result{};
        // Implementation akan menggunakan paradise_driver
        return result;
    }
};

// Global driver instance (akan diisi oleh main)
extern paradise_driver* g_driver;

// Helper macro untuk akses driver global
#define driver g_driver

#endif
