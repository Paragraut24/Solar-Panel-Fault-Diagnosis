# Solar PV Fault Diagnosis with Virtual Electrical Sensors

An Edge-AI IoT system for real-time solar photovoltaic (PV) fault detection, virtual feature synthesis, and telemetry streaming.

---

## Repository Structure

```text
Solar-Panel-Fault-Diagnosis/
├── firmware/
│   ├── sensor_test/              # I2C/ADC sanity test sketch (INA219, BH1750, LM35/NTC)
│   │   └── sensor_test.ino
│   └── main_node/                # Production ESP32 telemetry edge node
│       └── main_node.ino
├── data/
│   ├── raw/                      # Raw, unedited CSV telemetry captures
│   ├── processed/                # Cleaned, feature-engineered datasets
│   └── metadata/                 # YAML and experiment session logs
├── notebooks/
│   └── firebase_to_csv.py        # Exporter from Firebase RTDB to standard CSV
├── dashboard/
│   └── index.html                # Single-page real-time telemetry viewer (Chart.js + Firebase)
├── docs/
│   └── context/                  # Master specifications, hardware BOM, ML research, and roadmap
│       ├── 00_MASTER_CONTEXT.md
│       ├── 01_SYSTEM_ARCHITECTURE.md
│       ├── 02_HARDWARE_AND_SAFETY.md
│       ├── 03_DATA_ML_RESEARCH.md
│       ├── 04_ROADMAP_AND_DELIVERABLES.md
│       └── getting-started.md
├── .gitignore                    # Secrets, firmware build binaries, and python caches
└── README.md
```

---

## Hardware Specification & Bill of Materials

- **Microcontroller:** ESP32 NodeMCU-32S / ESP-WROOM-32
- **PV Panel:** 10 W to 30 W polycrystalline test panel
- **Electrical Sensor:** INA219 High-Side DC Voltage & Current (I2C address `0x40`)
- **Irradiance Proxy:** BH1750 Ambient Light Sensor (I2C address `0x23`)
- **Temperature Sensor:** LM35 analog temperature sensor or 10k NTC thermistor (ADC Pin 34)
- **Safety Elements:** 2 A fast-acting fuse, inline toggle switch, 10–50 W rated power resistor

---

## Telemetry Data Schema

Every telemetry packet streamed to Firebase Realtime Database follows this schema:

| Field | Type | Description |
|---|---|---|
| `timestamp` | integer | Unix epoch timestamp (seconds or milliseconds) |
| `experiment_id` | string | Session identifier (e.g. `EXP-001`) |
| `label` | string | Operating state (`normal`, `partial_shading`, `open_circuit`, `soiling`, `line_fault`) |
| `coverage_pct` | float | Percentage of panel surface covered under shading experiments |
| `load_state` | string | State of load connection (`resistive`, `open`, `dynamic`) |
| `voltage_v` | float | Measured panel terminal voltage (V) |
| `current_a` | float | Measured panel current (A) |
| `power_w` | float | Measured electrical power (`V × I`) |
| `panel_temp_c` | float | Measured panel surface temperature (°C) |
| `g_proxy` | float | Ambient light / irradiance proxy (lux) |

---

## Quickstart Guide

### 1. Hardware Verification
1. Open and flash `firmware/sensor_test/sensor_test.ino` to the ESP32.
2. Open Serial Monitor at 115200 baud to confirm `INA219` and `BH1750` detect at their I2C addresses.
3. Validate voltage and current measurements against a physical digital multimeter.

### 2. Live Telemetry Node
1. Update `WIFI_SSID`, `WIFI_PASSWORD`, `API_KEY`, and `DATABASE_URL` in `firmware/main_node/main_node.ino`.
2. Set `CURRENT_LABEL` (e.g. `normal`) and `EXPERIMENT_ID` (e.g. `EXP-001`).
3. Flash the sketch to the ESP32 to begin streaming to Firebase RTDB.

### 3. Real-Time Web Dashboard
1. Open `dashboard/index.html` in any modern web browser.
2. Enter your Firebase Database URL (e.g. `https://<project-id>-default-rtdb.firebaseio.com`).
3. Click **Connect Stream** to view live metric cards, a rolling 5-minute chart, and recent telemetry rows.

### 4. Exporting Data to CSV
Run the automated exporter script from the `notebooks` directory:

```bash
# Export from a downloaded Firebase JSON export
python notebooks/firebase_to_csv.py --input exported_telemetry.json --output data/raw/EXP-001_normal.csv

# Or fetch directly via Firebase REST endpoint
python notebooks/firebase_to_csv.py --db-url "https://<project-id>-default-rtdb.firebaseio.com" --output data/raw/EXP-001_normal.csv
```

---

## Safety and Research Context
Refer to the comprehensive guides in [`docs/context/`](file:///e:/SEM%205/AIOT/CP/Solar-Panel-Fault-Diagnosis/docs/context/) for detailed circuit schematics, risk matrices, feature engineering formulas (performance ratio, power residual, estimated efficiency), and the 10-week execution roadmap.
