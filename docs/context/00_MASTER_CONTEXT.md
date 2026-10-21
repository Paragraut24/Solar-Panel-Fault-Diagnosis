# Master Context: Solar PV Fault Diagnosis Using Virtual Electrical Sensors

## 1. Project identity

| Item | Definition |
|---|---|
| Working title | **Virtual Electrical Sensor Framework for Edge-AI Solar PV Fault Diagnosis** |
| Domain | AIoT, edge computing, embedded sensing, machine learning, renewable energy |
| Prototype type | Low-cost monitoring and diagnostic assistant for a small solar PV panel/string |
| Core novelty | Fusion of physical sensors with **virtual electrical sensors** derived from a simplified PV/digital-twin model |
| Intended users | Students, campus-lab teams, small rooftop PV maintainers, researchers |
| Academic alignment | AIOT: sensors and virtualisation, IoT analytics, edge AI, hardware interfacing, dashboarding |

## 2. The problem

Solar photovoltaic panels lose output because of partial shading, loose/disconnected wiring, open-circuit conditions, soiling, degradation, bypass-related behaviour, and electrical faults. A small rooftop or campus installation often measures only voltage and current, if it measures anything at all. Full I–V tracing, thermal inspection, electroluminescence imaging, and commercial SCADA platforms can be costly or impractical for a student-scale deployment.

Manual inspection is reactive and can miss gradual performance decline. A low measured power value alone does not explain *why* power is low: irradiance, temperature, shading, an open circuit, or degradation can all produce different patterns. The key diagnostic need is therefore to compare actual electrical behaviour against expected behaviour under the current environment.

## 3. Proposed solution

Build a low-cost AIoT node that measures panel voltage, current, temperature, and an irradiance proxy. The edge node calculates virtual electrical quantities—such as expected power, performance ratio, power residual, current/voltage residual, and a rough resistance indicator—and combines rules with an ML classifier to identify likely fault conditions.

### One-line aim

> Design an IoT edge system that diagnoses common PV faults using physical electrical sensors plus virtual electrical sensors derived from a simplified digital-twin PV model and machine learning.

### Core operating principle

1. Measure the operating point: voltage `V`, current `I`, temperature `T_panel`, and irradiance proxy `G_proxy`.
2. Calculate measured power: `P_meas = V × I`.
3. Estimate expected power `P_exp` for the same irradiance and temperature.
4. Calculate residuals such as `PR = P_meas/P_exp` and `ΔP = P_exp − P_meas`.
5. Apply immediate safety rules for abnormal electrical states.
6. Run an ML model on a short feature window to classify the probable condition.
7. Display health locally, publish telemetry, log history, and send an alert if required.

## 4. Scope

### In scope

- One small 10–50 W PV panel or low-power panel/string
- ESP32-based sensing and telemetry node
- Physical measurements: voltage, current, panel temperature, irradiance proxy
- Virtual features and a calibrated expected-power model
- Four required classes: Normal, Partial Shading, Open Circuit, Soiling/Degradation Proxy
- Optional controlled fifth class: Short/Line-Fault Risk on a tiny fused test setup
- Edge rules, dashboard, historical logging, and alerting
- ML baseline on public and self-collected data

### Out of scope

- Certification as a grid-tied protection system
- Direct switching/control of a real rooftop solar plant
- Accurate laboratory I–V curve tracing
- Guaranteed diagnosis of every PV failure mode
- High-voltage or high-current experiments
- Claims that an LDR/BH1750 is equivalent to a calibrated pyranometer

## 5. Objectives

1. Build a sensor node for PV voltage, current, temperature, and irradiance proxy.
2. Implement an expected-power virtual sensor model.
3. Derive health features: `P_exp`, `PR`, `ΔP`, residuals, efficiency proxy, and fault score.
4. Collect labelled fault-condition data from controlled experiments.
5. Train and compare lightweight fault classifiers.
6. Deploy rules and, if feasible, compact model inference at the edge.
7. Show live system status, trends, alerts, and estimated energy impact.
8. Demonstrate that virtual features improve diagnosis compared with raw voltage/current alone.

## 6. Physical and virtual sensors

| Layer | Variables | Meaning |
|---|---|---|
| Physical | `V`, `I`, `T_panel`, `G_proxy`, load state | Directly measured hardware data |
| Derived | `P_meas = V×I`, rolling mean/std, gradients | Computed from measurements |
| Virtual electrical | `P_exp`, `PR`, `ΔP`, `ΔV`, `ΔI`, efficiency proxy, `R_s'` | Model-based estimates and residuals |
| Virtual health | class, confidence, health score, energy-loss estimate | Diagnostic output for a user |

### MVP virtual-sensor equations

```text
P_exp = P_stc × (G_proxy / G_ref) × [1 + γ × (T_panel − 25)]
P_meas = V × I
PR = P_meas / max(P_exp, ε)
ΔP = P_exp − P_meas
η_proxy = P_meas / max(G_proxy × panel_area, ε)
R_s' ≈ (V_oc_est − V_operating) / max(I_operating, ε)
```

`R_s'` is a qualitative resistance proxy, not a laboratory-calibrated series-resistance measurement. `ε` prevents division by zero. The irradiance input must be described honestly as a proxy unless measured with calibrated instrumentation.

## 7. Fault taxonomy

| Class | Lab creation method | Expected signature |
|---|---|---|
| Normal | Clean panel, stable light/load | PR near calibrated baseline; stable V/I/P |
| Partial shading | Cover portions of cells with cardboard/mesh | Reduced power; pattern depends on coverage and bypass diodes |
| Open circuit | Safely disconnect panel/load path | Current approaches zero; voltage may be high/floating |
| Soiling/degradation proxy | Dust/film or controlled series resistance | Persistent lower PR/current; slower variation than clouds |
| Short/line-fault risk (optional) | Controlled, fused, low-power resistor/load arrangement only | Low voltage and abnormal current/power behaviour |
| Hotspot proxy (optional) | Local partial shading plus temperature observation | Local temperature anomaly plus power drop |

## 8. Proposed architecture

```text
Small Solar PV Panel (10–50 W)
   ├── Voltage measurement
   ├── Current sensor (INA219 preferred)
   ├── Back-sheet temperature sensor (DS18B20)
   └── Irradiance proxy (BH1750/LDR, calibrated as far as practical)
             │
             ▼
ESP32 Edge Node
   ├── sampling and filtering
   ├── virtual-sensor calculations
   ├── hard safety rules
   ├── optional TinyML / compact classifier
   ├── OLED + RGB LED local state
   └── Wi-Fi telemetry using MQTT / Firebase / HTTP
             │
             ▼
Backend / Dashboard
   ├── live values and virtual-sensor trends
   ├── diagnosis timeline
   ├── alert engine (Telegram/email)
   ├── CSV/database history
   └── estimated energy-loss view
```

## 9. Hardware and software decisions

### Recommended MVP hardware

- 10–30 W polycrystalline PV panel
- ESP32 DevKit
- INA219 current/voltage sensor; use a safe divider/ADC only where INA219 range is unsuitable
- DS18B20 bonded to rear of panel for temperature
- BH1750 or LDR system as an irradiance proxy
- 0.96-inch I2C OLED
- RGB LED/buzzer for status
- Fused resistive load or small USB load
- Inline fuse, switches, insulated connectors, enclosure, mounting stand

### Recommended software

| Layer | Recommended tools |
|---|---|
| Firmware | Arduino C++ or ESP-IDF; INA219/DS18B20/OLED/MQTT libraries |
| Data logging | CSV initially; MQTT + Firebase/InfluxDB/SQLite later |
| ML research | Python, pandas, matplotlib, scikit-learn; XGBoost optional |
| Deployment | Rules first; compact Random Forest/Logistic Regression/TFLite if feasible |
| Backend | Flask/FastAPI or Firebase |
| Dashboard | HTML/JS, Flask templates, React optional; Power BI optional |
| Alerts | Telegram bot or email |
| Version control | Git/GitHub with dataset cards and experiment records |

## 10. Hybrid diagnostic approach

### Stage A: deterministic rules

Rules protect the system and provide explainable immediate output:

- If `G_proxy` is sufficiently high and `I` is approximately zero: probable **Open Circuit**.
- If voltage is abnormally low with unusual current for the test setup: **Short/Line-Fault Risk**; alarm and stop any test procedure.
- If `PR` remains low under stable irradiance for a defined duration: persistent underperformance.
- If sensor values are missing/out of range: **Sensor/Communication Fault**, not a PV diagnosis.

### Stage B: ML classification

Use a 30–60 second feature window with mean, standard deviation, min/max, and trend features. Start with Logistic Regression, Decision Tree, Random Forest, and XGBoost. Select the simplest model that has good class-wise performance and can be explained/deployed.

### Stage C: health score

A simple transparent health score is:

```text
health = 100 × clip(PR, 0, 1) × (1 − α × fault_confidence)
```

The final mapping should be calibrated empirically and presented as a prototype indicator, not a certified maintenance metric.

## 11. Research basis and contribution

The project is motivated by PV fault-diagnosis research that uses electrical parameters, irradiance, temperature, I–V characteristics, thermal information, and ML to separate normal operation from faults. The practical gap is that research setups can rely on expensive sensors or detailed I–V acquisition, while small installations need low-cost retrofit approaches.

### Proposed contribution

- A **physical-to-virtual sensor layer** that estimates expected PV behaviour from inexpensive signals.
- A hybrid of interpretable edge safety rules and ML classification.
- A real, self-collected data set from a low-power panel under controlled fault proxies.
- An ablation study: raw physical features vs physical + virtual features.
- A dashboard that reports both fault class and the reason indicators behind it.

## 12. Evaluation plan

| Metric | Target / purpose |
|---|---|
| Overall accuracy | Target ≥ 90% on held-out own test data if conditions are controlled |
| Per-class precision, recall, F1 | Essential because class imbalance can hide weak fault detection |
| Confusion matrix | Show which classes are confused, especially shade vs soiling |
| Detection latency | Target under 30–60 seconds for sustained events |
| False-alarm count | Test under changing sunlight/cloud conditions |
| Edge inference time | Target below 100 ms for a compact tabular model |
| Ablation result | Compare raw `V,I` with the complete virtual-feature set |
| Cost | Document MVP hardware cost and cost-performance trade-off |

## 13. Risks and mitigation

| Risk | Mitigation |
|---|---|
| LDR/BH1750 does not represent true irradiance | Name it `G_proxy`, calibrate to stable reference conditions, avoid overclaiming |
| Indoor lamp differs from sunlight spectrum | Use outdoor data for final validation whenever possible |
| Domain shift across different panels | Fine-tune thresholds/model on the actual panel |
| Shade and degradation look similar | Use time-window features, stable-light controls, and optional temperature data |
| Unsafe short-circuit tests | Keep panel tiny, use fuses/resistors, use supervised experiments only |
| Sensor drift or noise | Validate against multimeter, filter readings, record calibration data |
| Wi-Fi failure | Keep local OLED/rules operational and buffer telemetry if possible |
| Limited data | Use public/synthetic data only for baseline; clearly label own-data evaluation separately |

## 14. Success definition

The project is successful when:

1. The node reliably logs `V`, `I`, `T_panel`, and `G_proxy`.
2. `P_exp`, PR, and residual features update in real time.
3. At least four fault conditions are safely demonstrated.
4. The dashboard displays live readings, diagnosis history, and alerts.
5. The comparison shows whether virtual features improve classification quality.
6. The prototype remains low-cost, safe, reproducible, and honestly scoped.

## 15. Future extension

- Second healthy reference panel as a physical twin
- PV string-level monitoring for multiple panels
- Thermal camera or low-cost non-contact temperature sensing
- Drone/thermal-image fusion for larger PV sites
- Digital-twin simulation in MATLAB/Simulink
- Seasonal recalibration and online drift detection
- Mobile app for campus maintenance teams
- Federated analytics across multiple sites
