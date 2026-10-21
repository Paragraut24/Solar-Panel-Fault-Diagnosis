# Solar PV Fault Diagnosis — Getting Started Guide

> **Status:** ESP32 + Firebase Realtime Database working. Starting Week 3 of 10.

---

## Where You Are Right Now

```
[DONE] Week 1  — Scope, BOM, safety plan
[DONE] Week 2  — ESP32 + sensors wired + Firebase streaming
[HERE] Week 3  — Data logger, labels, normal data collection
        Week 4  — Virtual sensor engine (P_exp, PR, ΔP)
        Week 5  — Fault experiments
        Week 6  — ML baselines
        Week 7  — Edge diagnosis
        Week 8  — Dashboard + alerts
        ...
```

---

## Immediate Next Steps (Do These First)

### Step 1 — Set Up the Project Repository

Your workspace at `e:\SEM 5\AIOT\CP\Solar-Panel-Fault-Diagnosis` is basically empty. Set it up properly now.

**Folder structure to create:**

```
Solar-Panel-Fault-Diagnosis/
├── firmware/              ← ESP32 Arduino sketches
│   ├── sensor_test/       ← quick I2C/ADC sanity sketch
│   └── main_node/         ← the real firmware
├── data/
│   ├── raw/               ← CSV files, never edit these
│   ├── processed/         ← cleaned, feature-engineered CSVs
│   └── metadata/          ← YAML experiment logs
├── notebooks/             ← Jupyter EDA + ML training
├── dashboard/             ← Web app (Flask or React)
├── docs/                  ← Copy your context docs here
│   └── context/
└── README.md
```

---

### Step 2 — Verify What Your ESP32 is Currently Sending to Firebase

Before logging anything, confirm the fields being pushed. The telemetry packet **must** include these columns:

```
timestamp, voltage_v, current_a, power_w, panel_temp_c, g_proxy, load_state
```

If your current Firebase data looks different (e.g., missing temp or irradiance), fix the firmware first.

> [!IMPORTANT]
> Do NOT start data collection until all 4 physical channels (V, I, T_panel, G_proxy) are confirmed with plausible values and validated against a multimeter.

**Validate each sensor:**

| Channel | Expected range | How to verify |
|---|---|---|
| `voltage_v` | 0–25 V | Compare with multimeter across panel terminals |
| `current_a` | 0–2 A (for 10–30 W panel) | Compare with clamp meter or known resistive load |
| `panel_temp_c` | 25–70 °C | Compare with IR thermometer or reference thermometer |
| `g_proxy` | 0–65535 lux (BH1750) | Varies with light; note "full sun" baseline |

---

### Step 3 — Add a Data Logger to Your Firmware

Right now Firebase only shows live data. You need **timestamped, labeled CSV logs** for ML training.

Add these two things to your firmware immediately:

#### 3a. Add a `label` field to your Firebase push

Change your Firebase write to include a `label` field that you set manually when collecting fault data:

```cpp
// In your Firebase push
FirebaseJson json;
json.set("timestamp", getTimestamp());
json.set("voltage_v", voltage);
json.set("current_a", current);
json.set("power_w", voltage * current);
json.set("panel_temp_c", temperature);
json.set("g_proxy", irradiance);
json.set("load_state", "resistive");
json.set("label", "normal");          // ← Change this per experiment
json.set("experiment_id", "EXP-001"); // ← Change per session
```

#### 3b. Write raw CSV simultaneously (optional but recommended)

Log to an SD card or serial port so you have a local backup independent of Firebase:

```csv
timestamp,experiment_id,label,coverage_pct,load_state,voltage_v,current_a,power_w,panel_temp_c,g_proxy
```

---

### Step 4 — Start Collecting "Normal" Data Immediately

**Target: 1,000–3,000 rows at 2–5 second intervals = about 1–2 hours of logging**

**How:**
1. Clean panel, full available light (lamp indoors or direct sun)
2. Set `label = "normal"`, `experiment_id = "EXP-001"`
3. Run for 30–60 minutes in stable conditions
4. Run for another 30 minutes with naturally varying light (clouds, lamp dimming)
5. Save the Firebase export as `data/raw/EXP-001_normal.csv`

> [!TIP]
> You can export Firebase Realtime Database data as JSON from the console. Write a small Python script to convert it to CSV right after each session.

---

## What to Build This Week

### Dashboard Phase 1 — Raw Data Viewer (Week 3 task)

Since Firebase is already working, build a minimal web page this week that:

1. **Reads live Firebase data** and shows V, I, P, T, G_proxy as numbers
2. **Plots a 5-minute rolling chart** of power and irradiance
3. **Shows a raw data table** with timestamps

This is your Week 3 deliverable. You do NOT need ML or virtual sensors yet.

**Tech stack decision (choose one):**

| Option | Effort | Recommended |
|---|---|---|
| Plain HTML + Firebase JS SDK | 2–3 hours | ✅ Start here |
| Flask + Firebase Admin SDK | 4–6 hours | Good if you want a Python backend |
| React + Firebase | 1–2 days | Overkill for now |

---

## Virtual Sensors — Implement in Week 4

Once normal data is collected and displayed, add the virtual sensor engine. These are the core equations:

```python
# Constants (calibrate on your panel)
P_STC = 20       # your panel's rated watt at STC
G_REF = 1000     # W/m² reference
GAMMA = -0.004   # temperature coefficient (~-0.4%/°C for poly panels)
EPSILON = 1e-6   # prevent divide-by-zero

# Virtual sensor calculations
P_meas = voltage_v * current_a
P_exp = P_STC * (g_proxy / G_REF) * (1 + GAMMA * (panel_temp_c - 25))
PR = P_meas / max(P_exp, EPSILON)
delta_P = P_exp - P_meas
delta_P_pct = 100 * delta_P / max(P_exp, EPSILON)
eta_proxy = P_meas / max(g_proxy * panel_area_m2, EPSILON)
```

> [!NOTE]
> `g_proxy` from a BH1750 gives lux, not W/m². You need a calibration factor. On a clear day, note what your BH1750 reads vs. a known reference (e.g., assume 1000 W/m² at peak sun) and derive a `lux_to_irradiance` multiplier. Call it `G_proxy` and document this limitation.

---

## Fault Classes — What to Do in Week 5

| Fault | How to create it | Safety note |
|---|---|---|
| **Normal** | Clean panel, stable light | Already doing this |
| **Partial Shading** | Cover 25%, 50%, 75% of panel with cardboard | Safe |
| **Open Circuit** | Disconnect load safely using a switch | Safe |
| **Soiling/Degradation** | Apply dust/film OR add a series resistor (e.g., 5–10 Ω) in the load path | Safe |
| **Short (optional)** | Only on tiny fused test panel with resistive load — NOT hard short | Use fuse |

Label each session in your firmware before starting.

---

## Firebase Structure Recommendation

Organize your Realtime Database like this:

```
solar-pv-monitor/
├── telemetry/
│   └── {push_id}/
│       ├── timestamp
│       ├── voltage_v
│       ├── current_a
│       ├── power_w
│       ├── panel_temp_c
│       ├── g_proxy
│       ├── label
│       └── experiment_id
├── events/
│   └── {push_id}/
│       ├── type: "FAULT_TRANSITION"
│       ├── from: "NORMAL"
│       ├── to: "OPEN_CIRCUIT"
│       └── reason: "..."
└── device_status/
    └── pv-node-01/
        ├── last_seen
        ├── firmware_version
        └── wifi_rssi
```

> [!CAUTION]
> Firebase Realtime Database stores every pushed row forever under `telemetry/`. Set a **data retention rule** or periodically export and trim to avoid hitting the free tier storage limit during long data collection sessions.

---

## This Week's Checklist

- [ ] Create the folder structure above in your project repo
- [ ] Verify all 4 sensor channels with multimeter (document in `docs/`)
- [ ] Add `label` and `experiment_id` fields to your Firebase push
- [ ] Collect 1,000+ rows of Normal data and export to `data/raw/`
- [ ] Write a Python script `notebooks/firebase_to_csv.py` to download and convert Firebase JSON to CSV
- [ ] Build a minimal HTML dashboard that reads live Firebase data and shows the raw numbers + one chart
- [ ] Commit everything to GitHub with a proper initial commit

---

## Files to Create/Edit Immediately

| File | Purpose |
|---|---|
| `firmware/main_node/main_node.ino` | Add `label` + `experiment_id` fields |
| `data/raw/.gitkeep` | Placeholder so folder is tracked |
| `notebooks/firebase_to_csv.py` | Export Firebase → CSV |
| `dashboard/index.html` | Minimal live dashboard |
| `docs/context/` | Copy all your context docs here |
| `README.md` | Fill this in properly |

---

## Do NOT Do Yet

- ❌ ML model training (no labeled data yet)
- ❌ Virtual sensor calculations on the firmware (Week 4)
- ❌ Telegram alerts (Week 8)
- ❌ Edge TFLite inference (Week 7)

---

## Summary: Priority Order

```
1. Verify all sensors → multimeter validation
2. Add label/experiment_id to firmware → Firebase
3. Log 1,000+ rows of normal data → export CSV
4. Build minimal live dashboard → shows raw Firebase data
5. Structure the GitHub repo properly
```
