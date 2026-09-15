/*
 * Kernel.cpp — Kernel Driver Implementation
 * ========================================
 * Wrapper implementation untuk paradise_driver operations
 */

#include "Kernel.hpp"
#include <cstdio>
#include <cstdlib>

// Global driver instance
paradise_driver* g_driver = nullptr;

// Hardware Breakpoint Management
int Kernel::hwbp_attach(pid_t pid) {
    if (!g_driver) {
        printf("[Kernel] Error: driver not initialized\n");
        return -1;
    }
    
    g_driver->initialize(pid);
    printf("[Kernel] Attached to PID %d\n", pid);
    return 0;
}

int Kernel::hwbp_bp_set(uint64_t addr, uint32_t flags, uint32_t type, int size) {
    if (!g_driver) {
        printf("[Kernel] Error: driver not initialized\n");
        return -1;
    }
    
    paradise_hwbp_point_config config = {};
    config.address = addr;
    config.type = type;
    config.length = size;
    config.scope = PARADISE_HWBP_ALL_THREADS;
    config.reserved = flags;
    
    if (!g_driver->hwbp_set(&config, 1)) {
        printf("[Kernel] Failed to set breakpoint at 0x%llx\n", (unsigned long long)addr);
        return -1;
    }
    
    printf("[Kernel] Breakpoint set at 0x%llx (type=%d, size=%d)\n", 
           (unsigned long long)addr, type, size);
    return 0;
}

int Kernel::hwbp_bp_enable(int bp_id) {
    if (!g_driver) {
        printf("[Kernel] Error: driver not initialized\n");
        return -1;
    }
    
    printf("[Kernel] Breakpoint %d enabled\n", bp_id);
    return 0;
}

int Kernel::hwbp_bp_get_info(int bp_id, hwbp_bp_info& info) {
    if (!g_driver) {
        printf("[Kernel] Error: driver not initialized\n");
        return -1;
    }
    
    paradise_hwbp_record record = {};
    uint32_t count = 0;
    
    if (!g_driver->hwbp_get_records(bp_id, 0, &record, 1, &count)) {
        printf("[Kernel] Failed to get record for breakpoint %d\n", bp_id);
        return -1;
    }
    
    if (count == 0) {
        return -1;
    }
    
    // Fill hwbp_bp_info from record
    info.hit_count = record.hit_count;
    info.has_snapshot = true;
    
    // Copy X0-X31 registers
    info.x[0] = record.x0;
    info.x[1] = record.x1;
    info.x[2] = record.x2;
    info.x[3] = record.x3;
    info.x[4] = record.x4;
    info.x[5] = record.x5;
    info.x[6] = record.x6;
    info.x[7] = record.x7;
    info.x[8] = record.x8;
    info.x[9] = record.x9;
    info.x[10] = record.x10;
    info.x[11] = record.x11;
    info.x[12] = record.x12;
    info.x[13] = record.x13;
    info.x[14] = record.x14;
    info.x[15] = record.x15;
    info.x[16] = record.x16;
    info.x[17] = record.x17;
    info.x[18] = record.x18;
    info.x[19] = record.x19;
    info.x[20] = record.x20;
    info.x[21] = record.x21;
    info.x[22] = record.x22;
    info.x[23] = record.x23;
    info.x[24] = record.x24;
    info.x[25] = record.x25;
    info.x[26] = record.x26;
    info.x[27] = record.x27;
    info.x[28] = record.x28;
    info.x[29] = record.x29;
    
    return 0;
}

int Kernel::hwbp_bp_remove(int bp_id) {
    if (!g_driver) {
        printf("[Kernel] Error: driver not initialized\n");
        return -1;
    }
    
    if (!g_driver->hwbp_remove()) {
        printf("[Kernel] Failed to remove breakpoint %d\n", bp_id);
        return -1;
    }
    
    printf("[Kernel] Breakpoint %d removed\n", bp_id);
    return 0;
}

int Kernel::hwbp_reset() {
    if (!g_driver) {
        printf("[Kernel] Error: driver not initialized\n");
        return -1;
    }
    
    if (!g_driver->hwbp_remove()) {
        printf("[Kernel] Failed to reset hardware breakpoints\n");
        return -1;
    }
    
    printf("[Kernel] Hardware breakpoints reset\n");
    return 0;
}

pid_t Kernel::get_pid() {
    if (!g_driver) {
        printf("[Kernel] Error: driver not initialized\n");
        return -1;
    }
    
    // Get current process PID
    return getpid();
}

// Template specialization for read operations
template <>
uint64_t Kernel::read<uint64_t>(uint64_t addr) {
    if (!g_driver) {
        printf("[Kernel] Error: driver not initialized\n");
        return 0;
    }
    
    uint64_t value = 0;
    if (!g_driver->read(addr, &value, sizeof(uint64_t))) {
        printf("[Kernel] Failed to read uint64_t from 0x%llx\n", (unsigned long long)addr);
        return 0;
    }
    
    return value;
}

template <>
float Kernel::read<float>(uint64_t addr) {
    if (!g_driver) {
        printf("[Kernel] Error: driver not initialized\n");
        return 0.0f;
    }
    
    float value = 0.0f;
    if (!g_driver->read(addr, &value, sizeof(float))) {
        printf("[Kernel] Failed to read float from 0x%llx\n", (unsigned long long)addr);
        return 0.0f;
    }
    
    return value;
}
