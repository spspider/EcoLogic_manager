/*
 * Computer Power Control Module
 *
 * Purpose: Monitor and control computer power state via GPIO pins
 * Trigger: Automatically reboot computer when it becomes unreachable via ping
 *
 * Hardware Requirements:
 * - Input pin (optional): Detects computer power state (HIGH = on, LOW = off) - informative only
 * - Output pin (required): Controls computer power button
 *
 * Timing:
 * - Power ON: Quick pulse (100ms HIGH)
 * - Force shutdown: Hold HIGH for 10 seconds
 * - Wait after shutdown: 10 seconds before attempting power on
 * - Wait after boot: 30 minutes for HDD check and system stabilization
 * - Initial boot wait: 30 minutes on program startup (computer may be booting)
 *
 * Memory optimization:
 * - Uses F() macro for all Serial strings (stores in flash, not RAM)
 * - Minimal global variables (only state tracking)
 * - No dynamic memory allocation
 */

// Generic device log, kept outside USE_COMPUTER_POWER_CONTROL so any module can use it
// and the log page/API works even when no feature define is active.
// Stored on flash (not RAM) as plain-text lines: "<seconds>|<type>|<message>".
// Plain text is used instead of JSON-per-line because it has no repeated key/quote
// overhead, keeping the capped file noticeably smaller on the flash-limited ESP8266.
// The UI (computer_power_logs.htm) fetches the raw file and parses it client-side;
// the device never needs to hold the whole log in memory.
#define LOG_FILE_PATH "/computer_power_log.txt"
#define LOG_FILE_TMP_PATH "/computer_power_log.tmp"
#define MAX_LOG_LINES 40

/**
 * Create the log file on first boot so the UI never sees a 404 before any
 * log line has been written.
 */
void initLogs() {
  if (!fileSystem->exists(LOG_FILE_PATH)) {
    File logFile = fileSystem->open(LOG_FILE_PATH, "w");
    if (logFile) logFile.close();
  }
}

/**
 * Append a log entry to the log file, evicting the oldest line(s) once the
 * line cap is reached so the file never grows unbounded.
 */
void addLogEntry(const char* message, const char* logType) {
  Serial.print(F("[LOG] "));
  Serial.print(logType);
  Serial.print(F(": "));
  Serial.println(message);

  // Count existing lines to know whether the oldest one(s) must be dropped
  uint16_t existingLines = 0;
  File countFile = fileSystem->open(LOG_FILE_PATH, "r");
  if (countFile) {
    while (countFile.available()) {
      countFile.readStringUntil('\n');
      existingLines++;
    }
    countFile.close();
  }

  if (existingLines >= MAX_LOG_LINES) { // Rewrite file without the oldest line(s)
    File src = fileSystem->open(LOG_FILE_PATH, "r");
    File dst = fileSystem->open(LOG_FILE_TMP_PATH, "w");
    if (src && dst) {
      uint16_t toSkip = existingLines - MAX_LOG_LINES + 1;
      uint16_t idx = 0;
      while (src.available()) {
        String line = src.readStringUntil('\n');
        line.trim();
        idx++;
        if (idx <= toSkip || line.length() == 0) continue;
        dst.print(line);
        dst.print('\n');
      }
    }
    if (src) src.close();
    if (dst) dst.close();
    fileSystem->remove(LOG_FILE_PATH);
    fileSystem->rename(LOG_FILE_TMP_PATH, LOG_FILE_PATH);
  }

  File logFile = fileSystem->open(LOG_FILE_PATH, "a");
  if (logFile) {
    logFile.print(millis() / 1000);
    logFile.print('|');
    logFile.print(logType);
    logFile.print('|');
    logFile.print(message);
    logFile.print('\n');
    logFile.close();
  }
}

#ifdef USE_COMPUTER_POWER_CONTROL

// Pin definitions - configure in EcoLogic_manager.ino
#ifndef COMPUTER_STATUS_PIN
  #define COMPUTER_STATUS_PIN 5  // Input: reads computer power state (informative, optional connection)
#endif

#ifndef COMPUTER_POWER_BUTTON_PIN
  #define COMPUTER_POWER_BUTTON_PIN 14  // Output: controls power button
#endif

// Target computer to monitor
#ifndef COMPUTER_IP
  #define COMPUTER_IP "192.168.1.160"
#endif

// Timing constants (milliseconds)
#define POWER_ON_PULSE_MS 100        // Quick pulse to power on
#define FORCE_SHUTDOWN_MS 10000      // Hold for 10 seconds to force shutdown
#define WAIT_AFTER_SHUTDOWN_MS 10000 // Wait 10 seconds after shutdown
#define WAIT_AFTER_BOOT_MS 1800000   // Wait 30 minutes (30 * 60 * 1000) after boot for HDD check
#define PING_CHECK_INTERVAL_MS 60000 // Check every 60 seconds
#define PING_TIMEOUT_MS 5000         // Ping timeout
#define PING_FAIL_THRESHOLD 3        // Consecutive failures before action

// State machine states
enum ComputerControlState {
  STATE_INITIAL_BOOT_WAIT,  // Wait on startup for computer to boot
  STATE_IDLE,               // Normal operation, periodic ping checks
  STATE_PINGING,            // Actively checking if computer responds
  STATE_SHUTTING_DOWN,      // Sending shutdown signal
  STATE_WAITING_SHUTDOWN,   // Waiting for shutdown to complete
  STATE_POWERING_ON,        // Sending power on pulse
  STATE_WAITING_BOOT        // Waiting for boot and HDD check
};

// Global state variables (minimal memory footprint)
static ComputerControlState currentState = STATE_INITIAL_BOOT_WAIT;
static unsigned long stateStartTime = 0;
static unsigned long lastPingCheck = 0;
static uint8_t consecutivePingFailures = 0;  // uint8_t instead of int saves RAM
static bool lastComputerStatus = false;

// Function prototypes
void setupComputerPowerControl();
void loopComputerPowerControl();
bool isComputerOn();
bool pingComputer(const char* ip);
void pressComputerPowerButton(unsigned long duration);
void handleComputerControlStateMachine();

/**
 * Initialize computer power control module
 */
void setupComputerPowerControl() { // Initialize pins and state
  pinMode(COMPUTER_STATUS_PIN, INPUT);
  pinMode(COMPUTER_POWER_BUTTON_PIN, OUTPUT);
  digitalWrite(COMPUTER_POWER_BUTTON_PIN, LOW);
  
  Serial.println(F("Computer Power Control initialized"));
  Serial.print(F("  Status pin (informative): "));
  Serial.println(COMPUTER_STATUS_PIN);
  Serial.print(F("  Power button pin: "));
  Serial.println(COMPUTER_POWER_BUTTON_PIN);
  Serial.print(F("  Target IP: "));
  Serial.println(COMPUTER_IP);
  Serial.println(F("  Waiting 30 minutes for initial computer boot..."));
  
  // Start in initial boot wait state
  currentState = STATE_INITIAL_BOOT_WAIT;
  stateStartTime = millis();
  lastPingCheck = millis();
  consecutivePingFailures = 0;
}

/**
 * Main loop - call this from main loop()
 */
void loopComputerPowerControl() { // Process state machine each loop iteration
  handleComputerControlStateMachine();
}

/**
 * Read computer power status from input pin (informative only, may not be connected)
 * @return true if computer is on, false if off
 */
bool isComputerOn() { // Read digital pin status
  return digitalRead(COMPUTER_STATUS_PIN) == HIGH;
}

/**
 * Ping target computer to check if it's reachable
 * @param ip IP address to ping (e.g., "192.168.1.160")
 * @return true if ping successful, false otherwise
 */
bool pingComputer(const char* ip) { // Attempt TCP connection on port 80 to test reachability
  WiFiClient client;
  
  Serial.print(F("Pinging "));
  Serial.print(ip);
  Serial.print(F("..."));
  
  // ESP8266 WiFiClient::connect() has no timeout overload; cap read timeout only
  client.setTimeout(PING_TIMEOUT_MS);
  bool result = client.connect(ip, 80);
  
  if (result) { // Connection successful
    Serial.println(F(" OK"));
    char msg[100];
    snprintf(msg, sizeof(msg), "Ping %s - SUCCESS", ip);
    addLogEntry(msg, "success");
    client.stop();
  } else { // Connection failed
    Serial.println(F(" FAILED"));
    char msg[100];
    snprintf(msg, sizeof(msg), "Ping %s - FAILED", ip);
    addLogEntry(msg, "failure");
  }
  
  return result;
}


/**
 * Simulate pressing computer power button
 * @param duration How long to hold the button (milliseconds, e.g., 100 for power on, 10000 for force shutdown)
 */
void pressComputerPowerButton(unsigned long duration) { // Send HIGH pulse for specified duration
  Serial.print(F("Pressing power button for "));
  Serial.print(duration);
  Serial.println(F("ms"));
  
  digitalWrite(COMPUTER_POWER_BUTTON_PIN, HIGH);
  delay(duration);
  digitalWrite(COMPUTER_POWER_BUTTON_PIN, LOW);
}

/**
 * State machine for computer control logic
 */
void handleComputerControlStateMachine() { // Main logic controller, processes current state
  unsigned long currentMillis = millis();
  bool computerStatus = isComputerOn();  // Read status pin (informative only)
  
  // Log status changes (informative - not used for control logic)
  if (computerStatus != lastComputerStatus) { // Status pin changed
    Serial.print(F("Computer status pin: "));
    Serial.println(computerStatus ? F("ON") : F("OFF"));
    lastComputerStatus = computerStatus;
  }
  
  switch (currentState) {
    case STATE_INITIAL_BOOT_WAIT: { // Wait 30 minutes on program start for computer to boot
      if (currentMillis - stateStartTime >= WAIT_AFTER_BOOT_MS) { // Initial wait complete
        Serial.println(F("Initial boot wait complete - starting normal operation"));
        currentState = STATE_IDLE;
        lastPingCheck = currentMillis;
      } else { // Still waiting
        // Print progress every minute
        unsigned long elapsed = currentMillis - stateStartTime;
        static unsigned long lastProgress = 0;
        if (elapsed - lastProgress >= 60000) { // Every 60 seconds
          lastProgress = elapsed;
          Serial.print(F("Initial boot wait: "));
          Serial.print(elapsed / 60000);
          Serial.print(F("/"));
          Serial.print(WAIT_AFTER_BOOT_MS / 60000);
          Serial.println(F(" minutes"));
        }
      }
      break;
    }
      
    case STATE_IDLE: { // Normal operation, check if it's time to ping
      if (currentMillis - lastPingCheck >= PING_CHECK_INTERVAL_MS) { // 60 seconds elapsed
        lastPingCheck = currentMillis;
        currentState = STATE_PINGING;
      }
      break;
    }
      
    case STATE_PINGING: { // Actively checking computer connectivity
      if (pingComputer(COMPUTER_IP)) { // Computer responded
        consecutivePingFailures = 0;
        currentState = STATE_IDLE;
      } else { // Ping failed
        consecutivePingFailures++;
        Serial.print(F("Consecutive ping failures: "));
        Serial.println(consecutivePingFailures);
        char msg[100];
        snprintf(msg, sizeof(msg), "Consecutive failures: %d/%d", consecutivePingFailures, PING_FAIL_THRESHOLD);
        addLogEntry(msg, "wait");
        
        if (consecutivePingFailures >= PING_FAIL_THRESHOLD) { // 3 failures, initiate reboot
          Serial.println(F("Computer not responding - initiating hard reboot"));
          addLogEntry("Threshold reached - initiating hard reboot", "action");
          consecutivePingFailures = 0;
          currentState = STATE_SHUTTING_DOWN;
          stateStartTime = currentMillis;
        } else { // Wait for next ping interval
          currentState = STATE_IDLE;
        }
      }
      break;
    }
      
    case STATE_SHUTTING_DOWN: { // Force shutdown by holding power button for 10 seconds
      pressComputerPowerButton(FORCE_SHUTDOWN_MS);
      Serial.println(F("Force shutdown initiated"));
      addLogEntry("Force shutdown - holding power button 10s", "action");
      currentState = STATE_WAITING_SHUTDOWN;
      stateStartTime = currentMillis;
      break;
    }
      
    case STATE_WAITING_SHUTDOWN: { // Wait 10 seconds after shutdown
      if (currentMillis - stateStartTime >= WAIT_AFTER_SHUTDOWN_MS) { // Shutdown wait complete
        Serial.println(F("Shutdown wait complete - powering on"));
        addLogEntry("Shutdown complete - initiating power on", "action");
        currentState = STATE_POWERING_ON;
      }
      break;
    }
      
    case STATE_POWERING_ON: { // Quick 100ms pulse to power on
      pressComputerPowerButton(POWER_ON_PULSE_MS);
      Serial.println(F("Power on pulse sent"));
      addLogEntry("Power ON pulse sent (100ms)", "action");
      currentState = STATE_WAITING_BOOT;
      stateStartTime = currentMillis;
      break;
    }
      
    case STATE_WAITING_BOOT: { // Wait 30 minutes for system boot and HDD check
      if (currentMillis - stateStartTime >= WAIT_AFTER_BOOT_MS) { // Boot wait complete
        Serial.println(F("Boot wait complete - resuming normal operation"));
        addLogEntry("Boot wait complete - resuming monitoring", "action");
        currentState = STATE_IDLE;
        lastPingCheck = currentMillis;
      } else { // Still waiting for boot
        // Print progress every minute
        unsigned long elapsed = currentMillis - stateStartTime;
        static unsigned long lastProgress = 0;
        if (elapsed - lastProgress >= 60000) { // Every 60 seconds
          lastProgress = elapsed;
          Serial.print(F("Waiting for boot: "));
          Serial.print(elapsed / 60000);
          Serial.print(F("/"));
          Serial.print(WAIT_AFTER_BOOT_MS / 60000);
          Serial.println(F(" minutes"));
        }
      }
      break;
    }
  }
}

#endif // USE_COMPUTER_POWER_CONTROL
