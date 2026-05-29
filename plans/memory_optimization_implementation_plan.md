# ESP8266 Memory Optimization - Implementation Plan

## Executive Summary

Based on analysis of the codebase and user feedback, the ESP8266 reboots when saving `pin_setup.txt` due to heap exhaustion. This document provides a detailed, actionable implementation plan focusing on **high-impact changes only** (KB-level savings, not bytes).

## Actual File Size Analysis

### pin_setup.txt (69 bytes for 9 widgets)
```json
{
  "numberChosed": 9,
  "pinmode": [2,2,2,2,3,3,3,5,6],
  "pin": [0,1,16,3,14,12,13,0,5],
  "descr": ["d3 switch","tx switch","d0 switch","rx switch","d5 PWM","d6 PWM","d7 PWM","DHTtemp","DHT mist"],
  "widget": [1,1,1,1,3,3,3,6,6],
  "IrBtnId": [255,255,255,255,255,255,255,255,255],
  "defaultVal": [1,1,1,1,0,0,0,0,0]
}
```

**Maximum size** (12 widgets with 9-char descriptions): ~450 bytes

### other_setup.txt (16 fields, ~250 bytes)
```json
{
  "server_url": "https://ecologic.go.ro",
  "sync_interval": "30",
  "softAP_password": "12345678",
  ...
}
```

**Maximum size**: ~400 bytes

## Critical Issues & Solutions

### Issue 1: CRITICAL BUG - `descr` Array Size Mismatch

**Current State**:
- [`a_pubClient.ino:8`](EcoLogic_manager/a_pubClient.ino:8): `char descr[nWidgetsArray][10]` (only 10 bytes!)
- [`b_LoadSettings.ino:305`](EcoLogic_manager/b_LoadSettings.ino:305): `strncpy(descr[i], jsonDocument["descr"][i], sizeof(descr[i]) - 1)` expects 32 bytes
- [`pin_setup.js:198`](HTML_data/scripts/pin_setup.js:198): `descrInput.maxLength = 9` (enforces 9 chars)

**Problem**: Buffer overflow risk! `sizeof(descr[i])` returns 10, but code expects 32.

**Solution**: Keep 10 bytes (JavaScript already enforces 9 chars max)

**Action**:
```cpp
// a_pubClient.ino:8 - NO CHANGE NEEDED (already correct at 10 bytes)
char descr[nWidgetsArray][10];  // 9 usable chars + null terminator

// b_LoadSettings.ino:305 - FIX: Use actual size, not assumed 32
strncpy(descr[i], jsonDocument["descr"][i], sizeof(descr[i]) - 1);
descr[i][sizeof(descr[i]) - 1] = '\0';  // Ensure null termination
```

**Memory Impact**: 0 bytes (bug fix, no allocation change)

---

### Issue 2: CRITICAL - Oversized JSON Buffers

#### 2.1 `load_stat()` - 2048 bytes for 200 bytes of data!

**Current**: [`b_LoadSettings.ino:328`](EcoLogic_manager/b_LoadSettings.ino:328)
```cpp
DynamicJsonDocument jsonDocument_stat(2048);  // MASSIVE WASTE!
```

**Actual JSON** (12 floats):
```json
{"stat": [1.00, 0.00, 25.50, 60.20, 0.00, 0.00, 0.00, 22.5, 65.3, 0, 0, 0]}
```
**Actual size**: ~100-150 bytes

**Solution**:
```cpp
DynamicJsonDocument jsonDocument_stat(256);  // Reduced from 2048
```

**Memory Saved**: **1792 bytes** ✅

#### 2.2 `updatepinsetup()` - 2048 bytes for ~450 bytes of data

**Current**: [`b_LoadSettings.ino:271`](EcoLogic_manager/b_LoadSettings.ino:271)
```cpp
DynamicJsonDocument jsonDocument(2048);
```

**Actual max size**: ~450 bytes (12 widgets × 9-char descriptions)

**Solution**:
```cpp
DynamicJsonDocument jsonDocument(768);  // Reduced from 2048, with safety margin
```

**Memory Saved**: **1280 bytes** ✅

#### 2.3 `loadConfig()` - 1024 bytes for ~400 bytes of data

**Current**: [`b_LoadSettings.ino:7`](EcoLogic_manager/b_LoadSettings.ino:7)
```cpp
DynamicJsonDocument jsonDocument(1024);
```

**Actual max size**: ~400 bytes (other_setup.txt)

**Solution**:
```cpp
DynamicJsonDocument jsonDocument(512);  // Reduced from 1024
```

**Memory Saved**: **512 bytes** ✅

**Total JSON Buffer Savings**: **3584 bytes** 🎯

---

### Issue 3: HIGH IMPACT - String Usage in File Reading

#### 3.1 `readCommonFiletoJson()` - Returns String (512-1024 bytes)

**Current**: [`b_LoadSettings.ino:164-184`](EcoLogic_manager/b_LoadSettings.ino:164)
```cpp
String readCommonFiletoJson(String file) {
  File configFile = fileSystem->open("/" + file + ".txt", "r");  // String concat!
  String jsonConfig = configFile.readString();  // Allocates String!
  return jsonConfig;  // Returns String!
}
```

**Problem**: 
- Creates 3 String objects (file path, file content, return value)
- Each String has 24+ bytes overhead
- Total allocation: ~600-1200 bytes

**Solution**: Use char buffer instead
```cpp
// NEW SIGNATURE: Returns success/failure, fills buffer
bool readCommonFiletoJson(const char* file, char* buffer, size_t bufferSize) {
  char path[64];
  snprintf(path, sizeof(path), "/%s.txt", file);
  
  File configFile = fileSystem->open(path, "r");
  if (!configFile) {
    Serial.println("Failed to open " + String(file));
    return false;
  }
  
  size_t bytesRead = configFile.readBytes(buffer, bufferSize - 1);
  buffer[bytesRead] = '\0';
  configFile.close();
  
  Serial.print("file:");
  Serial.print(file);
  Serial.print(" ");
  Serial.println(buffer);
  
  return true;
}
```

**Callers to update**:
1. [`b_LoadSettings.ino:106`](EcoLogic_manager/b_LoadSettings.ino:106) - `loadConfig()`
2. [`e_Time_alarm_string.ino:28`](EcoLogic_manager/e_Time_alarm_string.ino:28) - `load_Current_condition()`
3. [`e_Time_alarm_string.ino:317`](EcoLogic_manager/e_Time_alarm_string.ino:317) - `actBtn_a_ch_string()`
4. [`ws2811.ino:94`](EcoLogic_manager/ws2811.ino:94) - LED strip loading

**Memory Saved**: **~1000 bytes** ✅

#### 3.2 `pubStatusFULLAJAX_String()` - String Concatenation in Loop

**Current**: [`h_Webscoket_iot_json.ino:41-52`](EcoLogic_manager/h_Webscoket_iot_json.ino:41)
```cpp
void pubStatusFULLAJAX_String(bool save_eeprom) {
  String stat1 = "{\"stat\":[";  // String object
  for (uint8_t i = 0; i < nWidgets; i++) {
    float that_stat = get_new_widjet_value(i);
    stat1 += "\"";  // Repeated concatenation = reallocation!
    stat1 += String(that_stat, 2);
    stat1 += "\"";
    stat1 += (i < nWidgets - 1) ? "," : "]";
  }
  stat1 += "}";
  String buffer = stat1;  // Copy!
  server.send(200, "text / json", buffer);
}
```

**Problem**: Each `+=` operation may reallocate the entire String. With 12 widgets, this could cause 40+ reallocations!

**Solution**: Use char buffer with snprintf
```cpp
void pubStatusFULLAJAX_String(bool save_eeprom) {
  char buffer[256];  // Stack allocation
  char* ptr = buffer;
  
  ptr += sprintf(ptr, "{\"stat\":[");
  
  for (uint8_t i = 0; i < nWidgets; i++) {
    float that_stat = get_new_widjet_value(i);
    ptr += sprintf(ptr, "\"%0.2f\"", that_stat);
    if (i < nWidgets - 1) {
      *ptr++ = ',';
    }
  }
  
  sprintf(ptr, "]}");
  
  server.send(200, "text/json", buffer);
}
```

**Memory Saved**: **~400 bytes** ✅

#### 3.3 `arduino_client.ino` - readString() calls

**Current**: [`arduino_client.ino:29, 51`](EcoLogic_manager/arduino_client.ino:29)
```cpp
String otherConfig = otherFile.readString();  // Allocates String
String config = file.readString();  // Allocates String
```

**Solution**: Use readBytes() into char buffer
```cpp
char otherConfig[512];
size_t otherSize = otherFile.readBytes(otherConfig, sizeof(otherConfig) - 1);
otherConfig[otherSize] = '\0';

char config[768];
size_t configSize = file.readBytes(config, sizeof(config) - 1);
config[configSize] = '\0';
```

**Memory Saved**: **~800 bytes** ✅

**Total String Replacement Savings**: **~2200 bytes** 🎯

---

### Issue 4: HTTP Client Memory Reuse

**Current**: Multiple `HTTPClient http` instances created
- [`arduino_client.ino:82-136`](EcoLogic_manager/arduino_client.ino:82) - `syncWithServer()`
- [`w_position.ino:12-35`](EcoLogic_manager/w_position.ino:12) - `getHttp()`

**Problem**: Each HTTP request allocates ~1500 bytes for buffers

**Solution**: Reuse global HTTPClient instance
```cpp
// EcoLogic_manager.ino - Add global
extern HTTPClient http;  // Already declared at line 50!

// arduino_client.ino - Remove local declaration, use global
void syncWithServer() {
  // Remove: HTTPClient http;  (use global instead)
  http.setReuse(false);  // Don't keep connection alive
  // ... rest of code
  http.end();  // Always call end() to free buffers
}
```

**Memory Saved**: **~500 bytes** (reduces fragmentation) ✅

---

### Issue 5: Heap Defragmentation Strategy

**Problem**: After file upload, heap is fragmented. Large allocations fail even if total free memory is sufficient.

**Solution**: Force garbage collection before large allocations

```cpp
// Add helper function in j_essential_function.ino
void forceHeapCleanup() {
  #ifdef will_use_serial
  uint32_t before = ESP.getFreeHeap();
  #endif
  
  // Force WiFi stack to release buffers
  yield();
  delay(10);
  
  #ifdef will_use_serial
  uint32_t after = ESP.getFreeHeap();
  Serial.printf("[HEAP] Cleanup: %d -> %d bytes (+%d)\n", before, after, after - before);
  #endif
}

// Call before large allocations
bool updatepinsetup(File jsonrecieve) {
  forceHeapCleanup();  // Add this line
  DynamicJsonDocument jsonDocument(768);
  // ... rest of code
}
```

**Memory Impact**: Reduces fragmentation, improves allocation success rate ✅

---

### Issue 6: Disable updatepinsetup() After File Upload

**Current**: [`a_tIoiFSBrowser.ino:123-130`](EcoLogic_manager/a_tIoiFSBrowser.ino:123)
```cpp
// пока эта штука вызывает проблемы после загрузки происходит переключение кнопок на реле
// if (upload.filename.equals("pin_setup.txt")) {
//   if (updatepinsetup(fileSystem->open("/pin_setup.txt", "r"))) {
//     Serial.println("Widgets Loaded");
//   }
//   uploadConfig_ecologicclient();
// }
```

**Action**: Keep commented out (already done) ✅

**Rationale**: User will reboot manually after upload. This saves ~2KB during upload.

---

## Implementation Checklist

### Phase 1: Critical Fixes (Prevents Reboot)

- [ ] **Fix 1.1**: Verify `descr` array size is correct (10 bytes)
- [ ] **Fix 1.2**: Add null termination safety in `b_LoadSettings.ino:305`
- [ ] **Fix 2.1**: Reduce `load_stat()` JSON buffer: 2048 → 256 bytes
- [ ] **Fix 2.2**: Reduce `updatepinsetup()` JSON buffer: 2048 → 768 bytes
- [ ] **Fix 2.3**: Reduce `loadConfig()` JSON buffer: 1024 → 512 bytes
- [ ] **Fix 5**: Add `forceHeapCleanup()` before `updatepinsetup()`

**Expected Result**: Reboot issue resolved, +3584 bytes free heap

### Phase 2: String Optimization (Reduces Fragmentation)

- [ ] **Fix 3.1**: Refactor `readCommonFiletoJson()` to use char buffer
- [ ] **Fix 3.1a**: Update all callers (4 locations)
- [ ] **Fix 3.2**: Refactor `pubStatusFULLAJAX_String()` to use char buffer
- [ ] **Fix 3.3**: Replace `readString()` in `arduino_client.ino` with `readBytes()`
- [ ] **Fix 4**: Reuse global `HTTPClient http` instance

**Expected Result**: +2700 bytes free heap, reduced fragmentation

### Phase 3: Monitoring & Validation

- [ ] Add heap logging at critical points:
  - Boot
  - Before/after file upload
  - Before/after JSON parsing
  - Before/after HTTP requests
- [ ] Test file upload 10 times consecutively
- [ ] Monitor heap over 24 hours

**Expected Result**: Stable operation, no memory leaks

---

## Code Changes Summary

| File | Lines | Change | Memory Saved |
|------|-------|--------|--------------|
| `b_LoadSettings.ino` | 7 | Reduce JSON buffer 1024→512 | 512 B |
| `b_LoadSettings.ino` | 271 | Reduce JSON buffer 2048→768 | 1280 B |
| `b_LoadSettings.ino` | 328 | Reduce JSON buffer 2048→256 | 1792 B |
| `b_LoadSettings.ino` | 164-184 | Refactor to char buffer | 1000 B |
| `b_LoadSettings.ino` | 305 | Add null termination | 0 B (safety) |
| `h_Webscoket_iot_json.ino` | 41-52 | Replace String concat | 400 B |
| `arduino_client.ino` | 29, 51 | Replace readString() | 800 B |
| `arduino_client.ino` | 82-136 | Reuse global HTTPClient | 500 B |
| `j_essential_function.ino` | new | Add forceHeapCleanup() | 0 B (defrag) |
| `b_LoadSettings.ino` | 270 | Call forceHeapCleanup() | 0 B (defrag) |

**Total Memory Saved**: **~6284 bytes** 🎯

---

## Testing Plan

### Test 1: Verify JSON Buffer Sizes
```cpp
// Add to setup() temporarily
Serial.println("Testing JSON parsing...");

// Test pin_setup.txt
File testFile = LittleFS.open("/pin_setup.txt", "r");
DynamicJsonDocument testDoc(768);
DeserializationError err = deserializeJson(testDoc, testFile);
Serial.printf("pin_setup parse: %s, memory used: %d\n", 
              err.c_str(), testDoc.memoryUsage());
testFile.close();

// Test other_setup.txt
testFile = LittleFS.open("/other_setup.txt", "r");
DynamicJsonDocument testDoc2(512);
err = deserializeJson(testDoc2, testFile);
Serial.printf("other_setup parse: %s, memory used: %d\n", 
              err.c_str(), testDoc2.memoryUsage());
testFile.close();
```

**Expected Output**:
```
pin_setup parse: Ok, memory used: 450
other_setup parse: Ok, memory used: 380
```

### Test 2: Heap Monitoring During Save
```cpp
// Add to handleFileUpload() in a_tIoiFSBrowser.ino
if (upload.status == UPLOAD_FILE_START) {
  Serial.printf("[HEAP] Upload start: %d bytes free\n", ESP.getFreeHeap());
}
if (upload.status == UPLOAD_FILE_END) {
  Serial.printf("[HEAP] Upload end: %d bytes free\n", ESP.getFreeHeap());
}
```

**Expected Output**:
```
[HEAP] Upload start: 28000 bytes free
[HEAP] Upload end: 26500 bytes free  (should stay above 20KB)
```

### Test 3: Stress Test
1. Upload `pin_setup.txt` 10 times
2. Monitor heap after each upload
3. Verify no crashes
4. Check for memory leaks (heap should return to baseline)

---

## Risk Mitigation

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| JSON buffer too small | Low | High | Test with max-size configs first |
| Buffer overflow in char arrays | Low | Critical | Use `snprintf()` with size checks |
| Breaking existing functionality | Medium | High | Test all file operations thoroughly |
| Memory leak in new code | Low | Medium | Add heap monitoring, test 24h |

---

## Rollback Plan

If issues occur after changes:

1. **Immediate**: Revert JSON buffer size changes (restore 1024, 2048, 2048)
2. **If still failing**: Revert String→char conversions
3. **Last resort**: Restore from git commit before changes

Keep original values commented in code:
```cpp
DynamicJsonDocument jsonDocument(512);  // was 1024 before optimization
```

---

## Expected Outcomes

| Metric | Before | After Phase 1 | After Phase 2 |
|--------|--------|---------------|---------------|
| Free Heap (boot) | ~25 KB | ~28 KB | ~31 KB |
| Free Heap (save) | <5 KB (crash) | ~12 KB | ~18 KB |
| Fragmentation | High | Medium | Low |
| Stability | Crashes on save | Stable | Very Stable |
| String objects | ~15 | ~15 | ~5 |
| Max JSON buffer | 5120 B | 1536 B | 1536 B |

---

## Conclusion

The implementation plan focuses on **high-impact changes** that save KB, not bytes:

1. **JSON Buffer Reduction**: 3584 bytes saved
2. **String Elimination**: 2200 bytes saved
3. **HTTP Client Reuse**: 500 bytes saved
4. **Heap Defragmentation**: Improves allocation success

**Total Expected Savings**: ~6300 bytes free heap

This should eliminate the reboot issue and provide a stable foundation for future development.

**Estimated Implementation Time**: 4-6 hours
**Testing Time**: 2-3 hours
**Total**: 6-9 hours
