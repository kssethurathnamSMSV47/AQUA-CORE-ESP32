# AQUA-CORE-ESP32
### Automated Fluid Control, Monitoring & Evaluation System

**Team Name:** Nova Thinkers  
**Team ID:** `LRC-26_0050`

**Competition:** Lam Research Challenge 3.0  
**Round:** Hardware Hustle — Stage 2

---

## 📌 Project Overview

**AQUA-CORE** is an ESP32-based fluid transfer, monitoring, and control system developed for the Lam Research Challenge 3.0 — Hardware Hustle Round (Stage 2).

The project demonstrates coordinated fluid handling through multiple pumps, a solenoid valve, a manually operated ball valve, a float switch, and a flow sensor. The ESP32 acts as the central controller, coordinating the system's operating states, monitoring fluid flow, responding to level-related events, and supporting sequential fluid-transfer and flushing experiments.

A Python-based data acquisition and analysis workflow interfaces with the ESP32 over serial communication. It captures experimental telemetry, organizes test runs, and generates plots and reports to support performance evaluation and experimental verification.

This repository serves as the central project archive for the firmware, software tools, experimental graphs, reports, and demonstration videos produced by **Team Nova Thinkers**.

## 🎯 Project Objectives

- Develop an embedded fluid-transfer system using the ESP32 microcontroller.
- Coordinate pump operation and valve states during fluid-transfer experiments.
- Monitor fluid flow using the FT-01 flow sensor and respond to float-switch events.
- Demonstrate the process-transfer and flushing operating sequences.
- Characterize flow performance at different manually selected ball-valve positions.
- Acquire and record experimental data through Python serial communication.
- Generate graphs, calculate performance metrics, and document experimental observations.
- Maintain an organized and reproducible record of code, test runs, and project demonstrations.

## ⚙️ System Architecture

The system integrates the following principal components:

| Component | Function |
|---|---|
| ESP32 | Main controller for system logic, pump control, sensor acquisition, and serial communication |
| Pump-01 | Process-side fluid transfer |
| Pump-02 | Fluid circulation/return flow path |
| Pump-03 | Additional pumping function during the flushing experiment |
| ON/OFF solenoid valve | Controls the designated process-fluid path |
| Manual ball valve | Allows manual adjustment of the designated fluid path |
| Float switch | Provides a discrete fluid-level status input |
| FT-01 flow sensor | Measures flow through pulse-based sensing |
| Process tank | Holds the process fluid |
| Sump tank | Receives fluid from the process-side flow path |
| Flush tank | Supplies fluid for the flushing experiment |
| Python application | Receives serial telemetry, logs data, and generates analysis outputs |

The precise hydraulic connections and control behavior should be interpreted from the project's actual implementation and experimental documentation.

## 🔄 Experimental Operating Cases

### Case 1 — Fluid Transfer and Level-Responsive Operation

Case 1 evaluates fluid transfer between the process tank, sump tank, and return flow path. The ESP32 coordinates the relevant pumps and solenoid valve while monitoring the float switch and FT-01 flow sensor.

The experiment is intended to demonstrate coordinated pump operation, fluid-level response, flow monitoring, and continuous data acquisition. Recorded measurements can be used to examine flow-rate variation and cumulative sensor-derived volume over time.

### Case 2 — Fluid Flushing Experiment

Case 2 evaluates the flushing operation using the designated flush-tank flow path. The process-fluid path is isolated through the solenoid valve, and the flush-fluid path is operated according to the experiment's procedure.

The experiment examines the flow response during flushing and the behavior of the system as the available flush fluid decreases. The manual ball valve is operated according to the documented test procedure.

### Ball-Valve Flow Characterization

A separate experiment investigates the relationship between manually selected ball-valve positions and measured flow rate. Measurements are collected at the tested valve positions, allowing comparison of the observed flow response.

**Note:** Ball-valve position percentages represent manual valve settings, not pump PWM duty cycles.

## 🧰 Hardware and Software

### Hardware

- ESP32 development board
- Three DC pumps and their associated driver circuitry
- Solenoid valve
- Manually operated ball valve
- Float switch
- FT-01 flow sensor
- Process, sump, and flush tanks
- Tubing, hydraulic fittings, power supply, and interconnecting wiring

### Software and Tools

- Arduino IDE for ESP32 firmware development
- Python 3 for serial data acquisition and analysis
- PySerial for serial communication
- Data-processing and plotting libraries used by the supplied scripts
- GitHub for version control and project documentation

Refer to the source files and project documentation for the actual pin assignments, calibration coefficients, serial protocol, library requirements, and setup instructions.

## 📁 Repository Contents

The repository is organized into the following main resource categories.

### 1. ESP32 Firmware

**Location:** [`Codes/ESP32/`](Codes/ESP32/)

Contains the Arduino `.ino` source files for the ESP32-based fluid-transfer and experimental control system.

The firmware resources are intended to cover:

- Pump and valve control logic
- Float-switch input handling
- FT-01 pulse measurement and flow-rate estimation
- Case 1 and Case 2 operating logic
- Ball-valve flow-characterization experiments, where provided
- Serial telemetry for Python-side data acquisition

Open the folder to view the actual firmware files and their individual descriptions.

### 2. Python Scripts

**Location:** [`Python_Scripts/`](Codes/Python_Scripts/)

Contains the Python tools used for serial communication, experimental logging, data processing, plotting, and report generation, where included.

The relevant scripts support tasks such as:

- Reading telemetry transmitted by the ESP32
- Recording timestamped experimental samples
- Separating experimental runs
- Processing flow-rate and frequency data
- Generating flow-rate-versus-time and cumulative-volume plots
- Summarizing ball-valve characterization results
- Exporting analysis outputs for reporting

**Important:** Python command strings, serial-port settings, baud rate, CSV field names, and expected ESP32 output must match the corresponding firmware. Refer to each script's instructions before running it.

### 3. Experimental Graphs

**Location:** [`Graphs/`](Graphs/)

Contains the plots generated from the experimental data.

Depending on the files included, the graphs can be used to examine:

- Case 1 flow rate versus time
- Case 1 cumulative sensor-derived volume
- Case 2 flow rate versus time
- Case 2 cumulative sensor-derived volume
- FT-01 flow rate versus pulse frequency
- Ball-valve position versus measured flow rate
- Other experimental comparisons and performance trends

The graphs provide a visual record of measured behavior and support comparison between experimental cases.

### 4. Documentation and Reports

**Location:** [`Documentation/`](docs/)

Contains the project report and supporting technical documents, where available.

These resources may document:

- System architecture and operating principles
- Experimental procedures
- Firmware and Python software workflow
- Calibration methodology and calculations
- Case 1 and Case 2 results
- Ball-valve flow-characterization results
- Graphs, observations, limitations, and conclusions

Consult the supplied reports for the specific experimental values and procedures. Sensor-derived measurements should not be interpreted as independent reference measurements unless a separate reference instrument or measurement method is documented.

### 5. Experiment Videos

**Location:** [`experiment_video/`](experiment_video/)

Contains the project demonstration videos organized into two subfolders.

- **Complete experiment explanation video:** [`complete_experiment_explaination_video/`](experiment_video/complete_experiment_explaination_video/) — intended to present the system, its operation, and the experimental workflow (google drive link provided).
- **Project journey video:** [`journey video/`](experiment_video/journey%20video/) — intended to document the project's development journey and team progress (google drive link provided).

Open each folder to view the video files available in the repository.

### 6. Google Drive Resources

**External project resources:** [Open the AQUA-CORE Google Drive folder]((https://drive.google.com/drive/folders/1EarbnH14BNHFJYKKAfEIHYIXhIpl5q3U?usp=sharing))

Use this section for supplementary files hosted on Google Drive, such as large videos, additional evidence, or supporting documents that are not stored directly in this repository.

Replace the placeholder URL with the actual share link before publishing. Confirm that the sharing permissions allow the intended reviewers to access the files.

## 📊 Experimental Data and Performance Evaluation

The project uses FT-01 pulse measurements to estimate flow rate and calculate cumulative volume. The firmware and Python tools support analysis of the recorded flow behavior across experimental periods.

The documented calibration model is:

\[
Q = 0.02420192f + 0.02599480
\]

where:

- \(Q\) is the estimated flow rate in L/min.
- \(f\) is the measured pulse frequency in Hz.

Use this calibration equation only for data processed with the corresponding calibration configuration. If the firmware uses different coefficients, update the equation in the documentation to match the version used for that experiment.

The experimental analysis can include:

- Flow-rate stability and variation
- Pulse frequency versus estimated flow rate
- Cumulative sensor-derived volume
- Differences between operating cases
- Flow response at different ball-valve positions
- Observations during float-switch events and flushing

**Measurement limitation:** Flow rate and cumulative volume calculated from the same flow sensor are derived measurements, not independent validation of one another. Independent accuracy validation requires comparison with a suitable reference measurement.

## 💻 Getting Started

### Step 1 — Obtain the project files

Clone the repository:

```bash
git clone https://github.com/kssethurathnamSMSV47/AQUA-CORE-ESP32.git
cd AQUA-CORE-ESP32
```

Alternatively, download the repository as a ZIP file from GitHub.

### Step 2 — Open the ESP32 firmware

1. Navigate to `Codes/ESP32/`.
2. Open the required `.ino` file in the Arduino IDE.
3. Confirm the ESP32 board selection and required libraries.
4. Verify the GPIO assignments and hardware connections against the source code and circuit documentation.
5. Check the pump-driver and valve-control electrical requirements before applying power.
6. Compile the firmware and upload it to the ESP32.

### Step 3 — Configure Python

1. Install a compatible Python 3 version.
2. Navigate to the Python scripts directory.
3. Install the dependencies required by the selected script.
4. Identify the ESP32 serial port.
5. Set the correct port and baud rate.
6. Ensure the script's expected telemetry format matches the firmware.

For example, if the script requires PySerial:

```bash
python -m pip install pyserial
```

Install other dependencies only as required by the individual script.

### Step 4 — Run the experiment

1. Check the tank levels, tubing, fittings, valve positions, and electrical connections.
2. Confirm that the firmware is running and the serial connection is established.
3. Start the appropriate Python logger if required by the experiment.
4. Follow the documented Case 1, Case 2, or ball-valve test procedure.
5. Monitor the serial output and confirm that measurements are being recorded.
6. Save the generated logs, graphs, and reports for subsequent analysis.

**Safety:** Keep liquids away from exposed electronics, use suitable pump drivers and power supplies, and verify that pumps cannot run dry or operate against an unintended closed flow path. Perform tests under appropriate supervision.

## 🔌 ESP32–Python Communication

The ESP32 and Python applications communicate over a serial interface. The microcontroller performs hardware-level sensing and control, while Python receives the transmitted measurements and organizes them for analysis.

A typical data workflow is:

1. The FT-01 sensor produces pulses corresponding to fluid flow.
2. The ESP32 measures pulse frequency and calculates an estimated flow rate using its configured calibration.
3. The ESP32 transmits measurement records over serial communication.
4. Python parses the records and saves experimental samples.
5. Analysis scripts generate plots and summaries from the collected data.

Correct operation depends on matching the firmware's serial format with the parser's expected format. A successful serial connection alone does not guarantee that every field is being interpreted correctly.

## 🎥 Demonstrations and Verification

The repository includes resources for examining both the technical implementation and the experimental process.

- Review the firmware to understand the implemented control logic.
- Review the Python scripts to understand the data acquisition and processing workflow.
- Inspect the graphs and reports to evaluate recorded experimental behavior.
- Watch the complete experiment explanation video to understand the demonstrated setup and operation.
- Watch the project journey video to follow the development process.

Multiple experimental runs can help identify inconsistent behavior, check repeatability, and compare results. The recorded data should be interpreted with attention to calibration, sensor limitations, test conditions, and any differences between firmware versions.

## 🔐 License and Intellectual Property

**License:** Proprietary — All Rights Reserved, subject to the terms of the repository's [`LICENSE`](LICENSE) file.

Unless expressly permitted by the copyright holder or applicable law, the project source code, ESP32 firmware, Python scripts, control logic, and original documentation may not be copied, modified, redistributed, published, sublicensed, or commercially used without prior written permission.

Access to this repository does not automatically grant permission to reuse the source code.

Third-party libraries, tools, and other components remain subject to their respective licenses and terms.

For the applicable terms, refer to the `LICENSE` file in the repository root.

**Repository access note:** A license states the conditions governing use; it does not technically prevent an authorized repository collaborator from copying accessible files. Repository visibility and collaborator permissions should therefore be configured appropriately.

## 👥 Team and Project Information

| Field | Details |
|---|---|
| Project | AQUA-CORE |
| Expanded project name | Automated Fluid Control, Monitoring & Evaluation System |
| Team name | Nova Thinkers |
| Team ID | `LRC-26_0050` |
| Competition | Lam Research Challenge 3.0 |
| Round | Hardware Hustle — Stage 2 |
| Repository | [AQUA-CORE-ESP32 on GitHub](https://github.com/kssethurathnamSMSV47/AQUA-CORE-ESP32) |

## 🙌 Acknowledgements

We acknowledge the opportunity provided by the Lam Research Challenge 3.0 to develop and demonstrate a hardware-oriented engineering solution. AQUA-CORE represents the work of Team Nova Thinkers in integrating embedded control, fluid handling, sensor-based monitoring, experimental data acquisition, and technical documentation.

---

**AQUA-CORE | Nova Thinkers | LRC-26_0050**  
*Lam Research Challenge 3.0 — Hardware Hustle Round (Stage 2)*
