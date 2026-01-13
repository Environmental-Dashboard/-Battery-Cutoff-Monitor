# Voltage Threshold Quick Reference Guide

## 🎯 Where to Change Settings

**File:** `voltage_meter.ino`  
**Find this line (around line 90-140):**

```cpp
// BATTERY PROTECTION THRESHOLDS - CUSTOMIZE THESE FOR YOUR NEEDS
```

**Change these two values:**

```cpp
const float V_CUTOFF = 12.0;      // ← Change this
const float V_RECONNECT = 12.9;   // ← Change this
```

## 📊 Quick Selection Chart

Choose a profile that matches your needs:

| Profile | Cutoff | Reconnect | Who Should Use This |
|---------|--------|-----------|---------------------|
| **Maximum Protection** | 12.4V | 13.2V | Old batteries, maximize lifespan |
| **Balanced (Default)** ⭐ | 12.0V | 12.9V | Most users, good compromise |
| **Maximum Runtime** | 11.5V | 12.5V | New batteries, need every bit of power |
| **High-Power Loads** | 12.2V | 13.5V | Motors, pumps (prevent cycling) |
| **Quick Reconnect** | 12.0V | 12.5V | Stable system, fast recovery |

## 🔄 How It Works (Visual)

```
        14.0V ┤                 ┌─────  Charging
             │                 │
        13.5V ┤             ┌───┘
             │             │
        13.0V ┤         ┌───┘              
             │         │
   ┌────────────────────────────────────────────────────────┐
   │ RECONNECT → 12.9V ┤ ────────┘                          │
   │         │         │      ↑                             │
   │         │         │      └─ Load turns ON here        │
   │         12.5V ┤     │                                  │
   │         │         │      Load stays OFF               │
   │         │         │      (waiting to reconnect)       │
   │         12.2V ┤     │                                  │
   │         │         │                                    │
   │ CUTOFF → 12.0V ┤ ────────┐                            │
   │         │         │      │                             │
   │         │         │      └─ Load turns OFF here       │
   └────────────────────────────────────────────────────────┘
        11.5V ┤     └───┐
             │         │
        11.0V ┤         └───  Danger Zone!
```

## ⚠️ Golden Rules

1. **RECONNECT must be HIGHER than CUTOFF**
   - ✅ Good: Cutoff=12.0V, Reconnect=12.9V
   - ❌ Bad: Cutoff=12.9V, Reconnect=12.0V

2. **Minimum gap: 0.3V** (0.5V+ recommended)
   - ✅ Good: 0.9V gap (12.0 → 12.9)
   - ⚠️ Risky: 0.2V gap (may cause cycling)

3. **Never go below 11.0V cutoff** (damages LiFePO4)

4. **Never set reconnect above 14.0V** (unrealistic)

## 🔧 Copy-Paste Examples

Just copy these into your code!

### Example 1: Maximum Battery Protection
```cpp
const float V_CUTOFF = 12.4;      // Disconnect at 20% SOC
const float V_RECONNECT = 13.2;   // Reconnect at 85% SOC
```
**Use when:** Battery longevity is top priority

---

### Example 2: Balanced (Default) ⭐
```cpp
const float V_CUTOFF = 12.0;      // Disconnect at 5% SOC
const float V_RECONNECT = 12.9;   // Reconnect at 70% SOC
```
**Use when:** Normal operation, good for most users

---

### Example 3: Maximum Runtime
```cpp
const float V_CUTOFF = 11.5;      // Disconnect at ~1% SOC
const float V_RECONNECT = 12.5;   // Reconnect at 40% SOC
```
**Use when:** Need every bit of runtime, healthy battery

---

### Example 4: Heavy Loads (Motors, Pumps)
```cpp
const float V_CUTOFF = 12.2;      // Disconnect before voltage sag
const float V_RECONNECT = 13.5;   // Wait for full recharge
```
**Use when:** Load causes big voltage drops

---

### Example 5: Quick Reconnect
```cpp
const float V_CUTOFF = 12.0;      // Standard cutoff
const float V_RECONNECT = 12.5;   // Fast reconnect
```
**Use when:** Small loads, fast solar recovery

---

### Example 6: Conservative (Winter/Cold)
```cpp
const float V_CUTOFF = 12.6;      // Early disconnect
const float V_RECONNECT = 13.3;   // Wait for good charge
```
**Use when:** Cold weather, less solar, protect battery

## 📝 After Changing Values

1. **Save the file** (Ctrl+S / Cmd+S)
2. **Upload to ESP32** (Click → button)
3. **Open Serial Monitor** (115200 baud)
4. **Verify your values:**
   ```
   Cutoff: 12.00V, Reconnect: 12.90V  ← Check this line
   ```
5. **Test for a few days** and adjust if needed

## 🐛 Troubleshooting

**Problem: Load keeps cycling on/off**
- **Fix:** Increase the gap
- Change from 12.0→12.5 (0.5V gap) to 12.0→13.0 (1.0V gap)

**Problem: Load disconnects too early**
- **Fix:** Lower V_CUTOFF
- Change from 12.4V to 12.0V or 11.8V

**Problem: Takes forever to reconnect**
- **Fix:** Lower V_RECONNECT
- Change from 13.5V to 12.9V or 12.6V

**Problem: Battery seems damaged**
- **Fix:** Raise V_CUTOFF (you're discharging too deeply!)
- Never go below 11.5V, consider raising to 12.2V+

## 🔍 Monitor Your System

Watch the Serial Monitor output:
```
12.45,25,1    ← 12.45V, 25%, Load ON
12.32,18,1
12.00,5,0     ← Disconnected here! (hit cutoff)
12.15,8,0     ← Still off (below reconnect)
12.90,70,1    ← Reconnected here! (hit reconnect)
```

Or use the web interface at `http://[ESP32_IP]/`

---

**Need more details?** See the full [README.md](README.md) file, especially the [Customizing Voltage Thresholds](README.md#customizing-voltage-thresholds) section.
