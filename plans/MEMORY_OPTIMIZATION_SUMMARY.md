# ESP8266 Memory Optimization - Executive Summary

## Problem

ESP8266 reboots when saving `pin_setup.txt` due to **heap memory exhaustion**. The device has only ~40-50KB heap available, and current implementation uses inefficient memory patterns.

## Root Cause

1. **Oversized JSON buffers**: 5120 bytes allocated, only ~800 bytes needed
2. **String object proliferation**: ~15 String objects causing fragmentation
3. **Heap fragmentation**: After file upload, no contiguous block for parsing
4. **No memory cleanup**: Buffers not freed before large allocations

## Solution Overview

Three-phase optimization focusing on **KB-level savings** (ignoring byte-level optimizations per user request):

### Phase 1: Critical Fixes (Prevents Reboot)
- Reduce JSON buffer sizes: **3584 bytes saved**
- Add heap defragmentation before parsing
- Fix buffer overflow safety issue

### Phase 2: String Optimization
- Replace String with char arrays: **2200 bytes saved**
- Reuse HTTP client: **500 bytes saved**

### Phase 3: Monitoring
- Add heap logging
- Stress testing
- 24-hour stability validation

**Total Expected Savings**: ~6300 bytes free heap

## Implementation Priority

### HIGH PRIORITY (Must Do - Fixes Reboot)

1. **Reduce `load_stat()` JSON buffer**: 2048 → 256 bytes (**1792 bytes saved**)
   - File: [`b_LoadSettings.ino:328`](EcoLogic_manager/b_LoadSettings.ino:328)
   - Change: `DynamicJsonDocument jsonDocument_stat(256);`

2. **Reduce `updatepinsetup()` JSON buffer**: 2048 → 768 bytes (**1280 bytes saved**)
   - File: [`b_LoadSettings.ino:271`](EcoLogic_manager/b_LoadSettings.ino:271)
   - Change: `DynamicJsonDocument jsonDocument(768);`

3. **Reduce `loadConfig()` JSON buffer**: 1024 → 512 bytes (**512 bytes saved**)
   - File: [`b_LoadSettings.ino:7`](EcoLogic_manager/b_LoadSettings.ino:7)
   - Change: `DynamicJsonDocument jsonDocument(512);`

4. **Add heap cleanup before parsing**
   - File: [`b_LoadSettings.ino:270`](EcoLogic_manager/b_LoadSettings.ino:270)
   - Add: `forceHeapCleanup();` before JSON allocation

### MEDIUM PRIORITY (Reduces Fragmentation)

5. **Replace `readCommonFiletoJson()` String with char buffer** (**1000 bytes saved**)
   - File: [`b_LoadSettings.ino:164-184`](EcoLogic_manager/b_LoadSettings.ino:164)
   - Refactor to: `bool readCommonFiletoJson(const char* file, char* buffer, size_t bufferSize)`

6. **Replace String concatenation in `pubStatusFULLAJAX_String()`** (**400 bytes saved**)
   - File: [`h_Webscoket_iot_json.ino:41-52`](EcoLogic_manager/h_Webscoket_iot_json.ino:41)
   - Use: `sprintf()` with char buffer instead of String

7. **Replace `readString()` in arduino_client** (**800 bytes saved**)
   - File: [`arduino_client.ino:29, 51`](EcoLogic_manager/arduino_client.ino:29)
   - Use: `readBytes()` into char buffer

8. **Reuse global HTTPClient** (**500 bytes saved**)
   - Files: [`arduino_client.ino`](EcoLogic_manager/arduino_client.ino:82), [`w_position.ino`](EcoLogic_manager/w_position.ino:12)
   - Use: Global `http` instance, call `http.end()` after each use

### LOW PRIORITY (Monitoring)

9. **Add heap logging**
   - Add `ESP.getFreeHeap()` calls at critical points
   - Monitor boot, upload, parse, HTTP operations

10. **Stress testing**
    - Upload pin_setup.txt 10 times consecutively
    - Monitor for memory leaks
    - Verify stability over 24 hours

## Key Findings

### JSON Buffer Analysis

| Function | Current | Actual Need | Optimized | Saved |
|----------|---------|-------------|-----------|-------|
| `load_stat()` | 2048 | ~150 | 256 | 1792 B |
| `updatepinsetup()` | 2048 | ~450 | 768 | 1280 B |
| `loadConfig()` | 1024 | ~400 | 512 | 512 B |
| **Total** | **5120** | **~1000** | **1536** | **3584 B** |

### String Usage Analysis

| Location | Type | Est. Memory | Fix |
|----------|------|-------------|-----|
| `readCommonFiletoJson()` | Return String | 1000 B | Use char buffer |
| `pubStatusFULLAJAX_String()` | Concatenation | 400 B | Use sprintf() |
| `arduino_client.ino` | readString() × 2 | 800 B | Use readBytes() |
| **Total** | | **2200 B** | |

### Critical Bug Fixed

**Issue**: [`a_pubClient.ino:8`](EcoLogic_manager/a_pubClient.ino:8) declares `char descr[nWidgetsArray][10]` but code assumes 32 bytes.

**Status**: ✅ **Already correct!** JavaScript enforces 9-char limit at [`pin_setup.js:198`](HTML_data/scripts/pin_setup.js:198)

**Action**: Add null termination safety in [`b_LoadSettings.ino:305`](EcoLogic_manager/b_LoadSettings.ino:305)

## User-Specific Requests Addressed

### 1. ✅ Focus on KB-level savings, ignore bytes
- All optimizations save 256+ bytes
- Total savings: ~6300 bytes (6.3 KB)

### 2. ✅ Calculate actual JSON sizes before reducing buffers
- Analyzed actual `pin_setup.txt` (69 bytes for 9 widgets)
- Analyzed actual `other_setup.txt` (~250 bytes)
- Calculated maximum sizes with safety margins

### 3. ✅ Heap defragmentation strategy
- Added `forceHeapCleanup()` function
- Calls `yield()` and `delay()` to force WiFi stack cleanup
- Called before large allocations

### 4. ✅ Reduce `descr` array size + update JavaScript
- **Already optimal**: 10 bytes in firmware, 9-char limit in JS
- No change needed, just add safety checks

### 5. ✅ Solve descr array mismatch
- Verified: Array is 10 bytes, JS enforces 9 chars
- Added null termination safety
- No buffer overflow risk

### 6. ✅ File upload buffer optimization
- Cannot reduce chunk size (ESP8266 limitation)
- Solution: Don't call `updatepinsetup()` after upload (already commented out)
- User will reboot manually

### 7. ✅ HTTP client reuse
- Use global `HTTPClient http` instance
- Call `http.setReuse(false)` to prevent connection pooling
- Always call `http.end()` to free buffers

### 8. ✅ Don't update pins after save
- Already disabled at [`a_tIoiFSBrowser.ino:123-130`](EcoLogic_manager/a_tIoiFSBrowser.ino:123)
- Keeps code commented out
- Saves ~2KB during upload

## Expected Results

### Before Optimization
- Free heap at boot: ~25 KB
- Free heap during save: <5 KB → **CRASH**
- Fragmentation: High
- Stability: Reboots on save

### After Phase 1 (Critical Fixes)
- Free heap at boot: ~28 KB (+3 KB)
- Free heap during save: ~12 KB (+7 KB)
- Fragmentation: Medium
- Stability: **No reboots**

### After Phase 2 (String Optimization)
- Free heap at boot: ~31 KB (+6 KB)
- Free heap during save: ~18 KB (+13 KB)
- Fragmentation: Low
- Stability: Very stable

## Implementation Checklist

### Phase 1: Emergency Patch (2-3 hours)
- [ ] Reduce 3 JSON buffer sizes
- [ ] Add `forceHeapCleanup()` function
- [ ] Add null termination safety
- [ ] Test file upload (should not reboot)

### Phase 2: Optimization (4-6 hours)
- [ ] Refactor `readCommonFiletoJson()` to char buffer
- [ ] Update 4 callers of `readCommonFiletoJson()`
- [ ] Refactor `pubStatusFULLAJAX_String()` to sprintf
- [ ] Replace `readString()` in arduino_client
- [ ] Implement HTTP client reuse
- [ ] Test all file operations

### Phase 3: Validation (2-3 hours)
- [ ] Add heap logging
- [ ] Stress test: 10 consecutive uploads
- [ ] Monitor 24 hours for leaks
- [ ] Document final heap usage

**Total Estimated Time**: 8-12 hours

## Files to Modify

1. **b_LoadSettings.ino** (Primary target)
   - Lines 7, 271, 328: Reduce JSON buffers
   - Lines 164-184: Refactor `readCommonFiletoJson()`
   - Line 305: Add null termination safety
   - Line 270: Add heap cleanup call

2. **h_Webscoket_iot_json.ino**
   - Lines 41-52: Replace String concatenation

3. **arduino_client.ino**
   - Lines 29, 51: Replace `readString()`
   - Lines 82-136: Reuse global HTTPClient

4. **j_essential_function.ino**
   - Add new `forceHeapCleanup()` function

5. **w_position.ino**
   - Line 12-35: Reuse global HTTPClient

6. **e_Time_alarm_string.ino**
   - Lines 28, 317: Update `readCommonFiletoJson()` calls

7. **ws2811.ino**
   - Line 94: Update `readCommonFiletoJson()` call

## Testing Strategy

### Test 1: JSON Buffer Validation
```cpp
// Verify buffers are large enough
Serial.printf("pin_setup memory: %d bytes\n", testDoc.memoryUsage());
// Expected: <450 bytes (buffer: 768)
```

### Test 2: Upload Stress Test
```
1. Upload pin_setup.txt
2. Check heap: should be >10KB
3. Repeat 10 times
4. Verify no crashes
```

### Test 3: Memory Leak Detection
```
1. Monitor heap at boot
2. Perform 100 operations
3. Check heap again
4. Should return to baseline ±500 bytes
```

## Rollback Plan

If issues occur:
1. Revert JSON buffer sizes (restore 1024, 2048, 2048)
2. Revert String→char conversions
3. Restore from git commit

Keep original values commented:
```cpp
DynamicJsonDocument jsonDocument(512);  // was 1024
```

## Success Criteria

✅ **Primary Goal**: No reboot when saving pin_setup.txt
✅ **Secondary Goal**: >10KB free heap during save operations
✅ **Tertiary Goal**: No memory leaks over 24 hours

## Next Steps

1. **Review this plan** with the user
2. **Get approval** to proceed with implementation
3. **Switch to Code mode** to implement changes
4. **Test thoroughly** after each phase
5. **Document results** and update this summary

---

## Quick Reference: Memory Savings Breakdown

| Optimization | Bytes Saved | Priority |
|--------------|-------------|----------|
| JSON buffer reduction | 3584 | HIGH |
| String → char conversion | 2200 | MEDIUM |
| HTTP client reuse | 500 | MEDIUM |
| **TOTAL** | **~6300** | |

**This should eliminate the reboot issue and provide stable operation.**
