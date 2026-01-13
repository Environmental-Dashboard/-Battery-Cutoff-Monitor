# Battery Cutoff Monitor - ESP32

**Simple battery protection system that automatically disconnects your load when voltage gets too low.**

🔋 **Protects your battery** • 🌐 **Web dashboard** • ⚙️ **Easy to customize**

---

## What Does It Do?

This device watches your battery voltage. When the voltage drops too low (default: **8V**), it automatically turns OFF your load. When the battery charges back up (to **12V**), it turns the load back ON.

**Current Settings:**
- 📉 **Turns OFF at:** 8.0V
- 📈 **Turns ON at:** 12.0V
- 🔌 **Relay Type:** NO (Normally Open)

---

## Quick Start

### 1. What You Need

| Item | Example |
|------|---------|
| ESP32 board | Any ESP32 dev board |
| Relay module | 5V relay with NO contact |
| 2 Resistors | 10kΩ and 1kΩ |
| 1 Capacitor | 0.1µF to 1µF |
| Battery | Your battery (works with 8V-16V) |
| Wires | Jumper wires |

### 2. Wire It Up

#### A. Measure Battery Voltage (Voltage Divider)

```
Battery + ──→ 10kΩ resistor ──→ [Middle Point] ──→ 1kΩ resistor ──→ GND
                                      ↓
                                  GPIO36 (VP)
                                      ↓
                              Capacitor to GND
```

**Why?** The ESP32 can only read 0-3.3V. The resistors divide your battery voltage down to a safe level.

#### B. Connect the Relay (NO - Normally Open)

```
Power Supply + ──→ Relay COM
Relay NO ──→ Your Load + (sensor, lights, etc.)
Load GND ──→ Common GND
```

**Important:** Use the **NO (Normally Open)** terminal, not NC!

- When relay is OFF → NO is open → Load has no power
- When relay is ON → NO closes → Load gets power

#### C. Control the Relay

```
ESP32 GPIO2 ──→ Relay IN pin
ESP32 GND ──→ Relay GND
5V ──→ Relay VCC (power for relay)
```

#### D. Power the ESP32

```
5V power supply ──→ ESP32 VIN (or 5V pin)
GND ──→ ESP32 GND
```

**⚠️ CRITICAL:** All grounds must be connected together!

---

## Software Setup

### Step 1: Install Arduino IDE

Download from: [arduino.cc](https://www.arduino.cc/en/software)

### Step 2: Add ESP32 Support

1. Open Arduino IDE
2. Go to: **File → Preferences**
3. In "Additional Board Manager URLs", paste:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Go to: **Tools → Board → Boards Manager**
5. Search "ESP32" and click **Install**

### Step 3: Configure WiFi

Open `voltage_meter.ino` and change these lines (around line 61):

```cpp
const char* WIFI_SSID = "YourWiFiName";      // ← Your WiFi name
const char* WIFI_PASS = "YourPassword";      // ← Your WiFi password
```

### Step 4: Upload to ESP32

1. Connect ESP32 to computer via USB
2. **Tools → Board** → Select "ESP32 Dev Module"
3. **Tools → Port** → Select your port (like `/dev/tty.usbserial-0001`)
4. **Tools → Upload Speed** → Select "115200"
5. Click **Upload** button (→)
6. **If it says "Connecting..."** → Hold the BOOT button on your ESP32
7. Wait for "Done uploading"

### Step 5: View Results

1. **Tools → Serial Monitor**
2. Set baud to **115200**
3. Press RESET button on ESP32
4. You'll see the IP address printed
5. Open that IP in your web browser!

---

## Web Dashboard

Once running, open your browser to the IP address shown (example: `http://10.17.192.114`)

**You'll see:**
- 🔋 **Battery Voltage** (updates every second)
- 📊 **Battery Percentage** (estimated)
- 🔴/🟢 **Load Status** (ON or OFF)
- 🎛️ **Control Buttons:**
  - **Auto Mode** - Automatic control (turns off at 8V, on at 12V)
  - **Force ON** - Manually turn load on (ignores voltage)
  - **Force OFF** - Manually turn load off

---

## Customize the Voltage Thresholds

Want to change when it turns on/off? Easy!

### Find This Section in the Code (around line 147):

```cpp
const float V_CUTOFF = 8.0;      // ← Change this
const float V_RECONNECT = 12.0;  // ← Change this
```

### Examples:

**For 12V car battery (lead acid):**
```cpp
const float V_CUTOFF = 11.5;     // Turn off at 11.5V
const float V_RECONNECT = 12.4;  // Turn on at 12.4V
```

**For 12V LiFePO4 battery:**
```cpp
const float V_CUTOFF = 12.0;     // Turn off at 12.0V
const float V_RECONNECT = 12.9;  // Turn on at 12.9V
```

**For more runtime (use more battery capacity):**
```cpp
const float V_CUTOFF = 10.0;     // Turn off at 10V
const float V_RECONNECT = 12.0;  // Turn on at 12V
```

**⚠️ Important Rules:**
- RECONNECT voltage must be HIGHER than CUTOFF voltage
- Larger gap = less on/off cycling
- Never go below 8V for most batteries

After changing, just **upload the code again**!

---

## Troubleshooting

### Problem: Wrong voltage reading

**Fix:** Check your resistor values with a multimeter. If they're different, update these lines:

```cpp
const float RTOP = 10000.0;   // Change to your actual top resistor value
const float RBOT = 1000.0;    // Change to your actual bottom resistor value
```

### Problem: Voltage jumps around

**Fix:** 
- Add a bigger capacitor (try 1µF instead of 0.1µF)
- Keep wires short
- Check all connections

### Problem: Relay doesn't click

**Fix:**
- Check relay has 5V power (VCC pin)
- Verify GPIO2 is connected to relay IN
- Check if your relay is active HIGH or active LOW
- If it's backwards, change this line (around line 68):

```cpp
const bool RELAY_ACTIVE_LOW = true;   // Change to false if needed
```

### Problem: Can't upload code

**Fix:**
1. Close Serial Monitor
2. Try different USB port
3. Use `/dev/tty.usbserial-*` instead of `cu.usbserial-*`
4. Hold BOOT button while clicking upload

### Problem: Load doesn't turn on

**Fix:**
- Make sure you're using **NO (Normally Open)** terminal on relay, not NC
- Check all grounds are connected
- Try clicking "Force ON" button on web page
- Check relay is getting 5V power

---

## How It Works (Simple Explanation)

### The Protection Cycle

```
Battery is at 13V
        ↓
   [Load is ON] ✅
        ↓
Battery drains while load runs
        ↓
Voltage drops to 8V
        ↓
 [Load Turns OFF] 🔴 (relay opens)
        ↓
Battery charges (solar, charger, etc.)
        ↓
Voltage rises: 8V → 9V → 10V → 11V → 12V
        ↓
 [Load Turns ON] 🟢 (relay closes)
        ↓
Cycle repeats
```

### Why Two Different Voltages?

**Without gap (bad):**
- Battery at 10V → Load OFF
- Load turns off → Voltage rises to 10.1V
- Voltage above 10V → Load ON
- Load turns on → Voltage drops to 10V
- **Result:** Relay clicks on/off rapidly! ⚡

**With gap (good):**
- Battery at 8V → Load OFF
- Must charge all the way to 12V before turning back on
- **Result:** Smooth operation ✅

The gap between 8V and 12V prevents this rapid cycling.

---

## Relay Wiring: NO vs NC

Your system uses **NO (Normally Open)** contact.

### What's the difference?

**NO (Normally Open) - What you have ✅**
```
Relay OFF → Contact OPEN → Load has NO power
Relay ON  → Contact CLOSED → Load has power
```

**NC (Normally Closed) - Not using this**
```
Relay OFF → Contact CLOSED → Load has power
Relay ON  → Contact OPEN → Load has NO power
```

### How to identify on your relay:

Most relay modules have 3 terminals:
- **COM** (Common) - Connect your power supply here
- **NO** (Normally Open) - Connect your load here ← **Use this one**
- **NC** (Normally Closed) - Don't use this

---

## Technical Specs

| Specification | Value |
|--------------|-------|
| **Input Voltage Range** | 8V - 16V (can measure higher with different resistors) |
| **Default Cutoff** | 8.0V |
| **Default Reconnect** | 12.0V |
| **Relay Type** | NO (Normally Open) |
| **ESP32 Pin (ADC)** | GPIO36 (VP) |
| **ESP32 Pin (Relay)** | GPIO2 |
| **Update Rate** | 4 times per second (250ms) |
| **Web Update** | Once per second |
| **WiFi** | 2.4GHz only |

---

## JSON API

Want to read the data from another program? Use the JSON endpoint:

**URL:** `http://[ESP32_IP]/status.json`

**Example response:**
```json
{
  "voltage_v": 11.87,
  "percent": 57,
  "load_on": false,
  "auto_mode": true,
  "uptime_ms": 123456
}
```

**Example in Python:**
```python
import requests
response = requests.get('http://10.17.192.114/status.json')
data = response.json()
print(f"Battery: {data['voltage_v']}V")
print(f"Load: {'ON' if data['load_on'] else 'OFF'}")
```

**Example in curl:**
```bash
curl http://10.17.192.114/status.json
```

---

## Safety Notes

⚠️ **Important Safety Information:**

1. **Check your battery voltage rating** - Don't exceed the voltage divider limits
2. **All grounds must be connected** - Battery GND, ESP32 GND, relay GND, load GND
3. **Check relay current rating** - Make sure it can handle your load current
4. **Use appropriate wire gauge** - Thin wires can overheat with high current
5. **Test without load first** - Verify voltage readings before connecting your actual load
6. **Never short circuit the battery** - Always double-check connections

---

## Files in This Project

- **`voltage_meter.ino`** - Main code (upload this to ESP32)
- **`README.md`** - This file (instructions)
- **`THRESHOLD_QUICK_GUIDE.md`** - Quick reference for changing thresholds

---

## Support

**Having issues?**

1. Check the [Troubleshooting](#troubleshooting) section above
2. Verify all wiring matches the diagrams
3. Open Serial Monitor (115200 baud) to see debug messages
4. Check that all grounds are connected together

**Common mistakes:**
- ❌ Using NC instead of NO on the relay
- ❌ Forgetting to connect all grounds together
- ❌ Wrong resistor values in voltage divider
- ❌ Serial Monitor open while trying to upload
- ❌ Relay VCC not connected to 5V

---

## License

Open source and free to use. Modify as needed for your project!

---

**Made for protecting batteries and making monitoring easy! 🔋⚡**

**GitHub:** [https://github.com/Environmental-Dashboard/-Battery-Cutoff-Monitor](https://github.com/Environmental-Dashboard/-Battery-Cutoff-Monitor)
