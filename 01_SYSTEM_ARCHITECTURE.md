# System Architecture and Technical Design

## 1. Architecture goal

The architecture must support low-cost monitoring, local safety logic, explainable diagnosis, and optional cloud analytics. It should continue showing a local condition even if Wi-Fi is unavailable.

## 2. Logical layers

```text
┌──────────────────────────────────────────────────────────────────┐
│  Physical Layer                                                   │
│  PV panel | V/I measurement | panel temperature | light proxy     │
└───────────────────────────────┬──────────────────────────────────┘
                                │ I2C / ADC / OneWire
┌───────────────────────────────▼──────────────────────────────────┐
│  Edge Layer: ESP32                                                │
│  validation → filtering → feature engine → rules → ML → display  │
└───────────────────────────────┬──────────────────────────────────┘
                                │ Wi-Fi / MQTT / HTTPS
┌───────────────────────────────▼──────────────────────────────────┐
│  Service Layer                                                     │
│  MQTT broker or Firebase → database → alert service → REST API    │
└───────────────────────────────┬──────────────────────────────────┘
                                │
┌───────────────────────────────▼──────────────────────────────────┐
│  Presentation Layer                                                │
│  Live dashboard | diagnosis timeline | calibration | CSV export    │
└──────────────────────────────────────────────────────────────────┘
```

## 3. Hardware data flow

1. The panel supplies a low-power test load.
2. INA219 measures bus voltage/current where its measurement range is appropriate.
3. DS18B20 measures temperature on the panel back sheet.
4. BH1750 or a calibrated LDR circuit estimates ambient light as `G_proxy`.
5. ESP32 samples sensors every 1–5 seconds.
6. The firmware timestamps, validates, filters, and calculates physical/virtual features.
7. Firmware determines immediate state and publishes a compact telemetry packet.
8. OLED and RGB LED show local status whether or not network connectivity is present.

## 4. Edge processing pipeline

```text
Read sensors
   ↓
Range and missing-value checks
   ↓
Filter / smooth values (median or moving average)
   ↓
Calculate P_meas, P_exp, PR, ΔP, ΔV, ΔI, rolling features
   ↓
Safety rules and sensor-fault handling
   ↓
Optional ML inference over N-sample window
   ↓
Fuse rule + ML result into final state
   ↓
Update OLED / LED / local buzzer
   ↓
Publish and log telemetry
```

## 5. Fault-decision design

### Precedence order

1. **Sensor fault / invalid data**: prevent incorrect PV diagnosis.
2. **Immediate electrical safety condition**: e.g., abnormal low voltage/current pattern in test context.
3. **Open-circuit rule**: high light proxy with near-zero current.
4. **ML or threshold-based persistent-performance diagnosis**: shading, soiling/degradation, normal.
5. **Unknown/needs inspection**: low confidence or conflicting signals.

### State output

| Field | Example |
|---|---|
| `status` | `PARTIAL_SHADING` |
| `severity` | `warning` |
| `confidence` | `0.82` |
| `reason` | `PR=0.57; ΔP high; G_proxy stable` |
| `recommended_action` | `Inspect panel for cover/obstruction` |
| `health_score` | `58` |

## 6. Virtual feature engine

### Required features

```text
P_meas = V × I
P_exp = P_stc × (G_proxy/G_ref) × (1 + gamma × (T_panel − 25))
PR = P_meas / max(P_exp, epsilon)
DeltaP = P_exp − P_meas
DeltaP_pct = 100 × DeltaP / max(P_exp, epsilon)
eta_proxy = P_meas / max(G_proxy × area, epsilon)
```

### Window features

For a window of 30–60 seconds, calculate:

- mean and standard deviation of `V`, `I`, `P_meas`, `PR`, and `T_panel`
- min/max power
- power slope `dP/dt`
- variance of irradiance proxy
- duration of persistent low PR
- indicator flags from safety rules

## 7. Telemetry contract

### Suggested MQTT topic structure

```text
pvdiag/<device_id>/telemetry
pvdiag/<device_id>/event
pvdiag/<device_id>/command
pvdiag/<device_id>/status
```

### Example telemetry JSON

```json
{
  "device_id": "pv-node-01",
  "timestamp": "2026-10-22T12:00:05+05:30",
  "voltage_v": 17.42,
  "current_a": 0.38,
  "power_w": 6.62,
  "panel_temp_c": 41.8,
  "g_proxy": 712.0,
  "p_expected_w": 10.16,
  "pr": 0.652,
  "delta_p_w": 3.54,
  "health_score": 62,
  "status": "PARTIAL_SHADING",
  "confidence": 0.84,
  "wifi_rssi": -61
}
```

### Example event JSON

```json
{
  "event_id": "evt-20261022-001",
  "device_id": "pv-node-01",
  "timestamp": "2026-10-22T12:00:05+05:30",
  "type": "FAULT_TRANSITION",
  "from": "NORMAL",
  "to": "OPEN_CIRCUIT",
  "severity": "critical",
  "reason": "High G_proxy and current near zero for 45 s"
}
```

## 8. Dashboard design

### Live page

- Voltage, current, measured power
- Panel temperature and light proxy
- Expected power and PR
- Current class, confidence, health score
- Latest alert and Wi-Fi/edge status

### Diagnosis page

- Current fault state
- Top contributing indicators
- Rule flags and ML confidence
- Recommended operator action

### History page

- Power and expected-power trend
- PR trend and fault-event timeline
- Filter by date and fault class
- CSV export

### Impact page

- Estimated energy not produced during flagged periods
- Number of events and duration
- Availability / data completeness

### Admin/calibration page

- `P_stc`, temperature coefficient `gamma`, area, and thresholds
- Sensor calibration offset
- Dataset label mode
- Device heartbeat and firmware version

## 9. Backend options

| Option | When to choose it | Trade-off |
|---|---|---|
| ThingsSpeak | Fastest proof-of-concept charts | Limited custom workflow |
| Firebase | Quick web/mobile integration and live updates | Vendor service dependency |
| MQTT + Flask + SQLite | Strong learning value and full control | More setup and maintenance |
| MQTT + InfluxDB + Grafana | Good time-series capability | Higher setup complexity |

### Recommended student path

Start with ESP32 + CSV/MQTT + simple Flask dashboard. Add Firebase or a managed platform only if it reduces demo risk.

## 10. Reliability and security

- Use topic/device access control where MQTT is used.
- Do not embed production passwords in public repositories.
- Use TLS if the selected service supports it and setup is practical.
- Validate commands; do not expose direct switching/control functions casually.
- Include a local-safe default when Wi-Fi or backend is unavailable.
- Record firmware version, uptime, and sensor-fault states.
- Buffer a small number of readings locally if the network drops.

## 11. Architecture acceptance checks

| Check | Pass condition |
|---|---|
| Sensor loop | New valid reading every configured sample interval |
| Local operation | OLED state changes even with Wi-Fi disabled |
| Telemetry | Backend receives values with correct timestamps and units |
| Feature engine | `P_meas`, `P_exp`, and PR computed without invalid divide-by-zero behaviour |
| Alert workflow | Event is produced only after persistence threshold is reached |
| Recovery | Device reconnects after Wi-Fi restart without manual reflash |
| Data quality | Missing/out-of-range values are marked, not silently treated as normal |
