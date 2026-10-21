# Roadmap, Deliverables, Demo, and Documentation Plan

## 1. Development strategy

Build the project in layers. Do not begin with complex ML before sensor readings, calibration, logging, and fault labels are trustworthy.

```text
Safe hardware → reliable logging → virtual features → rules → ML baseline
→ edge/dashboard integration → evaluation → demo/report
```

## 2. Ten-week roadmap

| Week | Main objective | Tasks | Exit criteria / deliverable |
|---:|---|---|---|
| 1 | Define scope and safety | Read literature, choose four required classes, finalise BOM, write safety plan | Approved architecture, purchase list, risk register |
| 2 | Build sensor node | Wire ESP32, V/I, temperature, light proxy; validate using multimeter | Serial log with stable, plausible readings |
| 3 | Logging and normal data | Add timestamped CSV/MQTT logging, build label protocol, collect normal data | First clean labelled data set |
| 4 | Virtual sensors | Implement `P_exp`, PR, ΔP, calibration constants, OLED values | Live physical + virtual values shown locally |
| 5 | Fault experiments | Collect shading, open, and soiling/degradation-proxy data | Labelled multi-class experimental data |
| 6 | ML baselines | EDA, feature engineering, rules, LR/DT/RF training | Baseline notebook and confusion matrix |
| 7 | Edge diagnosis | Put rules and compact selected model on ESP32/gateway; add LED/buzzer | Local diagnosis works without dashboard |
| 8 | Dashboard and alerting | MQTT/Firebase/Flask, live page, history, Telegram/email alert | End-to-end telemetry and alert path |
| 9 | Evaluation | Ablation, stress tests, latency, false-alarm tests, cost/limitations | Metrics table, plots, final results |
| 10 | Final packaging | Report, poster, slides, demo video, GitHub cleanup, viva prep | Submission-ready project pack |

## 3. Minimum viable product

The MVP is complete when all items below work:

- [ ] ESP32 reads voltage, current, panel temperature, and light proxy.
- [ ] Readings are logged with timestamps and labels.
- [ ] `P_meas`, `P_exp`, PR, and ΔP are calculated.
- [ ] Normal, partial shading, open-circuit, and soiling/degradation-proxy modes are identified.
- [ ] OLED/RGB LED shows status locally.
- [ ] A dashboard shows live and historical values.
- [ ] At least one alert is sent for a sustained critical condition.
- [ ] A report compares raw vs virtual-feature diagnosis.

## 4. Milestone reviews

### M1 — Sensor credibility review (end of Week 2)

Required evidence:

- wiring photograph or diagram,
- multimeter comparison table,
- sensor scan output,
- sample serial log,
- explicit safe operating range.

### M2 — Data credibility review (end of Week 5)

Required evidence:

- dataset schema,
- label definitions,
- per-class data counts,
- experiment photos/notes,
- examples of raw time-series plots.

### M3 — Diagnosis credibility review (end of Week 7)

Required evidence:

- rule flow chart,
- model comparison table,
- confusion matrix,
- selected-feature list,
- edge inference or gateway inference proof.

### M4 — Project credibility review (end of Week 10)

Required evidence:

- working demo,
- dashboard screenshots,
- ablation result,
- cost table,
- limitations section,
- cleaned repository and documentation.

## 5. Deliverable checklist

### Code

- [ ] ESP32 firmware
- [ ] Sensor-test sketch
- [ ] Virtual-feature module
- [ ] Telemetry publisher
- [ ] Rule engine
- [ ] ML training notebook/script
- [ ] Model export/deployment artifact
- [ ] Dashboard/backend source

### Data

- [ ] Raw CSV data, retained read-only
- [ ] Cleaned/model-ready data
- [ ] Data dictionary
- [ ] Experiment metadata
- [ ] Label protocol
- [ ] Train/validation/test split record

### Documentation

- [ ] Master context document
- [ ] Architecture document
- [ ] Hardware and safety guide
- [ ] Data/ML/research plan
- [ ] README with setup instructions
- [ ] Literature matrix
- [ ] Final report and references

### Presentation

- [ ] Poster or one-page project summary
- [ ] 8–12 slide presentation
- [ ] 3–4 minute demo script
- [ ] Viva Q&A sheet
- [ ] Backup recorded demo video

## 6. Demo plan: 3–4 minutes

1. **Introduce the problem:** PV systems can underperform, and raw power alone does not reveal the fault reason.
2. **Show normal operation:** full available light; dashboard/OLED shows Normal and healthy PR.
3. **Demonstrate partial shading:** cover a defined portion of the panel; expected power remains relatively high while measured power and PR fall; system reports Partial Shading.
4. **Demonstrate open circuit:** use safe switch disconnect; current approaches zero; red alert and Open Circuit state appear.
5. **Demonstrate soiling/degradation proxy:** apply approved proxy or series-resistance test; show persistent PR reduction and diagnostic warning.
6. **Show dashboard history:** expected vs measured power, fault timeline, health score, and energy-loss estimate.
7. **Prove edge capability:** temporarily disconnect Wi-Fi; local OLED/rules still identify condition.
8. **Conclude with novelty:** low-cost physical sensors plus virtual electrical sensors and edge intelligence.

## 7. Report structure

1. Title, abstract, and keywords
2. Introduction and motivation
3. Problem statement and objectives
4. Literature review
5. Proposed system and architecture
6. Hardware design and safety measures
7. Virtual electrical sensor methodology
8. Dataset collection and labelling
9. ML/rule-based diagnosis design
10. Results and evaluation
11. Dashboard and edge deployment
12. Cost, limitations, ethics, and sustainability
13. Conclusion and future work
14. References and appendices

## 8. Recommended result figures

- System block diagram
- Photograph of prototype
- Sensor wiring diagram
- Example raw time-series plot: V, I, P, G_proxy, temperature
- Expected vs measured power plot
- PR trend during normal and fault condition
- Confusion matrix
- Model comparison bar chart
- Ablation table: raw inputs vs virtual features
- Dashboard screenshots
- Cost table

## 9. Viva preparation prompts

| Likely question | Strong concise answer |
|---|---|
| What is the novelty? | We do not rely only on raw V/I thresholds; we create virtual electrical sensors from a simple expected-power model and use the residual patterns for diagnosis. |
| Why not use only a thermal camera? | Thermal inspection can be valuable but raises cost and is not continuously available in a low-cost retrofit. Our system uses inexpensive electrical/environmental sensing. |
| What is a virtual sensor? | It is an estimated quantity produced using physical measurements and a model/algorithm instead of a dedicated hardware sensor. |
| Why use ML with rules? | Rules provide fast, explainable safety handling; ML helps separate overlapping performance patterns such as shading and soiling proxy. |
| Is BH1750 actual irradiance? | No. It is used as a calibrated light/irradiance proxy, and that limitation is explicitly documented. |
| How do you validate accuracy? | With held-out data by experiment session, per-class F1/confusion matrix, latency, false alarms, and comparison of raw vs virtual-feature models. |
| Can it be used on a real plant? | It is a prototype diagnostic assistant. Real deployment needs calibrated sensors, robust enclosures, electrical compliance, and further validation. |

## 10. Definition of done

The final submission is ready only if:

- hardware works repeatedly and safely,
- data and labels are traceable,
- virtual features are visible and explained,
- fault outputs have evidence rather than screenshots only,
- limitations are stated honestly,
- the dashboard and local edge state both work,
- every reported metric can be reproduced from saved code and data.
