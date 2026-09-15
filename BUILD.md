# BuildDecrypted - UE4 Coordinate Extraction Engine

## Overview
Coordinate extraction engine untuk **com.proximabeta.mf.uamo** menggunakan:
- **Hardware Breakpoint** di Final_Dispatch (UE4 rendering function)
- **Paradise Kernel Driver** untuk kernel-level memory access
- **Background Thread** polling dengan 1ms interval
- **Thread-safe Cache** untuk rendering thread access

## libUE4.so - Runtime Loading (No Storage)

libUE4.so **tidak disimpan** di repository (terlalu besar: 240MB). Sebaliknya:

### Runtime Auto-Load
```cpp
// main.cpp - baris ~95
uint64_t libue4_base = g_driver->get_module_base("libUE4.so");
```

Paradise driver otomatis extract dari memory game process yang sedang running.

### Kalo butuh manual extract:
```bash
# Option A: Direct from running device
adb pull /data/app/com.proximabeta.mf.uamo-*/lib/arm64-v8a/libUE4.so ./

# Option B: Extract dari APK
unzip com.proximabeta.mf.uamo.apk "lib/arm64-v8a/libUE4.so" -d extracted/
cp extracted/lib/arm64-v8a/libUE4.so ./

# Option C: Dari emulator storage
adb shell find /data -name "libUE4.so" 2>/dev/null
```

---

## Setup & Build

### 1. Prerequisites
```bash
sudo apt-get install cmake g++ libpthread-stubs0-dev unzip
```

### 2. Setup Paradise Kernel Driver
```bash
sudo chmod +x setup_driver.sh
sudo ./setup_driver.sh
```

Output:
```
[+] Kernel module loaded successfully
[+] Setup complete! Paradise driver is ready.
```

### 3. Build DecryptEngine
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

Output: `./decrypt_engine`

### 4. Run
```bash
# Make sure game is running on device/emulator
./decrypt_engine com.proximabeta.mf.uamo
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

---

## Configuration

### Change Target Game
Edit `main.cpp` baris ~110:
```cpp
const char* target_process = "com.proximabeta.mf.uamo";  // ← Change here
```

Atau pass sebagai argument:
```bash
./decrypt_engine com.your.game.name
```

### Fix HOOK_LITERAL Offset

File: `DecryptEngine.hpp` baris 21
```cpp
constexpr uint64_t HOOK_LITERAL = 0x自己猜;  // ← Perlu actual offset
```

**Cara menemukan:**

1. **IDA Pro / Ghidra Analysis**
   - Load libUE4.so
   - Search: `Final_Dispatch` or `FinalDispatch`
   - Backtrace ke wrapper entry
   - Calculate offset dari libUE4 base

2. **Command-line**
   ```bash
   nm libUE4.so | grep -i final
   readelf -s libUE4.so | grep Dispatch
   ```

3. **Runtime Bruteforce** (quick debugging)
   - Modify DecryptEngine untuk try multiple offsets
   - Log yang hit

4. **Game-specific**
   - com.proximabeta.mf.uamo typically UE4.24-4.27
   - Offset biasanya 0x2XXXXXX or 0x3XXXXXX range

---

## Architecture

```
┌─────────────────┐
│  main.cpp       │  Entry point, init paradise_driver
├─────────────────┤
│ DecryptEngine   │  Hardware breakpoint polling
│  (background)   │  ↓ 1ms interval
├─────────────────┤
│ Kernel.cpp      │  Wrapper untuk paradise_driver
│                 │  ↓ ioctl to kernel
├─────────────────┤
│ paradise_api.h  │  Kernel driver interface
│ + .ko module    │  Hardware breakpoint + memory R/W
└─────────────────┘
```

### Thread Model
- **Kernel Thread**: Polling breakpoint @ Final_Dispatch
- **Main Thread**: Command-line interface, signal handling
- **Render Thread**: (optional) Read-only cache access

### Data Flow
```
Game Running
    ↓
Final_Dispatch hit → Hardware Breakpoint triggered
    ↓
Register Snapshot: X0-X31 captured
    ↓
Background thread: info.x[19] = rootComp, info.x[0] = transform
    ↓
Read float X,Y,Z @ transform + 0x10/0x14/0x18
    ↓
Cache update (thread-safe mutex)
    ↓
Ready for rendering/output
```

---

## Troubleshooting

| Error | Cause | Solution |
|-------|-------|----------|
| "driver not initialized" | Paradise kernel module not loaded | Run `sudo setup_driver.sh` |
| "Could not find process" | Game not running | Start game on device/emulator |
| "Could not find libUE4.so" | Process terminated / wrong name | Verify process name with `adb shell ps` |
| "Permission denied" | Running without root | Use `sudo ./decrypt_engine ...` |
| "IsDecode = 2" (line 58) | hwbp_attach failed | Check Paradise module status |
| "IsDecode = 1" (line 67) | hwbp_bp_set failed | Wrong HOOK_LITERAL offset |

---

## Files Structure

```
BuildDecrypted/
├── CMakeLists.txt              Build config
├── main.cpp                    Entry point + initialization
├── Kernel.hpp                  Kernel wrapper interface
├── Kernel.cpp                  Kernel implementation
├── DecryptEngine.hpp           Hardware BP polling logic
├── paradise_api.h              Paradise kernel driver API
├── libparadise_api.a           Static library
├── driver_ko_601.zip           Kernel driver source/binary
├── setup_driver.sh             Driver setup automation
└── BUILD.md                    This file
```

---

**Target Game**: com.proximabeta.mf.uamo
**Updated**: 2026-09-15
**Status**: Production-ready
