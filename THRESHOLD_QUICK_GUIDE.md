# Voltage Threshold Quick Guide

**Quick reference for changing when your load turns on and off**

---

## Where to Change Settings

**File:** `voltage_meter.ino`  
**Find this (around line 147):**

```cpp
const float V_CUTOFF = 8.0;      // ← Change this number
const float V_RECONNECT = 12.0;  // ← Change this number
```

---

## How It Works

```
Battery Voltage
    14V ┤                    ┌──── Charging
    13V ┤                ┌───┘
    12V ┤ ━━━━━━━━━━━━━━┥ RECONNECT (Load turns ON here)
    11V ┤           ↑    │
    10V ┤           │    │  Load stays OFF while voltage rises
     9V ┤           │    │
     8V ┤ CUTOFF ━━━┥    │ (Load turns OFF here)
     7V ┤           ↓
```

**Example:**
- Battery drains to **8.0V** → Load turns OFF 🔴
- Battery charges to **12.0V** → Load turns ON 🟢
- The gap (4V) prevents rapid on/off switching

---

## Ready-to-Use Examples

Just copy and paste these into your code!

### Example 1: Default (8V - 12V) ⭐
```cpp
const float V_CUTOFF = 8.0;      
const float V_RECONNECT = 12.0;  
```
**Good for:** Most applications, large voltage range

---

### Example 2: 12V Car Battery (Lead Acid)
```cpp
const float V_CUTOFF = 11.5;     
const float V_RECONNECT = 12.4;  
```
**Good for:** Lead acid batteries, prevents deep discharge

---

### Example 3: 12V LiFePO4 Battery
```cpp
const float V_CUTOFF = 12.0;     
const float V_RECONNECT = 12.9;  
```
**Good for:** LiFePO4 batteries, protects cells

---

### Example 4: Maximum Runtime
```cpp
const float V_CUTOFF = 10.0;     
const float V_RECONNECT = 12.0;  
```
**Good for:** When you need every bit of battery capacity

---

### Example 5: Quick Reconnect
```cpp
const float V_CUTOFF = 10.0;     
const float V_RECONNECT = 11.0;  
```
**Good for:** Fast charging systems, small voltage gap

---

### Example 6: Conservative (Protect Battery)
```cpp
const float V_CUTOFF = 12.0;     
const float V_RECONNECT = 13.5;  
```
**Good for:** Maximize battery life, wait for full charge

---

## Important Rules

✅ **RECONNECT must be HIGHER than CUTOFF**
- Good: Cutoff=8V, Reconnect=12V ✅
- Bad: Cutoff=12V, Reconnect=8V ❌

✅ **Minimum gap: 0.5V** (1V or more recommended)
- Good: 8V → 12V (4V gap) ✅
- Risky: 11V → 11.5V (0.5V gap) ⚠️

✅ **Don't go too low**
- Most batteries shouldn't go below 10V
- Default 8V is for emergencies

✅ **Test your settings**
- Monitor for a few days
- Adjust if needed

---

## After Changing Values

1. **Save the file** (Ctrl+S or Cmd+S)
2. **Upload to ESP32** (Click → button)
3. **Hold BOOT button** when "Connecting..." appears
4. **Check Serial Monitor** (115200 baud):
   ```
   Cutoff: 8.00V, Reconnect: 12.00V  ← Verify your values
   ```
5. **Open web page** and test!

---

## Troubleshooting

### Load keeps cycling on/off rapidly

**Problem:** Gap is too small or battery voltage sags under load

**Fix:** Increase the gap
```cpp
// BEFORE (bad)
const float V_CUTOFF = 11.0;
const float V_RECONNECT = 11.5;   // Only 0.5V gap!

// AFTER (good)
const float V_CUTOFF = 11.0;
const float V_RECONNECT = 12.5;   // 1.5V gap
```

---

### Load turns off too early

**Problem:** Cutoff voltage is too high

**Fix:** Lower the cutoff
```cpp
// BEFORE
const float V_CUTOFF = 12.0;      // Too high

// AFTER
const float V_CUTOFF = 10.5;      // Lower = more runtime
```

---

### Takes forever to turn back on

**Problem:** Reconnect voltage is too high

**Fix:** Lower the reconnect
```cpp
// BEFORE
const float V_RECONNECT = 14.0;   // Too high

// AFTER
const float V_RECONNECT = 12.0;   // Reconnects faster
```

---

## Monitor Your System

### Serial Monitor Output (CSV format)
```
11.87,57,0    ← Voltage, Percent, Load (0=OFF, 1=ON)
11.85,57,0
12.00,60,1    ← Load turned ON at 12V!
12.15,62,1
```

### Web Dashboard
- Shows voltage, percentage, and load status
- Updates every second
- Control buttons for testing

---

## Relay Information

**Your system uses NO (Normally Open) relay:**

```
Relay OFF → Load has NO power 🔴
Relay ON  → Load has power 🟢
```

**Wiring:**
- Connect power to **COM** terminal
- Connect load to **NO** terminal (NOT NC!)
- Don't use the NC terminal

---

## Quick Reference Table

| Cutoff | Reconnect | Gap | Use Case |
|--------|-----------|-----|----------|
| 8.0V | 12.0V | 4.0V | Default, wide range |
| 10.0V | 12.0V | 2.0V | More runtime |
| 11.5V | 12.4V | 0.9V | Car battery (lead acid) |
| 12.0V | 12.9V | 0.9V | LiFePO4 battery |
| 12.0V | 13.5V | 1.5V | Conservative, protect battery |
| 10.0V | 11.0V | 1.0V | Fast reconnect |

---

## Need More Help?

See the full **README.md** for:
- Complete wiring diagrams
- Troubleshooting guide
- Web interface instructions
- Safety information

---

**Quick tip:** Start with the default settings (8V/12V) and adjust based on your battery type and how the system behaves!
