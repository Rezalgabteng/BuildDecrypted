/*
 * main.cpp — BuildDecrypted Entry Point
 * ===================================
 * Initialize Paradise driver, setup DecryptEngine, run coordinate capture
 */

#include "Kernel.hpp"
#include "DecryptEngine.hpp"
#include "paradise_api.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include <chrono>
#include <thread>

// Global state
static bool g_running = true;
static DecryptEngine* g_engine = nullptr;

void signal_handler(int sig) {
    printf("\n[Main] Received signal %d, shutting down...\n", sig);
    g_running = false;
}

void setup_signal_handlers() {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
}

int find_target_process(const char* process_name) {
    if (!g_driver) {
        printf("[Main] Error: driver not initialized\n");
        return -1;
    }
    
    pid_t target_pid = g_driver->get_pid(process_name);
    if (target_pid <= 0) {
        printf("[Main] Could not find process: %s\n", process_name);
        return -1;
    }
    
    printf("[Main] Found target process: %s (PID: %d)\n", process_name, target_pid);
    return target_pid;
}

uint64_t find_libue4_base(pid_t target_pid) {
    if (!g_driver) {
        printf("[Main] Error: driver not initialized\n");
        return 0;
    }
    
    g_driver->initialize(target_pid);
    
    // Try to get libUE4 base address
    uint64_t libue4_base = g_driver->get_module_base("libUE4.so");
    if (libue4_base == 0) {
        printf("[Main] Could not find libUE4.so base address\n");
        return 0;
    }
    
    printf("[Main] libUE4.so base: 0x%llx\n", (unsigned long long)libue4_base);
    return libue4_base;
}

int main(int argc, char* argv[]) {
    printf("╔═══════════════════════════════════════╗\n");
    printf("║     BuildDecrypted - Coordinate      ║\n");
    printf("║        Extraction Engine v1.0        ║\n");
    printf("╚═══════════════════════════════════════╝\n\n");
    
    // Setup signal handlers for clean shutdown
    setup_signal_handlers();
    
    // Initialize Paradise driver
    printf("[Main] Initializing Paradise driver...\n");
    try {
        g_driver = new paradise_driver();
        printf("[Main] Paradise driver initialized successfully\n");
    } catch (const std::exception& e) {
        printf("[Main] ERROR: Failed to initialize Paradise driver: %s\n", e.what());
        return EXIT_FAILURE;
    }
    
    if (!g_driver) {
        printf("[Main] ERROR: Paradise driver is null\n");
        return EXIT_FAILURE;
    }
    
    // Get target process name (default: "com.tencent.jkq")
    const char* target_process = "com.tencent.jkq";
    if (argc > 1) {
        target_process = argv[1];
    }
    
    printf("[Main] Searching for process: %s\n", target_process);
    
    pid_t target_pid = find_target_process(target_process);
    if (target_pid <= 0) {
        printf("[Main] ERROR: Could not find target process\n");
        delete g_driver;
        return EXIT_FAILURE;
    }
    
    // Find libUE4 base address
    uint64_t libue4_base = find_libue4_base(target_pid);
    if (libue4_base == 0) {
        printf("[Main] ERROR: Could not find libUE4 base address\n");
        delete g_driver;
        return EXIT_FAILURE;
    }
    
    // Initialize DecryptEngine
    printf("[Main] Initializing DecryptEngine...\n");
    g_engine = &DecryptEngine::get();
    
    if (!g_engine->init(libue4_base)) {
        printf("[Main] ERROR: Failed to initialize DecryptEngine\n");
        printf("[Main] Possible causes:\n");
        printf("  - Incorrect HOOK_LITERAL offset\n");
        printf("  - Paradise driver not properly loaded\n");
        printf("  - Insufficient permissions\n");
        delete g_driver;
        return EXIT_FAILURE;
    }
    
    printf("[Main] DecryptEngine initialized successfully\n");
    printf("[Main] Background thread polling for coordinates...\n\n");
    
    // Main loop - query coordinates periodically
    int capture_count = 0;
    while (g_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // Optional: could add rendering or output logic here
        if (g_engine->ready()) {
            capture_count++;
            if (capture_count % 20 == 0) {  // Print status every 10 seconds
                printf("[Main] Engine running, collecting coordinates...\n");
            }
        }
    }
    
    // Cleanup
    printf("\n[Main] Shutting down...\n");
    if (g_engine) {
        g_engine->cleanup();
    }
    
    if (g_driver) {
        delete g_driver;
        g_driver = nullptr;
    }
    
    printf("[Main] Goodbye!\n");
    return EXIT_SUCCESS;
}
