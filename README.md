# Solar PV Fault Diagnosis with Virtual Electrical Sensors

## Documentation Pack

This folder contains the working context and implementation documents for the AIOT project:

> **Virtual Electrical Sensor Framework for Edge-AI Solar PV Fault Diagnosis**

The project is a low-cost AIoT prototype that monitors a small solar photovoltaic (PV) panel, derives virtual electrical sensors from measured data, detects faults with rules plus machine learning, and presents health information on an edge display and web dashboard.

## Document guide

| File | Purpose |
|---|---|
| `00_MASTER_CONTEXT.md` | Single source of truth: problem, solution, scope, architecture, research, risks, and success definition |
| `01_SYSTEM_ARCHITECTURE.md` | Component-level architecture, data flow, interfaces, dashboard and alert design |
| `02_HARDWARE_AND_SAFETY.md` | Bill of materials, connection guidance, test setup, safety controls, and validation checklist |
| `03_DATA_ML_RESEARCH.md` | Research basis, data collection, labels, virtual features, models, metrics, and experiments |
| `04_ROADMAP_AND_DELIVERABLES.md` | 10-week execution plan, milestones, deliverables, demo script, report and viva preparation |

## Project at a glance

**Inputs:** panel voltage, current, panel temperature, irradiance proxy, time, load state.  
**Virtual features:** expected power, performance ratio, power residual, estimated efficiency, voltage/current residuals, resistance proxy, health score.  
**Outputs:** Normal / Partial Shading / Open Circuit / Short or Line-Fault Risk / Soiling–Degradation; confidence, alert, energy-loss estimate.  
**Target hardware:** 10–50 W panel + ESP32 + INA219 + temperature sensor + irradiance proxy sensor.  
**Target cost:** approximately ₹2,500–5,000 for the ESP32 MVP.

## Important safety statement

This is an academic prototype, not a certified solar-plant protection or SCADA system. Use only a small 10–50 W panel, a fuse, an appropriate resistive load, insulated connectors, and supervised testing. Do **not** hard-short a large panel or a live rooftop array. Simulate a line-fault/short condition with a controlled low-power load arrangement whenever possible.

## Naming convention

- `P_meas`: measured electrical power = `V × I`
- `P_exp`: expected power from the virtual PV model
- `PR`: performance ratio = `P_meas / P_exp`
- `ΔP`: power residual = `P_exp − P_meas`
- `G_proxy`: irradiance proxy, not a laboratory-grade irradiance measurement

## Source basis

The documentation synthesises the project concept and implementation plan already developed in the project repository, including the solar virtual-sensor concept document, the build roadmap, and the AIOT syllabus.
