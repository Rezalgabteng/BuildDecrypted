# BuildDecrypted

**UE4 Coordinate Extraction Engine** untuk **com.proximabeta.mf.uamo** menggunakan hardware breakpoint dan kernel-level memory access.

```
╔════════════════════════════════════════════════════════════╗
║  Hardware Breakpoint → Final_Dispatch → Coordinate Cache  ║
║  Background Thread @ 1ms polling | Thread-safe access    ║
╚════════════════════════════════════════════════════════════╝
```

## Features

✅ **Hardware Breakpoint Polling** - Capture coordinates at UE4's Final_Dispatch function  
✅ **Runtime libUE4.so Loading** - Auto-extract from running game (no 240MB storage needed)  
✅ **Kernel-Level Memory Access** - Via Paradise driver (root-level R/W)  
✅ **Background Thread Architecture** - 1ms polling interval, decoupled from rendering  
✅ **Thread-Safe Caching** - Mutex-protected coordinate storage  
✅ **Automated Setup** - One-command kernel driver installation  

## Quick Start

### 1️⃣ Prerequisites
```bash
sudo apt-get install cmake g++ libpthread-stubs0-dev unzip
```

### 2️⃣ Setup Kernel Driver
```bash
sudo chmod +x setup_driver.sh
sudo ./setup_driver.sh
```

### 3️⃣ Build
```bash
chmod +x build.sh
./build.sh
```

### 4️⃣ Run
Start game on device, then:
```bash
./build/decrypt_engine com.proximabeta.mf.uamo
```

Output:
```
╔═══════════════════════════════════════╗
║     BuildDecrypted - Coordinate      ║
║        Extraction Engine v1.0        ║
╚═══════════════════════════════════════╝

[Main] Initializing Paradise driver...
[Main] Paradise driver initialized successfully
[Main] Searching for process: com.proximabeta.mf.uamo
[Main] Found target process: com.proximabeta.mf.uamo (PID: 12345)
[Main] libUE4.so base: 0x7f12ab000000
[Main] Initializing DecryptEngine...
[Main] DecryptEngine initialized successfully
[Main] Background thread polling for coordinates...
```

## Architecture

### libUE4.so - Runtime Loading (No Storage)
- `paradise_driver->get_module_base("libUE4.so")` auto-extracts from running game
- Scans `/proc/[pid]/maps` → finds memory range → parses ELF headers
- **Result:** No need to store 240MB file in repository ✓

### Thread Model
```
Main Thread
  ├─ Signal handling (SIGINT, SIGTERM)
  ├─ Initialize paradise_driver
  ├─ Find target process
  └─ Start DecryptEngine

Background Thread (DecryptEngine)
  ├─ Poll hardware breakpoint @ 1ms interval
  ├─ Extract registers (X0-X31)
  ├─ Read coordinates from memory
  ├─ Cache update (mutex-protected)
  └─ Loop until shutdown

Render Thread (optional)
  └─ Read-only cache access (no ioctl)
```

### Data Flow
```
Game Process
  ↓
Final_Dispatch execution
  ↓
Hardware Breakpoint triggered
  ↓
Register snapshot captured (X0-X31)
  ↓
X19 = rootComp, X0 = transform
  ↓
Read float X,Y,Z @ transform+0x10/0x14/0x18
  ↓
Cache update (thread-safe)
  ↓
Ready for use
```

## Configuration

### Change Target Game
Edit `main.cpp` line ~110 or pass as argument:
```bash
./decrypt_engine com.your.game.name
```

### Fix HOOK_LITERAL Offset

File: `DecryptEngine.hpp` line 21
```cpp
constexpr uint64_t HOOK_LITERAL = 0x自己猜;  // ← Update this
```

**Discovery methods:**

1. **IDA Pro / Ghidra**
   - Load libUE4.so
   - Find `Final_Dispatch` or `FinalDispatch` symbol
   - Backtrace to wrapper entry point
   - Calculate offset from libUE4 base

2. **Command-line analysis**
   ```bash
   nm libUE4.so | grep -i final
   readelf -s libUE4.so | grep Dispatch
   ```

3. **Quick extract from device**
   ```bash
   adb pull /data/app/com.proximabeta.mf.uamo-*/lib/arm64-v8a/libUE4.so ./
   # Then analyze locally
   ```

## Files Structure

```
BuildDecrypted/
├── CMakeLists.txt                 Build configuration
├── main.cpp                       Entry point & initialization
├── Kernel.hpp                     Kernel wrapper interface
├── Kernel.cpp                     Kernel implementation
├── DecryptEngine.hpp              Hardware BP polling logic
├── paradise_api.h                 Paradise kernel driver API
├── libparadise_api.a              Static library (link)
├── driver_ko_601.zip              Kernel driver source/binary
├── setup_driver.sh                Driver setup script
├── build.sh                       Automated build script
├── BUILD.md                       Detailed setup guide
└── README.md                      This file
```

## Troubleshooting

| Error | Cause | Solution |
|-------|-------|----------|
| `driver not initialized` | Paradise module not loaded | `sudo setup_driver.sh` |
| `Could not find process` | Game not running | Start game on device |
| `Could not find libUE4.so` | Process terminated | Verify with `adb shell ps` |
| `Permission denied` | Not running as root | Use `sudo ./decrypt_engine ...` |
| `IsDecode = 2` | hwbp_attach failed | Check module status: `lsmod` |
| `IsDecode = 1` | Wrong HOOK_LITERAL offset | Update offset in DecryptEngine.hpp |

## Dependencies

- **CMake** 3.10+
- **GCC/G++** with C++17 support
- **libparadise_api.a** (static library, included)
- **Paradise Kernel Driver** (driver_ko_601.zip, auto-extracted)
- **pthread** (standard library)

## Notes

- **ARM64 only** - Designed for ARM64 architecture (mobile devices)
- **Root required** - Kernel module loading needs sudo
- **Game must be running** - paradise_driver extracts from live process
- **No libUE4.so storage** - Runtime extraction from game memory
- **Thread-safe** - All coordinate access protected by mutex

## Related Files

- **BUILD.md** - Comprehensive setup guide with offset discovery methods
- **paradise_api.h** - Full Paradise kernel driver API documentation
- **DecryptEngine.hpp** - Hardware breakpoint implementation details

---

**Target:** com.proximabeta.mf.uamo  
**Status:** Production-ready  
**Last Updated:** 2026-09-15

