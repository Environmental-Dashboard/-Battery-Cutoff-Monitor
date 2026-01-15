# Circuit Documentation

## Summary

This circuit is designed to integrate various components including resistors, a microcontroller (ESP32), a power management system, and sensors. The primary function of this circuit is to manage power distribution from a solar panel and a battery, control a relay for switching purposes, and interface with a Purple Air Sensor for environmental monitoring. The ESP32 microcontroller serves as the central processing unit, handling data acquisition and control tasks.

---

## Component List

### Resistor (10k Ohms)
**Description:** A resistor with a resistance of 10,000 Ohms.  
**Purpose:** Used for voltage division or current limiting.

### Resistor (1k Ohms)
**Description:** A resistor with a resistance of 1,000 Ohms.  
**Purpose:** Used for voltage division or current limiting.

### ESP32 (30 pin)
**Description:** A versatile microcontroller with Wi-Fi and Bluetooth capabilities.  
**Purpose:** Central processing unit for data acquisition and control.

### Electrolytic Capacitor (10µF)
**Description:** A capacitor with a capacitance of 10 microfarads.  
**Purpose:** Used for smoothing voltage fluctuations.

### 5V Step Up/Down Converter
**Description:** A power converter that can step up or step down voltage to 5V.  
**Purpose:** Provides a stable 5V output for powering components.

### Purple Air Sensor
**Description:** An air quality sensor.  
**Purpose:** Monitors environmental air quality.

### 1 Channel Relay (5V)
**Description:** A relay module that operates at 5V.  
**Purpose:** Used for switching high-power devices.

### LiFEPO4 Battery (12.8V, 18Ah)
**Description:** A rechargeable lithium iron phosphate battery.  
**Purpose:** Provides power storage for the circuit.

### Solar Panel (380W)
**Description:** A solar panel capable of generating 380 watts of power.  
**Purpose:** Provides renewable energy to the circuit.

---

## Wiring Details

### Resistor (10k Ohms)
**Pin 1:** Connected to Pin 2 of the 1k Ohm Resistor.  
**Pin 2:** Connected to the positive pin of the Electrolytic Capacitor and VP pin of the ESP32.

### Resistor (1k Ohms)
**Pin 1:** Connected to Pin 2 of the 10k Ohm Resistor.  
**Pin 2:** Connected to the ground net shared with the ESP32, Electrolytic Capacitor, and other components.

### ESP32 (30 pin)
**VP:** Connected to Pin 2 of the 10k Ohm Resistor and the positive pin of the Electrolytic Capacitor.  
**GND:** Connected to the ground net shared with the battery, solar panel, and other components.  
**Vin:** Connected to the VOUT of the 5V Step Up/Down Converter.  
**D27:** Connected to the IN pin of the 1 Channel Relay.

### Electrolytic Capacitor (10µF)
**Positive (+):** Connected to Pin 2 of the 10k Ohm Resistor and VP pin of the ESP32.  
**Negative (-):** Connected to the ground net shared with the ESP32, battery, and other components.

### 5V Step Up/Down Converter
**VIN:** Connected to the positive terminals of the battery and solar panel.  
**GND:** Connected to the ground net shared with the ESP32, battery, and other components.  
**VOUT:** Connected to the VCC of the 1 Channel Relay and Vin of the ESP32.

### Purple Air Sensor
**Positive (+):** Connected to the NC pin of the 1 Channel Relay.  
**Negative (-):** Connected to the ground net shared with the ESP32, battery, and other components.

### 1 Channel Relay (5V)
**VCC:** Connected to the VOUT of the 5V Step Up/Down Converter.  
**GND:** Connected to the ground net shared with the ESP32, battery, and other components.  
**IN:** Connected to the D2 pin of the ESP32.  
**NC:** Connected to the positive pin of the Purple Air Sensor.  
**COM:** Connected to the positive terminals of the battery and solar panel.

### LiFEPO4 Battery (12.8V, 18Ah)
**Positive (+):** Connected to the VIN of the 5V Step Up/Down Converter and COM of the 1 Channel Relay.  
**Negative (-):** Connected to the ground net shared with the ESP32, solar panel, and other components.

### Solar Panel (380W)
**Positive (+):** Connected to the VIN of the 5V Step Up/Down Converter and COM of the 1 Channel Relay.  
**Negative (-):** Connected to the ground net shared with the ESP32, battery, and other components.

---

## Circuit Diagram

View the complete wiring diagram here: [Cirkit Designer - Battery Monitor Wiring](https://app.cirkitdesigner.com/project/de4b9459-3ebd-45a3-887c-bb5d5f185a88)

---

## Code Documentation

The code for this system is documented in `voltage_meter.ino`. Key features include:

- Battery voltage monitoring via ADC
- Two-threshold hysteresis control
- Web interface for monitoring and control
- JSON API for external integration
- Automatic load protection based on battery voltage

For detailed code documentation, see the header comments in `voltage_meter.ino`.
