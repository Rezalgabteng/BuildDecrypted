/*
 * DecryptEngine.hpp — Final_Dispatch 断点 + 后台线程 + 坐标缓存
 * ================================================================
 * 独立线程持续轮询硬件断点, 渲染线程只读缓存, ioctl 与渲染解耦
 */

#ifndef DECRYPT_ENGINE_HPP
#define DECRYPT_ENGINE_HPP

#include "Kernel.hpp"
#include <cstring>
#include <cstdint>
#include <cstdio>
#include <map>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>

namespace DecryptOffsets {
    constexpr uint64_t HOOK_LITERAL = 0x自己猜;
    constexpr int64_t  WR_CONTEXT_DATA   = -8;
    constexpr uint64_t WR_VM_ENTRY_PTR   = 0xA0;
    constexpr uint64_t WR_FINAL_DISPATCH = 0x150;
    int IsDecode = 3;
}

struct ResolvedAddrs { uint64_t wrapper_base, vm_base; bool valid; };

class AddrResolver {
public:
    static ResolvedAddrs resolve(uint64_t libUE4_base) {
        ResolvedAddrs a = {};
        if (!libUE4_base) return a;
        a.wrapper_base = driver->read<uint64_t>(libUE4_base + DecryptOffsets::HOOK_LITERAL) & 0x00FFFFFFFFFFFFFFULL;
        if (!a.wrapper_base || a.wrapper_base < 0x100000) return a;
        a.vm_base = driver->read<uint64_t>(a.wrapper_base + DecryptOffsets::WR_VM_ENTRY_PTR) & 0x00FFFFFFFFFFFFFFULL;
        a.valid = true;
        printf("[Decrypt] wrapper=0x%llx vm=0x%llx\n", (unsigned long long)a.wrapper_base, (unsigned long long)a.vm_base);
        return a;
    }
};

class DecryptEngine {
public:
    static DecryptEngine& get() { static DecryptEngine inst; return inst; }

    bool init(uint64_t libUE4_base) {
        if (_bp_id >= 0) return true;
        _addr = AddrResolver::resolve(libUE4_base);
        if (!_addr.valid) return false;

        uint64_t bp_addr = _addr.wrapper_base + DecryptOffsets::WR_FINAL_DISPATCH;
        printf("[Decrypt] 设置捕获断点: 0x%llx (Final_Dispatch)\n", (unsigned long long)bp_addr);

        if (driver->hwbp_attach(driver->get_pid()) < 0) {
        printf("[Attach] 解密执行失败\n"); 
        DecryptOffsets::IsDecode = 2;
        return false; }

        _bp_id = driver->hwbp_bp_set(bp_addr,
            Kernel::HWBP_ENABLED | Kernel::HWBP_CAPTURE | Kernel::HWBP_TIMING_BYPASS |
            Kernel::HWBP_INTERCEPT | Kernel::HWBP_DIAGNOSTIC | Kernel::HWBP_BAIT_GUARD,
            Kernel::HWBP_TYPE_X, 4);
        if (_bp_id < 0) { 
        printf("[Decrypt] 解密执行失败 %d\n", _bp_id); 
        DecryptOffsets::IsDecode = 1;
        return false; }

        driver->hwbp_bp_enable(_bp_id);
        printf("[Decrypt] 断点 ID=%d 已就绪\n", _bp_id);

        // ★ 启动后台轮询线程 ★
        _running = true;
        _thread = std::thread(&DecryptEngine::_thread_loop, this);
        printf("[Decrypt] 后台线程已启动\n");
        DecryptOffsets::IsDecode = 0;
        return true;
    }

    /*
     * 渲染线程调用: 纯缓存查找, 无 ioctl
     */
    bool get(uint64_t rootComp, float& x, float& y, float& z) {
        std::lock_guard<std::mutex> lk(_mtx);
        auto it = _cache.find(rootComp);
        if (it == _cache.end()) return false;
        x = it->second.x; y = it->second.y; z = it->second.z;
        return true;
    }

    bool ready() const { return _bp_id >= 0; }

    void cleanup() {
        if (_running) {
            _running = false;
            if (_thread.joinable()) _thread.join();
            printf("[Decrypt] 后台线程已停止\n");
        }
        if (_bp_id >= 0) { driver->hwbp_bp_remove(_bp_id); _bp_id = -1; }
        driver->hwbp_reset();
        printf("[Decrypt] 断点已清理\n");
    }

private:
    DecryptEngine() : _bp_id(-1), _last_hit(0), _running(false) {}

    /*
     * 后台线程: 持续轮询断点, 1ms 间隔
     */
    void _thread_loop() {
        while (_running) {
            _poll_internal();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    /*
     * 内部轮询: 查断点 → 写缓存 (不加锁 poll 部分, 只在写缓存时加锁)
     */
    void _poll_internal() {
        if (_bp_id < 0) return;

        Kernel::hwbp_bp_info info;
        if (driver->hwbp_bp_get_info(_bp_id, info) < 0 || !info.has_snapshot) return;
        if (info.hit_count <= _last_hit) return;
        _last_hit = info.hit_count;
        
        printf("hit=%llu clear=%llu dfi=%llu mdscr=%llu\n",
           info.hit_count, info.external_clear_count,
           info.dfi_restore_count, info.mdscr_restore_count);

        
        uint64_t comp = info.x[19];
        uint64_t transform = info.x[0];
        
        float x = driver->read<float>(transform + 0x10);
        float y = driver->read<float>(transform + 0x14);
        float z = driver->read<float>(transform + 0x18);

        if (comp > 0x100000
            && (x > 10.0f || x < -10.0f)
            && (y > 10.0f || y < -10.0f)
            && (z > 10.0f || z < -10.0f)
            && !std::isnan(x) && !std::isnan(y) && !std::isnan(z)) {
            printf("X %f Y %f Z %f\n", x, y, z);
            std::lock_guard<std::mutex> lk(_mtx);
            _cache[comp] = {x, y, z};
        }
    }

    struct Vec3 { float x, y, z; };
    ResolvedAddrs _addr;
    int _bp_id;
    uint64_t _last_hit;
    std::map<uint64_t, Vec3> _cache;
    std::mutex _mtx;

    std::thread _thread;
    std::atomic<bool> _running;
};

#endif
