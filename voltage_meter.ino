/*
 * ============================================================================
 * Battery Cutoff Monitor for 12.8V LiFePO4 (4S)
 * ============================================================================
 * 
 * PURPOSE:
 * Protects a 12.8V LiFePO4 battery by disconnecting a sensor load at low 
 * voltage and reconnecting after battery recovery. Hosts a web interface
 * showing live battery stats.
 * 
 * FEATURES:
 * - Measures battery voltage via voltage divider into ADC1 pin (GPIO36/VP)
 * - Controls NC relay to disconnect load at low voltage
 * - Hysteresis prevents relay chatter (different ON/OFF thresholds)
 * - Web interface with live updates
 * - JSON API endpoint for external monitoring
 * - Serial output for debugging (CSV format)
 * 
 * HARDWARE REQUIREMENTS:
 * - ESP32 Development Board
 * - 12.8V (4S) LiFePO4 battery
 * - DC-DC step-down converter (12V to 5V)
 * - Relay module (NC contact, typically active-LOW)
 * - Resistors for voltage divider (10kΩ + 1kΩ recommended)
 * - Capacitor for ADC filtering (0.1µF to 1µF)
 * 
 * WIRING SUMMARY:
 * 
 * Voltage Divider (Battery Measurement):
 *   Battery+ → RTOP → ADC_NODE → RBOT → GND
 *   ADC_NODE → GPIO36 (VP)
 *   Capacitor from ADC_NODE to GND (0.1µF - 1µF)
 * 
 * Relay Control (NO - Normally Open):
 *   DC-DC Output+ → Relay COM
 *   Relay NO → Load+ (Sensor+)
 *   DC-DC GND → Load GND and ESP32 GND (common ground)
 * 
 * ESP32 Power (BEFORE relay cutoff):
 *   DC-DC Output+ → ESP32 VIN/5V
 *   DC-DC GND → ESP32 GND
 * 
 * Relay Signal:
 *   ESP32 GPIO2 → Relay IN
 *   ESP32 GND → Relay GND
 * 
 * IMPORTANT: All grounds must be connected together (common ground)!
 * 
 * ============================================================================
 */

#include <WiFi.h>
#include <WebServer.h>

// ============================================================================
// CONFIGURATION SECTION - MODIFY THESE VALUES FOR YOUR SETUP
// ============================================================================

// WiFi Credentials
// Replace with your WiFi network name and password
const char* WIFI_SSID = "ObieConnect";
const char* WIFI_PASS = "122ElmStreet";

// Pin Assignments
const int ADC_PIN = 36;      // GPIO36 (VP) - ADC1_CH0, works with WiFi enabled
const int RELAY_PIN = 2;     // GPIO2 - Controls relay module

// Relay Configuration
// Most relay modules are "active LOW" - setting pin LOW energizes the coil
// If your relay works opposite (HIGH = ON), set this to false
const bool RELAY_ACTIVE_LOW = true;

// Voltage Divider Configuration
// IMPORTANT: Choose resistor values that keep ADC voltage ≤ 3.3V
// 
// Recommended combinations for 12V LiFePO4:
// - Option A (best): RTOP=10kΩ, RBOT=1kΩ  → divides by 11, max input ~36V
// - Option B (okay): RTOP=100kΩ, RBOT=10kΩ → divides by 11, max input ~36V
// 
// Formula: Vadc = Vbat × (RBOT / (RTOP + RBOT))
// For safety: Vadc_max should be < 3.3V when Vbat is at maximum (14.6V)
//
const float RTOP = 10000.0;   // Top resistor (Battery+ to ADC node) in Ohms
const float RBOT = 1000.0;    // Bottom resistor (ADC node to GND) in Ohms

// ADC Configuration
const float VREF = 3.3;       // ESP32 reference voltage (typically 3.3V)
const int ADC_MAX = 4095;     // 12-bit ADC resolution (0-4095)
const int SAMPLES = 32;       // Number of samples to average for stable reading

// ============================================================================
// BATTERY PROTECTION THRESHOLDS - CUSTOMIZE THESE FOR YOUR NEEDS
// ============================================================================
//
// These values create HYSTERESIS to prevent relay chattering (rapid on/off)
//
// HOW IT WORKS:
// - When battery drops to or below V_CUTOFF → Load disconnects
// - Load stays OFF even as voltage rises slightly
// - When battery recovers to V_RECONNECT → Load reconnects
// - The gap between these values prevents rapid cycling
//
// EXAMPLE SCENARIOS:
//
// Scenario 1: CONSERVATIVE (Maximum Battery Protection)
//   const float V_CUTOFF = 12.4;     // Disconnect at 12.4V (~20% SOC)
//   const float V_RECONNECT = 13.0;  // Reconnect at 13.0V (~80% SOC)
//   USE WHEN: Battery longevity is priority, or battery is small/old
//
// Scenario 2: BALANCED (Default - Good for Most Users)
//   const float V_CUTOFF = 12.0;     // Disconnect at 12.0V (~5% SOC)
//   const float V_RECONNECT = 12.9;  // Reconnect at 12.9V (~70% SOC)
//   USE WHEN: Normal operation, good balance of protection and runtime
//
// Scenario 3: AGGRESSIVE (Maximum Runtime)
//   const float V_CUTOFF = 11.5;     // Disconnect at 11.5V (battery empty)
//   const float V_RECONNECT = 12.5;  // Reconnect at 12.5V (~40% SOC)
//   USE WHEN: Runtime is critical, battery is new/healthy
//   WARNING: May reduce battery lifespan if used frequently
//
// Scenario 4: TIGHT GAP (Fast Reconnection)
//   const float V_CUTOFF = 12.0;     // Disconnect at 12.0V
//   const float V_RECONNECT = 12.5;  // Reconnect at 12.5V (small gap)
//   USE WHEN: Battery recovers quickly, minimal voltage sag
//   WARNING: May cause relay chatter with high loads or weak batteries
//
// Scenario 5: WIDE GAP (Prevent Cycling)
//   const float V_CUTOFF = 12.0;     // Disconnect at 12.0V
//   const float V_RECONNECT = 13.5;  // Reconnect at 13.5V (large gap)
//   USE WHEN: Load causes significant voltage drop, or slow solar charging
//   BENEFIT: Ensures battery is nearly full before reconnecting
//
// IMPORTANT NOTES:
// - V_RECONNECT must ALWAYS be higher than V_CUTOFF
// - Minimum recommended gap: 0.3V (prevents chatter)
// - Typical gap: 0.5V - 1.0V (good for most applications)
// - Never set V_CUTOFF below 11.0V (damages LiFePO4 cells)
// - Never set V_RECONNECT above 14.0V (only reached during charging)
//
// TO CHANGE THESE VALUES:
// 1. Edit the two lines below
// 2. Upload the modified code to your ESP32
// 3. Check Serial Monitor to confirm new values are active
//
// QUICK REFERENCE:
// See THRESHOLD_QUICK_GUIDE.md for copy-paste examples and visual guide
// See README.md "Customizing Voltage Thresholds" section for detailed explanation
//
// ============================================================================

const float V_CUTOFF = 8.0;      // Disconnect load at or below this voltage
const float V_RECONNECT = 12.0;  // Reconnect load at or above this voltage

// ============================================================================

// ============================================================================
// GLOBAL STATE VARIABLES
// ============================================================================

WebServer server(80);         // Web server on port 80

bool loadEnabled = true;      // Current load state: true = load connected
bool autoMode = true;         // Control mode: true = automatic, false = manual

float lastVBat = 0.0;        // Last measured battery voltage
int lastPct = 0;             // Last calculated battery percentage

// ============================================================================
// RELAY CONTROL FUNCTIONS
// ============================================================================

/**
 * Sets the relay energization state
 * 
 * @param energized true to energize relay coil, false to de-energize
 * 
 * Note: With NO (Normally Open) wiring:
 * - Relay NOT energized → NO open → Load has NO power
 * - Relay IS energized → NO closes → Load connected
 */
void setRelayEnergized(bool energized) {
  if (RELAY_ACTIVE_LOW) {
    // Active LOW relay: LOW = energized, HIGH = not energized
    digitalWrite(RELAY_PIN, energized ? LOW : HIGH);
  } else {
    // Active HIGH relay: HIGH = energized, LOW = not energized
    digitalWrite(RELAY_PIN, energized ? HIGH : LOW);
  }
}

/**
 * Applies the desired load state
 * 
 * @param wantLoadOn true to enable load (relay ON), false to disable (relay OFF)
 * 
 * This is the main function to control load power.
 * NO relay logic: To enable load, relay must be energized.
 */
void applyLoadState(bool wantLoadOn) {
  loadEnabled = wantLoadOn;
  // NO relay logic: To enable load, relay must be energized
  setRelayEnergized(wantLoadOn);
}

// ============================================================================
// BATTERY VOLTAGE MEASUREMENT
// ============================================================================

/**
 * Reads and calculates the actual battery voltage
 * 
 * Process:
 * 1. Takes multiple ADC samples and averages them (reduces noise)
 * 2. Converts ADC value to voltage at the ADC pin
 * 3. Applies voltage divider formula to get actual battery voltage
 * 
 * @return Battery voltage in volts (float)
 * 
 * Formula breakdown:
 * - ADC reading → voltage at pin: Vadc = (ADC / ADC_MAX) × VREF
 * - Pin voltage → battery voltage: Vbat = Vadc × ((RTOP + RBOT) / RBOT)
 */
float readBatteryVoltage() {
  // Take multiple samples for averaging (reduces noise and spikes)
  long sum = 0;
  for (int i = 0; i < SAMPLES; i++) {
    sum += analogRead(ADC_PIN);
    delayMicroseconds(200);  // Small delay between samples
  }
  
  // Calculate average ADC reading
  float avgADC = (float)sum / (float)SAMPLES;
  
  // Convert ADC value to voltage at the pin
  float vAdc = (avgADC / (float)ADC_MAX) * VREF;
  
  // Apply voltage divider formula to get actual battery voltage
  // Vbat = Vadc × (divider ratio)
  float vBat = vAdc * ((RTOP + RBOT) / RBOT);
  
  return vBat;
}

// ============================================================================
// BATTERY PERCENTAGE ESTIMATION
// ============================================================================

/**
 * Estimates battery state of charge from voltage
 * 
 * @param v Battery voltage in volts
 * @return Estimated percentage (0-100)
 * 
 * Generic battery voltage curve:
 * - 8.0V or below → 0% (cutoff/empty)
 * - 8.0V - 10.0V → 0-20% (low battery)
 * - 10.0V - 12.0V → 20-60% (mid range)
 * - 12.0V - 13.0V → 60-85% (good charge)
 * - 13.0V - 14.0V → 85-100% (full)
 * - 14.0V or above → 100% (charging/full)
 * 
 * Note: Adjust these ranges based on your specific battery type
 */
int lifepo4Percent(float v) {
  if (v <= 8.0) return 0;
  if (v >= 14.0) return 100;
  
  // 0-20%: 8.0V to 10.0V
  if (v < 10.0) return (int)((v - 8.0) / 2.0 * 20.0);
  
  // 20-60%: 10.0V to 12.0V
  if (v < 12.0) return 20 + (int)((v - 10.0) / 2.0 * 40.0);
  
  // 60-85%: 12.0V to 13.0V
  if (v < 13.0) return 60 + (int)((v - 12.0) / 1.0 * 25.0);
  
  // 85-100%: 13.0V to 14.0V
  return 85 + (int)((v - 13.0) / 1.0 * 15.0);
}

// ============================================================================
// WEB SERVER - HTML PAGE
// ============================================================================

/**
 * Generates the HTML page for the web interface
 * 
 * Features:
 * - Responsive design (works on mobile)
 * - Live updating display (fetches /status.json every second)
 * - Manual control buttons (Auto, Force On, Force Off)
 * - Shows voltage, percentage, load state, and control mode
 * 
 * @return HTML page as String
 */
String htmlPage() {
  String ip = WiFi.isConnected() ? WiFi.localIP().toString() : String("not connected");
  
  String s;
  
  // HTML head with responsive viewport and styling
  s += "<!doctype html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  s += "<title>Battery Monitor</title>";
  
  // Embedded CSS for clean, card-based design
  s += "<style>";
  s += "body{font-family:Arial,sans-serif;padding:16px;background:#f5f5f5;margin:0}";
  s += ".card{border:1px solid #ddd;border-radius:12px;padding:20px;max-width:460px;background:white;margin:0 auto;box-shadow:0 2px 4px rgba(0,0,0,0.1)}";
  s += ".big{font-size:36px;font-weight:700;color:#2c3e50;margin:10px 0}";
  s += ".row{margin:12px 0;font-size:16px}";
  s += "button{padding:12px 16px;margin:4px;border-radius:8px;border:1px solid #3498db;background:#3498db;color:white;cursor:pointer;font-size:14px;font-weight:600}";
  s += "button:hover{background:#2980b9;border-color:#2980b9}";
  s += "button:active{transform:scale(0.98)}";
  s += ".status{display:inline-block;padding:4px 12px;border-radius:6px;font-weight:600}";
  s += ".status-on{background:#27ae60;color:white}";
  s += ".status-off{background:#e74c3c;color:white}";
  s += ".status-auto{background:#f39c12;color:white}";
  s += ".status-manual{background:#95a5a6;color:white}";
  s += "small{color:#7f8c8d}";
  s += "</style>";
  s += "</head><body>";
  
  // Main content card
  s += "<div class='card'>";
  s += "<h2 style='margin-top:0;color:#2c3e50'>Battery Monitor</h2>";
  
  // ESP32 IP address
  s += "<div class='row'>ESP32 IP: <b>" + ip + "</b></div>";
  
  // Large voltage display
  s += "<div class='row big' id='v'>--.-- V</div>";
  
  // Battery percentage
  s += "<div class='row'>Battery: <b id='p'>--</b>%</div>";
  
  // Load status with colored badge
  s += "<div class='row'>Load: <span class='status' id='on'>--</span></div>";
  
  // Control mode with colored badge
  s += "<div class='row'>Mode: <span class='status' id='mode'>--</span></div>";
  
  // Control buttons
  s += "<div class='row' style='margin-top:20px'>";
  s += "<button onclick=\"fetch('/relay?auto=1').then(()=>tick())\">Auto Mode</button> ";
  s += "<button onclick=\"fetch('/relay?on=1').then(()=>tick())\">Force ON</button> ";
  s += "<button onclick=\"fetch('/relay?on=0').then(()=>tick())\">Force OFF</button>";
  s += "</div>";
  
  // Footer info
  s += "<div class='row' style='margin-top:20px'><small>Updates every 1 second from <code>/status.json</code></small></div>";
  s += "</div>";
  
  // JavaScript for live updates
  s += "<script>";
  s += "async function tick(){";
  s += "  try{";
  s += "    const r = await fetch('/status.json',{cache:'no-store'});";
  s += "    const j = await r.json();";
  
  // Update voltage display
  s += "    document.getElementById('v').textContent = j.voltage_v.toFixed(2) + ' V';";
  
  // Update percentage display
  s += "    document.getElementById('p').textContent = j.percent;";
  
  // Update load status with colored badge
  s += "    const loadElem = document.getElementById('on');";
  s += "    loadElem.textContent = j.load_on ? 'ON' : 'OFF';";
  s += "    loadElem.className = 'status ' + (j.load_on ? 'status-on' : 'status-off');";
  
  // Update mode with colored badge
  s += "    const modeElem = document.getElementById('mode');";
  s += "    modeElem.textContent = j.auto_mode ? 'AUTO' : 'MANUAL';";
  s += "    modeElem.className = 'status ' + (j.auto_mode ? 'status-auto' : 'status-manual');";
  
  s += "  }catch(e){console.error('Fetch error:',e);}";
  s += "}";
  
  // Update every second and run immediately
  s += "setInterval(tick,1000); tick();";
  s += "</script>";
  
  s += "</body></html>";
  return s;
}

// ============================================================================
// WEB SERVER - REQUEST HANDLERS
// ============================================================================

/**
 * Handler for root path "/"
 * Serves the HTML web interface
 */
void handleRoot() {
  server.send(200, "text/html", htmlPage());
}

/**
 * Handler for "/status.json"
 * Returns current system status as JSON
 * 
 * JSON format:
 * {
 *   "voltage_v": 12.345,
 *   "percent": 65,
 *   "load_on": true,
 *   "auto_mode": true,
 *   "uptime_ms": 123456
 * }
 */
void handleStatus() {
  String json = "{";
  json += "\"voltage_v\":" + String(lastVBat, 3) + ",";
  json += "\"percent\":" + String(lastPct) + ",";
  json += "\"load_on\":" + String(loadEnabled ? "true" : "false") + ",";
  json += "\"auto_mode\":" + String(autoMode ? "true" : "false") + ",";
  json += "\"uptime_ms\":" + String(millis());
  json += "}";
  server.send(200, "application/json", json);
}

/**
 * Handler for "/relay"
 * Allows manual control of the relay via query parameters
 * 
 * Query parameters:
 * - ?auto=1 → Enable automatic mode (hysteresis control)
 * - ?on=1   → Force load ON (disables auto mode)
 * - ?on=0   → Force load OFF (disables auto mode)
 * 
 * Examples:
 * - http://ESP32_IP/relay?auto=1
 * - http://ESP32_IP/relay?on=1
 * - http://ESP32_IP/relay?on=0
 */
void handleRelay() {
  // Check for auto mode request
  if (server.hasArg("auto") && server.arg("auto") == "1") {
    autoMode = true;
    // Don't change load state, let automatic control handle it
  }
  
  // Check for manual on/off request
  if (server.hasArg("on")) {
    autoMode = false;  // Disable automatic control
    bool turnOn = (server.arg("on") == "1");
    applyLoadState(turnOn);
  }
  
  server.send(200, "text/plain", "OK");
}

// ============================================================================
// SETUP FUNCTION - RUNS ONCE AT STARTUP
// ============================================================================

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(115200);
  delay(300);  // Allow serial to stabilize
  
  Serial.println();
  Serial.println("========================================");
  Serial.println("  Battery Cutoff Monitor Starting");
  Serial.println("========================================");
  
  // Configure relay pin as output
  pinMode(RELAY_PIN, OUTPUT);
  
  // Configure ADC settings
  analogReadResolution(12);                     // 12-bit resolution (0-4095)
  analogSetPinAttenuation(ADC_PIN, ADC_11db);  // 11dB attenuation (0-3.3V range)
  
  Serial.println("ADC configured: 12-bit, 11dB attenuation");
  
  // Initialize system in automatic mode with load enabled
  autoMode = true;
  applyLoadState(true);  // Start with load connected
  
  Serial.println("Initial state: Load ON, Auto mode");
  Serial.print("Voltage divider: ");
  Serial.print(RTOP);
  Serial.print("Ω / ");
  Serial.print(RBOT);
  Serial.print("Ω (ratio: ");
  Serial.print((RTOP + RBOT) / RBOT, 2);
  Serial.println(")");
  
  Serial.print("Cutoff: ");
  Serial.print(V_CUTOFF);
  Serial.print("V, Reconnect: ");
  Serial.print(V_RECONNECT);
  Serial.println("V");
  
  // Connect to WiFi
  Serial.println();
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);
  
  WiFi.mode(WIFI_STA);  // Station mode (client)
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  // Wait up to 15 seconds for connection
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 15000) {
    delay(250);
    Serial.print(".");
  }
  
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Open in browser: http://");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi connection failed (timeout)");
    Serial.println("System will continue without WiFi");
  }
  
  // Configure web server routes
  server.on("/", handleRoot);              // Main page
  server.on("/status.json", handleStatus); // JSON API
  server.on("/relay", handleRelay);        // Manual control
  
  // Start web server
  server.begin();
  Serial.println("Web server started");
  Serial.println("========================================");
  Serial.println();
  Serial.println("CSV Output: Voltage,Percent,LoadState");
  Serial.println();
}

// ============================================================================
// MAIN LOOP - RUNS CONTINUOUSLY
// ============================================================================

void loop() {
  // Handle incoming web requests
  server.handleClient();
  
  // Perform periodic voltage measurement and control logic
  static unsigned long lastReadTime = 0;
  
  // Update every 250ms (4 times per second)
  if (millis() - lastReadTime >= 250) {
    lastReadTime = millis();
    
    // Read battery voltage and calculate percentage
    lastVBat = readBatteryVoltage();
    lastPct = lifepo4Percent(lastVBat);
    
    // Automatic hysteresis control (only if in auto mode)
    if (autoMode) {
      // Check if load should be disconnected (battery too low)
      if (loadEnabled && lastVBat <= V_CUTOFF) {
        Serial.println("! CUTOFF: Battery voltage low, disconnecting load");
        applyLoadState(false);
      }
      // Check if load should be reconnected (battery recovered)
      else if (!loadEnabled && lastVBat >= V_RECONNECT) {
        Serial.println("! RECONNECT: Battery voltage recovered, reconnecting load");
        applyLoadState(true);
      }
    }
    
    // Output CSV format to serial: voltage, percent, load_state
    // This format is easy to parse and log externally
    Serial.print(lastVBat, 2);
    Serial.print(",");
    Serial.print(lastPct);
    Serial.print(",");
    Serial.println(loadEnabled ? 1 : 0);
  }
}

// ============================================================================
// END OF CODE
// ============================================================================
