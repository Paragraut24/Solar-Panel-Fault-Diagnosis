# Hardware, Wiring, and Safety Guide

## 1. Recommended MVP build

Use a **small 10–30 W PV panel** for the first working prototype. This keeps current, energy, and failure risk manageable while still producing meaningful voltage/current variations under sun or a controlled light source.

## 2. Bill of materials

| Component | Purpose | Quantity | Approx. student cost (INR) |
|---|---:|---:|---:|
| 10–30 W PV panel | Device under test | 1 | 800–2,000 |
| ESP32 DevKit | Edge controller and Wi-Fi | 1 | 250–400 |
| INA219 | Current and bus-voltage sensing | 1 | 150–300 |
| DS18B20 | Panel-back temperature | 1 | 80–150 |
| BH1750 or LDR circuit | Irradiance/light proxy | 1 | 50–200 |
| 0.96-inch I2C OLED | Local display | 1 | 120–200 |
| RGB LED + resistors | Local health indication | 1 | 30–60 |
| Buzzer (optional) | Local critical alert | 1 | 30–60 |
| Fused resistive load / power resistor | Safe electrical load | 1 | 100–300 |
| Inline fuse + holder | Circuit protection | 1 | 50–100 |
| Connectors, switch, wire, terminal blocks | Safe assembly | set | 150–300 |
| Power bank or buck converter | ESP32 supply | 1 | 300–500 |
| Enclosure and mounting material | Protection and presentation | set | 200–400 |

**Estimated ESP32 MVP total:** roughly ₹2,500–5,000.  
**Optional:** Raspberry Pi Zero 2 W/4 for heavier local models or hosting; MLX90614 for contactless thermal indication.

## 3. Sensor choices and limitations

### INA219

- Measures current and bus voltage over I2C.
- Convenient for low-power DC measurements and easy ESP32 integration.
- Verify that panel voltage/current remain within the module’s safe range.
- If panel conditions may exceed the INA219 voltage range, use an appropriate divider/ADC or choose a sensor rated for the application.

### DS18B20

- Place on the back sheet of the panel using thermally conductive tape/paste and insulation from direct wind where practical.
- It estimates panel-back temperature, not exact cell-junction temperature.

### BH1750 / LDR

- Use only as an **irradiance proxy**.
- BH1750 is digitally convenient but measures illuminance, not calibrated solar irradiance.
- LDR is cheaper but nonlinear and sensitive to spectrum/temperature.
- Record a clear-condition reference and calibrate a practical `G_proxy` scale for the same panel setup.

## 4. Connection concept

```text
PV positive ── fuse ── INA219 current path ── controlled load ── PV negative
                      │
                      └── measurement to ESP32 through I2C

ESP32 I2C bus
  ├── INA219
  ├── BH1750
  └── OLED

ESP32 OneWire / GPIO
  └── DS18B20

ESP32 GPIO
  ├── RGB LED (through resistors)
  └── buzzer (optional, transistor drive if required)
```

### Important implementation note

The exact GPIO pins can be chosen to fit the ESP32 board and libraries. Typical ESP32 I2C defaults are GPIO 21 (SDA) and GPIO 22 (SCL), but confirm your board configuration before wiring. DS18B20 needs a pull-up resistor (commonly 4.7 kΩ) between data and VCC.

## 5. Suggested local display

```text
PV HEALTH MONITOR
STATE: PARTIAL SHADE
P: 6.62 W / 10.16 W
PR: 0.65   H: 62/100
Wi-Fi: ONLINE
```

### LED status

| Colour | State |
|---|---|
| Green | Normal / healthy |
| Yellow | Warning: partial shade or underperformance |
| Red | Critical: open circuit / abnormal electrical condition |
| Blue or blinking white | Sensor/network/calibration mode |

## 6. Controlled fault creation

| Condition | Safe demonstration method |
|---|---|
| Normal | Clean panel, stable load, full available light |
| Partial shading | Cover 25%, 50%, or 75% of selected cells with card/mesh; record coverage pattern |
| Open circuit | Use a switch or safely disconnect the load path while measuring the condition |
| Soiling proxy | Use removable translucent film/light dust only if it can be cleaned safely; document exact procedure |
| Degradation proxy | Use a known series resistor/load condition as a proxy, not as a claim of real material aging |
| Short/line fault proxy | Prefer a controlled resistor/load path in a tiny fused setup; do not hard-short high-power panels |

## 7. Mandatory safety rules

1. Use only low-power panels for experimental faults.
2. Install an inline fuse close to the panel positive terminal.
3. Use insulated connectors and avoid exposed metal conductors.
4. Do not work on a wet panel, in rain, or near water.
5. Never short a rooftop string or a high-power panel.
6. Do not connect the test circuit to grid-tied inverter wiring.
7. Check component voltage/current ratings before connection.
8. Keep lithium batteries/power banks away from heat and direct sun.
9. Use a multimeter to validate voltage before connecting the ESP32 sensor board.
10. Have a supervisor present for any intentionally abnormal electrical test.

## 8. Pre-power checklist

- [ ] Panel open-circuit voltage is within expected safe range.
- [ ] Fuse is installed and correctly rated for the small test circuit.
- [ ] Polarities have been checked with a multimeter.
- [ ] Current path passes through the sensor correctly.
- [ ] ESP32 ground/reference is connected according to the sensor-module requirements.
- [ ] No exposed loose wires or damaged insulation exist.
- [ ] Load resistor has adequate power rating and is not touching flammable material.
- [ ] OLED and sensors power up normally from a separate safe supply.

## 9. Sensor validation procedure

1. Record panel voltage using a multimeter and firmware output; compare values.
2. Record current with a known stable load; compare INA219 and multimeter values.
3. Place DS18B20 beside a reference thermometer for a simple sanity check.
4. Verify light-sensor readings change consistently between shade and full light.
5. Record calibration offsets and date in the experiment log.
6. Reject or flag readings that are impossible, missing, or outside hardware limits.

## 10. Common hardware issues

| Symptom | Likely cause | First check |
|---|---|---|
| Current always zero | Wrong sensor path, loose load wiring, I2C issue | Series current path and sensor address |
| Negative/incorrect current | Sensor orientation or sign convention | Current direction and code sign handling |
| Voltage reading too low | Divider/sensor range issue or wiring loss | Multimeter comparison and module rating |
| No DS18B20 data | Missing pull-up or wrong GPIO | 4.7 kΩ resistor and address scan |
| Unstable light values | Sun/cloud variation or sensor position | Mounting, averaging window, calibration |
| ESP32 resets | Weak power supply or shared noisy supply | Separate regulated supply and grounding |
| Hot resistor | Load undersized | Power rating and airflow; stop test immediately |
