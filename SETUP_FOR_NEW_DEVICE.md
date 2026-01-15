# Setup Guide for New Device

When using this code on a different ESP32 device, here's what needs to be changed:

## MUST CHANGE (Required)

### 1. WiFi Credentials (Lines 141-142)
```cpp
const char* WIFI_SSID = "YourWiFiName";      // ← Change this
const char* WIFI_PASS = "YourPassword";      // ← Change this
```

### 2. Calibration Factor (Set via Web API after upload)
- Each ESP32 chip has different ADC reference voltage
- **Don't change in code** - set it via web API after first upload:
  ```
  http://[ESP32-IP]/settings?calibrate=1.1937
  ```
- Calibration will be saved automatically

## MAY NEED TO CHANGE (Depends on your hardware)

### 3. Relay Pin (Line 146)
```cpp
const int RELAY_PIN = 27;    // Change if relay connected to different GPIO
```
**Common alternatives:** GPIO 2, 4, 5, 18, 19, 23, etc.
**Note:** Avoid GPIO 0, 1, 3 (used for serial), GPIO 6-11 (flash), GPIO 34-39 (input only)

### 4. Relay Type (Line 151)
```cpp
const bool RELAY_ACTIVE_LOW = true;  // Change to false if relay is active HIGH
```
**How to test:** 
- If relay turns ON when you set pin LOW → `true` (most common)
- If relay turns ON when you set pin HIGH → `false`

### 5. Voltage Divider Resistors (Lines 164-165)
```cpp
const float RTOP = 100000.0;  // Change to your top resistor value (Ohms)
const float RBOT = 10000.0;   // Change to your bottom resistor value (Ohms)
```
**Common combinations:**
- 100kΩ / 10kΩ (current setup)
- 10kΩ / 1kΩ (better for stability)
- 40kΩ / 10kΩ (if you have these)

**Formula check:** Make sure `Vbat_max × (RBOT / (RTOP + RBOT)) < 3.3V`

### 6. Voltage Thresholds (Lines 241-242) - Optional
```cpp
float V_CUTOFF = 12.0;      // Default cutoff (can change via web API)
float V_RECONNECT = 12.9;   // Default reconnect (can change via web API)
```
**Note:** You can also change these via web API, so code change is optional

## DON'T NEED TO CHANGE (Works for all devices)

- ADC_PIN = 36 (GPIO36 is best for ADC with WiFi)
- VREF = 3.3 (standard ESP32 reference)
- ADC_MAX = 4095 (12-bit ADC standard)
- SAMPLES = 200 (good for all devices)
- DISPLAY_SAMPLES = 30 (good for all devices)
- All the logic code (hysteresis, cycle counting, etc.)

## Quick Setup Checklist

1. ✅ Change WiFi SSID and password
2. ✅ Check relay pin (if different from GPIO27)
3. ✅ Check relay type (active LOW vs HIGH)
4. ✅ Update resistor values (if different from 100k/10k)
5. ✅ Upload code
6. ✅ Calibrate via web API: `http://[ESP32-IP]/settings?calibrate=X.XXXX`
7. ✅ Set thresholds via web API (optional): `http://[ESP32-IP]/settings?lower=X&upper=Y`

## Example: Different Device Setup

**Device 2:**
- WiFi: "MyNetwork" / "MyPassword"
- Relay on GPIO 2
- Resistors: 10kΩ / 1kΩ
- Active HIGH relay

**Changes needed:**
```cpp
const char* WIFI_SSID = "MyNetwork";
const char* WIFI_PASS = "MyPassword";
const int RELAY_PIN = 2;
const bool RELAY_ACTIVE_LOW = false;
const float RTOP = 10000.0;
const float RBOT = 1000.0;
```

That's it! Everything else stays the same.
