# Battery Cutoff Monitor (ESP32 + NC Relay + 12.8V LiFePO4)

A smart battery protection system that automatically disconnects loads at low voltage and reconnects them after battery recovery. Features a web interface for real-time monitoring and manual control.

> **⚡ Quick Start: Want to change when the load disconnects/reconnects?**  
> Jump to: [Customizing Voltage Thresholds](#customizing-voltage-thresholds)

## 📋 Table of Contents

- [What It Does](#what-it-does)
- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Wiring Guide](#wiring-guide)
- [Voltage Divider Guide](#voltage-divider-guide)
- [Software Setup](#software-setup)
- [Customizing Voltage Thresholds](#customizing-voltage-thresholds)
- [Web Interface](#web-interface)
- [Troubleshooting](#troubleshooting)
- [Technical Details](#technical-details)

## 🎯 What It Does

This system protects a **12.8V LiFePO4 battery** (4S configuration) by:

1. **Continuously monitoring** battery voltage through an ESP32 ADC pin
2. **Automatically disconnecting** the sensor load when voltage drops to 12.0V or below
3. **Reconnecting** the load only after voltage recovers to 12.9V or higher
4. **Preventing relay chatter** through hysteresis (different ON/OFF thresholds)
5. **Providing real-time monitoring** via web interface and JSON API

## ✨ Features

- **🔋 Battery Protection:** Prevents deep discharge damage to LiFePO4 cells
- **🌐 Web Interface:** Beautiful, responsive dashboard accessible from any device
- **📊 Live Monitoring:** Real-time voltage, percentage, and load status
- **🎛️ Manual Override:** Force load ON/OFF or enable automatic control
- **📡 JSON API:** Integrate with external monitoring systems
- **🔌 Relay Control:** Smart NC (Normally Closed) relay management
- **📈 Serial Output:** CSV format for logging and debugging
- **⚙️ Configurable:** Easy-to-adjust thresholds and parameters

## 🛠️ Hardware Requirements

### Main Components

| Component | Specification | Purpose |
|-----------|--------------|---------|
| **ESP32 Board** | Any development board | Main controller |
| **LiFePO4 Battery** | 12.8V (4S) | Power source |
| **Solar Panel** | 18V nominal (optional) | Battery charging |
| **Charge Controller** | For LiFePO4 | Battery management |
| **DC-DC Converter** | 12V → 5V, 2A+ | Power ESP32 and load |
| **Relay Module** | 5V, NC contact, active-LOW | Load disconnect |
| **Resistor (R1)** | 10kΩ, 1/4W | Voltage divider top |
| **Resistor (R2)** | 1kΩ, 1/4W | Voltage divider bottom |
| **Capacitor** | 0.1µF to 1µF | ADC noise filtering |
| **Jumper Wires** | Various | Connections |

### Recommended Relay Specifications

- **Type:** Electromechanical or Solid State Relay (SSR)
- **Coil Voltage:** 5V DC
- **Contact Type:** NC (Normally Closed)
- **Current Rating:** Match or exceed your load current
- **Control:** Active-LOW (most common)

## 🔌 Wiring Guide

### Critical: Common Ground

**⚠️ ALL grounds must be connected together!** This includes:

- Battery negative (-)
- DC-DC input ground
- DC-DC output ground
- ESP32 GND
- Relay module GND
- Sensor/load GND

Failure to connect all grounds will result in incorrect measurements and relay control issues.

### 1. Battery Measurement Circuit (Voltage Divider)

The ESP32 ADC can only handle **0-3.3V**. Since the battery voltage is **10-15V**, we need a voltage divider to scale it down safely.

```
Battery+ (Vin) ──┬── 10kΩ (RTOP) ──┬── ADC_NODE ──┬── 1kΩ (RBOT) ──┬── GND
                 │                  │              │                │
                 │                  ├──────────────┤                │
                 │                  │  Capacitor   │                │
                 │                  │  0.1µF-1µF   │                │
                 │                  └──────────────┘                │
                 │                       ↓                          │
                 │                   GPIO36 (VP)                    │
```

**Connections:**
1. Battery positive (+) → 10kΩ resistor (one end)
2. 10kΩ resistor (other end) → Junction point (ADC_NODE)
3. Junction point → 1kΩ resistor (one end)
4. 1kΩ resistor (other end) → GND
5. Junction point → ESP32 GPIO36 (VP pin)
6. Capacitor (+) → Junction point
7. Capacitor (-) → GND

**Why these resistor values?**
- Creates a divider ratio of 11:1
- At 14.6V (max charge): ADC sees 1.33V ✅ (safe)
- At 12.8V (nominal): ADC sees 1.16V ✅
- At 12.0V (cutoff): ADC sees 1.09V ✅
- Maximum safe input: ~36V

### 2. Relay Wiring (NC Cutoff Configuration)

The relay uses the **NC (Normally Closed)** contact to cut power to the load:

```
                    Relay Module
                  ┌──────────────┐
DC-DC Output+ ────┤ COM          │
                  │              │
                  │     NC       ├──── Load+ (Sensor+)
                  │              │
                  │     NO       │ (not used)
                  └──────────────┘
                        ↓
                   Relay Coil
                  ┌──────────────┐
ESP32 GPIO2  ─────┤ IN           │
                  │              │
ESP32 GND    ─────┤ GND          │
                  │              │
DC-DC Output+ ────┤ VCC (5V)     │
                  └──────────────┘

Load GND ───────────────────────────── Common GND
```

**How it works:**
- **Relay OFF (not energized):** NC closed → Load receives power ✅
- **Relay ON (energized):** NC opens → Load disconnected 🚫

**Important:** The ESP32 must be powered **before** the relay cutoff point, so it stays online to monitor the battery and control reconnection.

### 3. ESP32 Power Connection

Power the ESP32 from the DC-DC output **before** the relay:

```
DC-DC Output+ ──┬──→ ESP32 VIN (or 5V pin)
                │
                └──→ Relay VCC
                └──→ Relay COM

DC-DC Output- ──→ ESP32 GND (common ground)
```

### 4. Complete System Diagram

```
                    ┌─────────────────────────────────────┐
                    │   12.8V LiFePO4 Battery             │
                    │   (with Solar Panel & Controller)   │
                    └──┬────────────────────────────┬─────┘
                       │+                           │-
                       │                            │ (Common GND)
                       │                            │
         ┌─────────────┤                            ├──────────────┐
         │             │                            │              │
         │    ┌────────▼────────┐                   │              │
         │    │  DC-DC Converter│                   │              │
         │    │   12V → 5V      │                   │              │
         │    └────┬───────┬────┘                   │              │
         │         │+      │-                       │              │
         │         │       └────────────────────────┼──────────┐   │
         │         │                                │          │   │
    ┌────┴─────┐   │    ┌─────────────┐            │          │   │
    │ Voltage  │   │    │   ESP32     │            │          │   │
    │ Divider  │   ├────┤ VIN     GND ├────────────┤          │   │
    │          │   │    │             │            │          │   │
    │ 10kΩ─1kΩ│   │    │ GPIO36  GPIO2│            │          │   │
    └────┬─────┘   │    └───┬──────┬──┘            │          │   │
         │         │        │      │               │          │   │
         │         │        │  ┌───▼─────────┐     │          │   │
         └─────────┼────────┘  │ Relay Module│     │          │   │
                   │           │  IN     VCC ├─────┘          │   │
                   │           │ GND     COM ├────────┐       │   │
                   │           │          NC ├────────┼───────┘   │
                   │           │          NO │ (NC)   │           │
                   │           └─────────────┘        │           │
                   │                                  │           │
                   │                         ┌────────▼────┐      │
                   │                         │   LOAD      │      │
                   │                         │  (Sensor)   │      │
                   │                         └────────┬────┘      │
                   │                                  │           │
                   └──────────────────────────────────┴───────────┘
                                    Common GND
```

## 📐 Voltage Divider Guide

### Why We Need a Voltage Divider

The ESP32 ADC can only safely measure **0-3.3V**. LiFePO4 batteries range from **10-15V**, so we must reduce the voltage before measuring.

### How to Choose Resistor Values

**Rule 1: Keep ADC voltage under 3.3V**

The voltage divider formula is:

```
Vadc = Vbat × (RBOT / (RTOP + RBOT))
```

For safety, ensure:

```
Vadc_max ≤ 3.3V when Vbat = 14.6V (max charge voltage)
```

This means:

```
RBOT / (RTOP + RBOT) ≤ 3.3 / 14.6 = 0.226
```

So the divider should scale the battery voltage to **22.6% or less**.

**Rule 2: Lower resistance = less noise**

High-resistance dividers are more susceptible to noise and ADC input impedance effects. That's why **10kΩ/1kΩ is better than 100kΩ/10kΩ** (same ratio, lower noise).

**Rule 3: Don't waste battery power**

The divider continuously draws current:

```
I = Vbat / (RTOP + RBOT)
```

**Examples:**

| RTOP | RBOT | Ratio | Current @13V | Power | Max Input | Noise |
|------|------|-------|--------------|-------|-----------|-------|
| 10kΩ | 1kΩ | 11 | 1.18 mA | 15 mW | 36.3V | ⭐⭐⭐⭐⭐ Low |
| 100kΩ | 10kΩ | 11 | 0.12 mA | 1.5 mW | 36.3V | ⭐⭐⭐ Medium |
| 20kΩ | 5kΩ | 5 | 0.52 mA | 6.7 mW | 16.5V | ⭐⭐⭐⭐ Low |
| 40kΩ | 10kΩ | 5 | 0.26 mA | 3.4 mW | 16.5V | ⭐⭐⭐⭐ Low |

**Recommended for 12V LiFePO4:**
- **Best:** RTOP=10kΩ, RBOT=1kΩ (default in code)
- **Good:** RTOP=20kΩ, RBOT=5kΩ (if you have these values)
- **Okay:** RTOP=100kΩ, RBOT=10kΩ (noisier, use 1µF cap)

### Capacitor Placement

The capacitor smooths out voltage spikes and ADC noise:

- **Location:** Connect between ADC_NODE and GND
- **Value:** Start with 0.1µF (ceramic)
- **If readings still jump:** Increase to 0.47µF or 1µF
- **Type:** Ceramic or film capacitor (low ESR)

## 💻 Software Setup

### 1. Install Arduino IDE

Download and install from [arduino.cc](https://www.arduino.cc/en/software)

### 2. Install ESP32 Board Support

1. Open Arduino IDE
2. Go to **File → Preferences**
3. In "Additional Board Manager URLs", add:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Go to **Tools → Board → Boards Manager**
5. Search for "ESP32"
6. Install **"esp32 by Espressif Systems"**

### 3. Configure the Code

Open `voltage_meter.ino` and modify these settings:

#### WiFi Credentials (Required)

```cpp
const char* WIFI_SSID = "YOUR_WIFI_SSID";       // Change this
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";   // Change this
```

#### Resistor Values (If Different from Default)

```cpp
const float RTOP = 10000.0;   // Top resistor in Ohms
const float RBOT = 1000.0;    // Bottom resistor in Ohms
```

#### Voltage Thresholds (IMPORTANT - Customize for Your Needs)

The voltage thresholds determine when the load is disconnected and reconnected. These are **critical settings** that affect both battery protection and system runtime.

**Location in code:** Look for the section labeled:
```cpp
// BATTERY PROTECTION THRESHOLDS - CUSTOMIZE THESE FOR YOUR NEEDS
```

**The two values to change:**

```cpp
const float V_CUTOFF = 12.0;      // Disconnect load at or below this voltage
const float V_RECONNECT = 12.9;   // Reconnect load at or above this voltage
```

**How Hysteresis Works:**

The system uses two different thresholds (not just one) to prevent rapid on/off cycling:

1. **Load is ON** → Battery drops to **12.0V** → Load **disconnects**
2. Load stays OFF as battery slowly rises from 12.0V → 12.1V → 12.2V...
3. Battery reaches **12.9V** → Load **reconnects**
4. Cycle repeats

The gap between these values (0.9V in default) is called **hysteresis** and prevents relay chatter.

**Pre-configured Scenarios:**

| Scenario | Cutoff | Reconnect | Gap | Use Case |
|----------|--------|-----------|-----|----------|
| **Conservative** | 12.4V | 13.0V | 0.6V | Maximum battery protection, long lifespan |
| **Balanced** ⭐ | 12.0V | 12.9V | 0.9V | Default - good for most users |
| **Aggressive** | 11.5V | 12.5V | 1.0V | Maximum runtime, newer batteries |
| **Tight Gap** | 12.0V | 12.5V | 0.5V | Fast reconnection, stable batteries |
| **Wide Gap** | 12.0V | 13.5V | 1.5V | Heavy loads, prevent cycling |

**Choosing Your Values:**

**1. Choose your CUTOFF voltage (when to disconnect):**

| Cutoff | Battery % | Description | When to Use |
|--------|-----------|-------------|-------------|
| 12.8V | ~50% | Very conservative | Old/weak batteries |
| 12.4V | ~20% | Conservative | Maximize battery lifespan |
| **12.0V** | ~5% | **Balanced** ⭐ | **Recommended for most** |
| 11.5V | ~1% | Aggressive | Emergency/critical applications |
| 11.0V | 0% | Danger zone ⚠️ | Not recommended (damages cells) |

**2. Choose your RECONNECT voltage (when to reconnect):**

| Reconnect | Battery % | Description |
|-----------|-----------|-------------|
| 13.5V | ~95% | Wait until nearly full (slow reconnect) |
| 13.0V | ~80% | Conservative (good for small batteries) |
| **12.9V** | ~70% | **Balanced** ⭐ **Recommended** |
| 12.5V | ~40% | Aggressive (faster reconnect) |
| 12.2V | ~15% | Very aggressive (may cycle frequently) |

**3. Check the gap:**

- **Minimum:** 0.3V (risk of chatter)
- **Recommended:** 0.5V - 1.0V (good balance)
- **Large loads:** 1.0V - 1.5V (prevents cycling under load)

**Examples for Different Applications:**

**Example 1: Security Camera (24/7 operation needed)**
```cpp
const float V_CUTOFF = 11.5;      // Run as long as possible
const float V_RECONNECT = 12.5;   // Reconnect quickly
```

**Example 2: Weekend Cabin (battery longevity priority)**
```cpp
const float V_CUTOFF = 12.4;      // Protect battery health
const float V_RECONNECT = 13.2;   // Only reconnect when well charged
```

**Example 3: Weather Station (low power, occasional use)**
```cpp
const float V_CUTOFF = 12.0;      // Standard protection
const float V_RECONNECT = 12.6;   // Quick recovery for low-power devices
```

**Example 4: High-Power Load (motors, pumps)**
```cpp
const float V_CUTOFF = 12.2;      // Disconnect before heavy voltage sag
const float V_RECONNECT = 13.5;   // Wait for full charge before restarting
```

**⚠️ Important Rules:**

- ✅ V_RECONNECT **must be higher** than V_CUTOFF
- ✅ Gap should be **at least 0.3V** (0.5V+ recommended)
- ⚠️ Never set V_CUTOFF below **11.0V** (damages LiFePO4)
- ⚠️ Never set V_RECONNECT above **14.0V** (only during charging)
- 💡 Test your settings by monitoring for a few days
- 💡 If relay cycles frequently, **increase the gap**

### 4. Upload to ESP32

1. Connect ESP32 to computer via USB
2. Select board: **Tools → Board → ESP32 Dev Module** (or your specific board)
3. Select port: **Tools → Port → [Your COM port]**
4. Click **Upload** button (→)
5. Wait for "Done uploading" message

### 5. Monitor Serial Output

1. Open **Tools → Serial Monitor**
2. Set baud rate to **115200**
3. You should see:
   ```
   ========================================
     Battery Cutoff Monitor Starting
   ========================================
   ADC configured: 12-bit, 11dB attenuation
   Initial state: Load ON, Auto mode
   Voltage divider: 10000.00Ω / 1000.00Ω (ratio: 11.00)
   Cutoff: 12.00V, Reconnect: 12.90V
   
   Connecting to WiFi: YourNetwork
   ...
   WiFi connected!
   IP address: 192.168.1.100
   Open in browser: http://192.168.1.100
   ========================================
   
   CSV Output: Voltage,Percent,LoadState
   
   12.85,65,1
   12.84,64,1
   ```

## ⚙️ Customizing Voltage Thresholds

> 📄 **TL;DR?** See [THRESHOLD_QUICK_GUIDE.md](THRESHOLD_QUICK_GUIDE.md) for a one-page reference with copy-paste examples!

### Quick Reference: Where to Change Thresholds

**File:** `voltage_meter.ino`  
**Lines to modify:** Around lines 90-140 (look for the large comment block)

```cpp
const float V_CUTOFF = 12.0;      // Change this value
const float V_RECONNECT = 12.9;   // Change this value
```

### Understanding the Behavior

The system operates in a simple cycle:

```
Battery voltage is HIGH (e.g., 13.5V)
           ↓
    [LOAD IS ON]
           ↓
Battery drains during use
           ↓
Voltage drops to V_CUTOFF (12.0V)
           ↓
    [LOAD TURNS OFF] ← Relay disconnects
           ↓
Battery recovers (solar charging or rest)
           ↓
Voltage rises: 12.1V → 12.2V → ... → 12.8V
           ↓
     [STILL OFF] ← Waiting for V_RECONNECT
           ↓
Voltage reaches V_RECONNECT (12.9V)
           ↓
    [LOAD TURNS ON] ← Relay reconnects
           ↓
Cycle repeats
```

### Why Two Different Voltages?

**Without hysteresis (using only one threshold):**
- Battery at 12.0V → Load OFF
- Load turns off → Voltage instantly rises to 12.1V (no load)
- Voltage above 12.0V → Load ON again
- Load turns on → Voltage drops to 12.0V (under load)
- **Result: Relay chatters on/off rapidly!** 🔄⚡

**With hysteresis (two thresholds):**
- Battery at 12.0V → Load OFF
- Voltage must rise significantly (to 12.9V) before reconnecting
- Battery has time to properly recharge
- **Result: Smooth, stable operation** ✅

### Step-by-Step: Changing Thresholds

**Step 1:** Open `voltage_meter.ino` in Arduino IDE

**Step 2:** Find the configuration section (around line 90-140):
```cpp
// BATTERY PROTECTION THRESHOLDS - CUSTOMIZE THESE FOR YOUR NEEDS
```

**Step 3:** Edit the two lines:
```cpp
const float V_CUTOFF = 12.0;      // Your desired cutoff voltage
const float V_RECONNECT = 12.9;   // Your desired reconnect voltage
```

**Step 4:** Save the file (Ctrl+S or Cmd+S)

**Step 5:** Upload to ESP32:
- Connect ESP32 via USB
- Click Upload button (→)
- Wait for "Done uploading"

**Step 6:** Verify in Serial Monitor:
```
Cutoff: 12.00V, Reconnect: 12.90V  ← Your new values should appear here
```

**Step 7:** Test the behavior:
- Monitor the system for a few charge/discharge cycles
- Check if the load disconnects/reconnects at expected voltages
- Adjust if needed and re-upload

### Common Adjustment Scenarios

**Problem: Load cycles on/off too frequently**

**Cause:** Gap between thresholds is too small, or battery voltage sags heavily under load

**Solution:** Increase the gap
```cpp
// BEFORE
const float V_CUTOFF = 12.0;
const float V_RECONNECT = 12.5;   // Gap: 0.5V (too small)

// AFTER
const float V_CUTOFF = 12.0;
const float V_RECONNECT = 13.0;   // Gap: 1.0V (better)
```

---

**Problem: Load disconnects too early (battery still has capacity)**

**Cause:** V_CUTOFF is set too high

**Solution:** Lower the cutoff voltage
```cpp
// BEFORE
const float V_CUTOFF = 12.5;      // Too conservative

// AFTER
const float V_CUTOFF = 12.0;      // More runtime
```

---

**Problem: Load doesn't reconnect for a long time**

**Cause:** V_RECONNECT is set too high

**Solution:** Lower the reconnect voltage
```cpp
// BEFORE
const float V_RECONNECT = 13.5;   // Waits for nearly full charge

// AFTER
const float V_RECONNECT = 12.8;   // Reconnects sooner
```

---

**Problem: Battery gets damaged / reduced lifespan**

**Cause:** V_CUTOFF is too low (deep discharge)

**Solution:** Raise the cutoff voltage
```cpp
// BEFORE
const float V_CUTOFF = 11.0;      // Discharges too deeply!

// AFTER
const float V_CUTOFF = 12.2;      // Better protection
```

### Testing Your Settings

After changing thresholds, monitor the system:

1. **Watch Serial Monitor output:**
   ```
   12.45,25,1    ← Voltage, Percent, Load(1=ON)
   12.32,18,1
   12.15,8,1
   12.00,5,0     ← Load turned OFF at cutoff
   12.12,7,0     ← Still OFF
   12.56,42,0    ← Still OFF (below reconnect)
   12.90,70,1    ← Load turned ON at reconnect
   ```

2. **Check web interface:** Monitor live voltage and load status

3. **Measure actual battery voltage** with multimeter to verify accuracy

4. **Run for 2-3 full cycles** (discharge → recharge → discharge)

5. **Adjust if needed:**
   - Too much cycling? → Increase gap
   - Disconnects too late? → Raise V_CUTOFF
   - Reconnects too late? → Lower V_RECONNECT

### Advanced: Seasonal Adjustments

You may want different thresholds for different seasons:

**Winter (less solar, conservative):**
```cpp
const float V_CUTOFF = 12.4;      // Protect battery in cold
const float V_RECONNECT = 13.2;   // Wait for good charge
```

**Summer (abundant solar, aggressive):**
```cpp
const float V_CUTOFF = 11.8;      // More runtime
const float V_RECONNECT = 12.6;   // Reconnect faster
```

Simply re-upload the modified code when seasons change!

## 🌐 Web Interface

### Accessing the Dashboard

Once connected to WiFi, open your browser and navigate to the ESP32's IP address (shown in Serial Monitor):

```
http://192.168.1.100/
```

### Dashboard Features

The web interface shows:

- **ESP32 IP Address:** For easy reference
- **Voltage Display:** Large, easy-to-read voltage (updates every second)
- **Battery Percentage:** Estimated state of charge
- **Load Status:** ON (green) or OFF (red) badge
- **Control Mode:** AUTO (orange) or MANUAL (gray) badge

### Control Buttons

- **Auto Mode:** Enable automatic hysteresis control (recommended for normal operation)
- **Force ON:** Manually turn load on (disables automatic mode)
- **Force OFF:** Manually turn load off (disables automatic mode)

### JSON API Endpoint

For integration with external systems, use:

```
http://192.168.1.100/status.json
```

**Response format:**
```json
{
  "voltage_v": 12.845,
  "percent": 65,
  "load_on": true,
  "auto_mode": true,
  "uptime_ms": 123456
}
```

**Example usage (curl):**
```bash
curl http://192.168.1.100/status.json
```

**Example usage (Python):**
```python
import requests

response = requests.get('http://192.168.1.100/status.json')
data = response.json()
print(f"Battery: {data['voltage_v']}V ({data['percent']}%)")
print(f"Load: {'ON' if data['load_on'] else 'OFF'}")
```

## 🔧 Troubleshooting

### No WiFi Connection

**Problem:** ESP32 doesn't connect to WiFi (Serial shows "WiFi connection failed")

**Solutions:**
- Verify SSID and password are correct
- Check that WiFi is 2.4GHz (ESP32 doesn't support 5GHz)
- Move ESP32 closer to router
- Check router doesn't have MAC filtering enabled
- Try restarting router

**Note:** System continues to work without WiFi, but web interface won't be accessible.

### Incorrect Voltage Readings

**Problem:** Displayed voltage doesn't match multimeter reading

**Solutions:**

1. **Verify resistor values** with a multimeter (actual values may differ from labeled)
2. **Check connections** - ensure voltage divider is connected correctly
3. **Calibrate VREF** - ESP32's actual reference voltage may not be exactly 3.3V:
   ```cpp
   const float VREF = 3.28;  // Adjust based on actual measurement
   ```
4. **Measure ADC node voltage** directly with multimeter, compare to expected:
   ```
   Expected Vadc = Vbat × (RBOT / (RTOP + RBOT))
   ```

### Erratic/Jumping Readings

**Problem:** Voltage jumps around rapidly

**Solutions:**
- Add or increase capacitor value (try 1µF instead of 0.1µF)
- Shorten wires between battery and voltage divider
- Keep wires away from noisy sources (relay, DC-DC converter)
- Increase SAMPLES value in code:
  ```cpp
  const int SAMPLES = 64;  // Double the averaging
  ```
- Use twisted pair wire for voltage sensing

### Relay Doesn't Switch

**Problem:** Relay doesn't respond to commands

**Solutions:**
- Check relay module power (VCC should be 5V)
- Verify GPIO2 connection to relay IN pin
- Check if relay is active-LOW or active-HIGH:
  ```cpp
  const bool RELAY_ACTIVE_LOW = false;  // Try changing this
  ```
- Test relay manually: In `setup()` add:
  ```cpp
  digitalWrite(RELAY_PIN, LOW); delay(1000);
  digitalWrite(RELAY_PIN, HIGH); delay(1000);
  ```
- Check relay current rating matches your load

### Load Doesn't Receive Power

**Problem:** Load never gets power even when "ON"

**Solutions:**
- Verify you're using the **NC** (Normally Closed) terminal, not NO
- Check DC-DC converter output voltage (should be ~5V)
- Verify relay COM is connected to DC-DC output +
- Verify relay NC is connected to load +
- Ensure all grounds are connected (common ground)
- Test load directly from DC-DC output to verify it works

### Relay Chatters (Rapid ON/OFF)

**Problem:** Relay rapidly switches on and off

**Solutions:**
- Battery voltage is right at threshold during load changes (voltage sag)
- Increase hysteresis gap:
  ```cpp
  const float V_CUTOFF = 12.0;
  const float V_RECONNECT = 13.2;  // Bigger gap
  ```
- Battery may be too small for the load (high internal resistance)
- Add more capacitance to battery connection

### High Battery Drain

**Problem:** Battery drains faster than expected

**Solutions:**
- Check voltage divider current: I = Vbat / (RTOP + RBOT)
- Use higher resistance values (e.g., 100kΩ / 10kΩ)
- Check DC-DC converter efficiency (should be >85%)
- Measure ESP32 actual current draw (should be 80-150mA with WiFi)
- Check for load leakage current when "OFF"

### Serial Monitor Shows Gibberish

**Problem:** Unreadable characters in Serial Monitor

**Solutions:**
- Set baud rate to **115200** (match code)
- Try pressing ESP32 reset button
- Reconnect USB cable
- Try different USB cable (data cable, not charging-only)

## 📊 Technical Details

### System Specifications

| Parameter | Value |
|-----------|-------|
| **Input Voltage Range** | 10V - 15V (12V nominal) |
| **Max Measurable Voltage** | ~36V (with 10kΩ/1kΩ divider) |
| **ADC Resolution** | 12-bit (4096 steps) |
| **ADC Pin** | GPIO36 (VP) - ADC1_CH0 |
| **Relay Control Pin** | GPIO2 |
| **Measurement Rate** | 4 Hz (every 250ms) |
| **Web Update Rate** | 1 Hz (every 1 second) |
| **Samples per Reading** | 32 (averaged) |
| **WiFi** | 2.4GHz, Station mode |
| **Web Server Port** | 80 (HTTP) |

### Voltage Measurement Accuracy

**Expected accuracy:** ±2-3% with quality components

**Factors affecting accuracy:**
- Resistor tolerance (use 1% resistors for best accuracy)
- ESP32 VREF variation (typically 3.2-3.4V, not exactly 3.3V)
- ADC non-linearity (especially at extremes)
- Temperature effects
- Load current (voltage drop under load vs. at rest)

**Calibration procedure:**
1. Measure actual battery voltage with a quality multimeter
2. Note ESP32 reading
3. Calculate correction factor: `actual / measured`
4. Adjust VREF in code: `VREF × correction_factor`

### LiFePO4 Voltage Characteristics

**Nominal voltage:** 3.2V per cell × 4 cells = 12.8V

| State | Voltage (4S) | Notes |
|-------|--------------|-------|
| **Fully Charged** | 14.4 - 14.6V | Charging complete |
| **100% SOC (resting)** | 13.6V | After 30min rest |
| **90% SOC** | 13.2V | |
| **70% SOC** | 12.8V | Nominal |
| **50% SOC** | 12.8V | Flat plateau |
| **30% SOC** | 12.8V | Still flat |
| **20% SOC** | 12.4V | Starting to drop |
| **10% SOC** | 12.2V | |
| **0% SOC** | 12.0V | Recommended minimum |
| **Manufacturer Min** | 10.0V | Don't discharge this low |

**Note:** LiFePO4 has a very flat discharge curve. Most of the usable capacity is in the 12.4V-13.2V range.

### Power Consumption

**ESP32 consumption:**
- With WiFi active: 80-150mA @ 5V = 0.4-0.75W
- Deep sleep: <1mA (not used in this application)

**Voltage divider consumption (10kΩ/1kΩ):**
- At 12.8V: 1.16mA = 15mW
- At 14.6V: 1.33mA = 19mW

**Total system idle power:** ~0.5-0.8W (excluding load)

**Battery runtime without solar:**
- 100Ah battery, 0.5W draw: ~2400 hours (100 days)
- Conclusion: System overhead is negligible

### NC Relay Logic

**Normal state (relay coil NOT energized):**
- NC contact: CLOSED → Load has power ✅
- NO contact: OPEN → Not used

**Activated state (relay coil energized):**
- NC contact: OPEN → Load disconnected 🚫
- NO contact: CLOSED → Not used

**Why NC instead of NO?**
- Fail-safe: If ESP32 loses power, load stays connected (NC default closed)
- For battery protection: We need to actively energize relay to disconnect load
- More intuitive: Load "normally" has power

### Serial Output Format

CSV format for easy logging:

```
Voltage,Percent,LoadState
12.85,65,1
12.84,64,1
11.98,3,0
```

- **Field 1:** Voltage (float, 2 decimals)
- **Field 2:** Percentage (integer, 0-100)
- **Field 3:** Load state (0=OFF, 1=ON)

**Example log parsing (Python):**
```python
import serial

ser = serial.Serial('/dev/ttyUSB0', 115200)

while True:
    line = ser.readline().decode('utf-8').strip()
    try:
        voltage, percent, load = line.split(',')
        print(f"Battery: {voltage}V ({percent}%) Load: {'ON' if load=='1' else 'OFF'}")
    except:
        pass  # Skip header or error lines
```

## 📝 License

This project is open source and free to use for personal and commercial applications.

## 🤝 Contributing

Feel free to submit issues, improvements, or suggestions!

## ⚠️ Safety Warnings

- **Electrical safety:** Work with batteries carefully. Short circuits can cause fires.
- **Voltage limits:** Never exceed the voltage divider's maximum input (36V with default resistors)
- **Battery chemistry:** This code is designed for LiFePO4. Don't use with Li-ion/Li-Po without adjusting thresholds.
- **Load inductive kickback:** If switching inductive loads (motors, solenoids), add flyback protection.
- **Wire gauge:** Use appropriate wire gauge for your load current.
- **Fusing:** Consider adding a fuse on the battery positive terminal.

## 📞 Support

For questions or issues:
1. Check the [Troubleshooting](#troubleshooting) section
2. Review Serial Monitor output for error messages
3. Verify all connections match the wiring diagrams
4. Test components individually to isolate problems

---

**Made with ❤️ for off-grid and solar power systems**
