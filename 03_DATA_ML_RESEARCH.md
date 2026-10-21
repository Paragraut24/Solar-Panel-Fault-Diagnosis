# Data, Machine Learning, and Research Plan

## 1. Research question

Can low-cost physical sensing plus virtual electrical features improve identification of common solar PV fault conditions compared with raw voltage and current alone?

## 2. Research rationale

PV-fault research commonly uses electrical quantities, irradiance, temperature, thermal images, I–V curves, and ML algorithms to identify shading, open-circuit, short-circuit/line faults, degradation, and other anomalies. Detailed diagnostic instruments can be expensive, while small installations may only have basic electrical data.

This project investigates a practical student-scale alternative: use a simple PV performance model to create **virtual electrical sensors** from cheap measurements. The residual between expected and observed power becomes a meaningful diagnostic signal.

## 3. Research contribution statement

> The prototype evaluates whether model-derived virtual features—expected power, performance ratio, residuals, efficiency proxy, and temporal trends—add diagnostic value to low-cost physical measurements in an edge-deployable PV monitoring system.

## 4. Data strategy

### Layer 1: public/synthetic data

Use public or simulated PV-fault data only to:

- understand common labels and electrical signatures,
- prototype feature engineering,
- train/test baseline algorithms offline,
- compare methods described in related work.

Do not claim public-data results as proof that the physical prototype performs equally well.

### Layer 2: self-collected experimental data

This is mandatory for prototype claims. Log data from the same panel, sensor arrangement, load, and controlled conditions used in the demonstration.

### Layer 3: hybrid method

1. Explore baseline algorithms using public or synthetic data.
2. Collect labelled local data.
3. Calibrate rules and retrain/fine-tune using local data.
4. Report public-data and local-prototype results separately.

## 5. Raw data schema

### Required CSV columns

```csv
timestamp,experiment_id,label,coverage_pct,load_state,voltage_v,current_a,power_w,panel_temp_c,g_proxy,ambient_note,network_state
```

### Derived-feature columns

```csv
p_expected_w,pr,delta_p_w,delta_p_pct,v_expected_v,i_expected_a,delta_v,delta_i,eta_proxy,rs_proxy,dp_dt,pr_mean_30s,pr_std_30s,model_prediction,confidence
```

### Recommended metadata file per experiment

```yaml
experiment_id: EXP-2026-001
panel_rating_w: 20
panel_area_m2: 0.14
sensor_firmware_version: 0.1.0
location: campus_rooftop_or_lab
lighting: direct_sun / lamp
fault_label: partial_shading
fault_procedure: 50 percent cell area covered using cardboard
operator: initials
notes: intermittent cloud at minute 12
```

## 6. Label taxonomy

| Numeric label | Class | Definition |
|---:|---|---|
| 0 | Normal | Stable panel with no intentional fault proxy |
| 1 | Partial_Shading | Deliberate partial cell coverage, with percentage/pattern recorded |
| 2 | Open_Circuit | Safe disconnect of load/output path |
| 3 | Soiling_Degradation_Proxy | Controlled film/dust/series-resistance condition; document exact proxy |
| 4 | Short_Line_Risk_Proxy | Only controlled, fused, low-power test condition |
| 5 | Unknown | Conditions unsuitable for labelling or conflicting observations |
| 6 | Sensor_Fault | Missing, disconnected, impossible, or uncalibrated sensor values |

Never label transient cloud passage as degradation. If the light condition changes rapidly, either record it as normal environmental variation or mark it uncertain depending on the experiment objective.

## 7. Recommended data collection targets

| Class | Initial target | Collection guidance |
|---|---:|---|
| Normal | 1,000–3,000 rows | Multiple times of day; stable and variable natural conditions |
| Partial shading | 1,000–2,000 rows | 25/50/75% cover and several cover patterns |
| Open circuit | 500–1,000 rows | Safe switch-disconnect events and stable light |
| Soiling/degradation proxy | 1,000+ rows | Multiple levels; document exactly how proxy is created |
| Short/line-risk proxy | 300–800 rows | Optional; supervised and low-power only |

Sampling at 1–5 seconds is enough for this MVP. Do not create false independence: many consecutive rows from one static condition are correlated. Split train/test data by experiment session where possible.

## 8. Feature engineering

### Physical features

- `voltage_v`, `current_a`, `power_w`
- `panel_temp_c`, `g_proxy`
- load state
- elapsed time / experiment context only when justified

### Virtual features

- `p_expected_w`
- `pr`
- `delta_p_w`, `delta_p_pct`
- `v_expected_v`, `i_expected_a`, `delta_v`, `delta_i`
- `eta_proxy`
- `rs_proxy`
- relative temperature difference if ambient temperature is available

### Temporal features

- rolling mean and standard deviation for 30–60 seconds
- `dP/dt`, `dPR/dt`
- proportion of low-PR samples in last window
- irradiance stability score
- persistence duration of a rule condition

## 9. Baseline models

| Model | Why include it | Deployment suitability |
|---|---|---|
| Rule thresholds | Essential explainable fallback | Excellent |
| Logistic Regression | Simple, fast baseline | Excellent |
| Decision Tree | Interpretable nonlinear baseline | Good |
| Random Forest | Strong tabular baseline | Good if compact/converted; excellent on gateway/cloud |
| XGBoost | High tabular performance benchmark | Usually gateway/cloud for MVP |
| Small MLP/TFLite | Stretch option for edge deployment | Possible on Pi/ESP32-S3 with careful sizing |
| 1D CNN | Stretch research model for time windows | Not required for MVP |

### Recommended selection process

1. Start with thresholds and Logistic Regression.
2. Add Decision Tree and Random Forest.
3. Select using macro-F1, confusion matrix, latency, and explainability—not accuracy alone.
4. Deploy the simplest acceptable model.

## 10. Training protocol

1. Validate units, sensor ranges, timestamps, labels, and missing values.
2. Exclude `Unknown` and `Sensor_Fault` from the main PV classifier; handle them with rules.
3. Split train/validation/test by experiment session, not random adjacent rows only.
4. Fit scalers only on training data when using scale-sensitive models.
5. Use class weights or balanced sampling if labels are imbalanced.
6. Tune thresholds on validation data only.
7. Freeze a final test set before reporting final metrics.
8. Save model, feature order, preprocessing parameters, code commit hash, and dataset version.

## 11. Required experiments

### Experiment A: raw sensors vs virtual sensors

| Setup | Features |
|---|---|
| Baseline | `V`, `I`, `P`, `T_panel`, `G_proxy` |
| Proposed | Baseline + `P_exp`, PR, ΔP, residuals, temporal features |

Compare macro-F1, per-class F1, and confusion matrices. This is the central evidence for the project’s virtual-sensor claim.

### Experiment B: stable vs changing light

Measure false alarms during normal operation under clear conditions, moving clouds, and artificial shade. Explain that cloud transients can resemble shading and require time-window logic.

### Experiment C: local generalisation

Train/calibrate on earlier sessions and test on a later session. This gives a more realistic estimate than random row splitting.

### Experiment D: edge latency

Measure sampling time, feature computation time, inference time, and alert latency. Document any network contribution separately.

## 12. Metrics and reporting

| Metric | Why it matters |
|---|---|
| Accuracy | Overall summary only |
| Macro precision/recall/F1 | Treats rare classes fairly |
| Per-class F1 | Shows whether critical classes are actually detected |
| Confusion matrix | Reveals shade vs soiling or normal vs cloud confusion |
| Detection latency | Measures practical usefulness |
| False-alarm rate | Necessary for user trust |
| Data completeness | Shows system reliability |
| Ablation delta | Demonstrates value of virtual features |

## 13. Honest limitations

- A light proxy does not equal calibrated irradiance.
- A small-panel test may not generalise directly to a large PV plant.
- Soiling/degradation proxies are not identical to years of real degradation.
- A fixed load does not provide a complete I–V curve.
- Partial shade behaviour depends on cell layout, bypass diodes, and light geometry.
- Public data may use different sensors, panel types, weather, and label definitions.
- Results should be presented as prototype evidence, not certification-grade diagnostics.

## 14. Research references to preserve

The project concept file already identified research directions and sources involving PV fault datasets, ML diagnosis, electrical/thermal PV-fault analysis, synthetic I–V data, and multi-class fault studies. Retain a literature spreadsheet with fields:

```text
citation | year | fault classes | sensor inputs | dataset type | model | metrics | limitations | relevance to our design
```

For the final report, prioritise peer-reviewed papers and clearly distinguish them from dataset pages, tutorials, and manufacturer documentation.
