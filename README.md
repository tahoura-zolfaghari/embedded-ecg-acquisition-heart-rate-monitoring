# Embedded ECG Acquisition and Heart Rate Monitoring System

**Undergraduate Thesis | Islamic Azad University, Najafabad Branch | October 2026**

**Supervisor:** Dr. Mohammad Reza Yusefi

## Project Overview

This project focuses on building a low-cost, single-lead ECG acquisition and heart rate monitoring prototype using an Arduino microcontroller and the AD8232 ECG analog front-end.

The system has two operating modes. Since continuous ECG plotting and periodic text reports use the same serial connection, separate Arduino sketches are used for each mode.

## Operating Modes

### 1. Real-Time ECG Plotter

File: `ecg_realtime_plotter.ino`

- Reads ECG samples from the AD8232 module.
- Sends ECG values continuously over serial for real-time plotting.
- Estimates heart rate using threshold-based beat detection.
- Detects lead disconnection using the module's lead-off signals.

### 2. Periodic ECG Reporting

File: `ecg_periodic_reporting.ino`

- Estimates heart rate from detected beats.
- Calculates RR intervals and average BPM.
- Tracks the signal range to provide a basic signal quality estimate.
- Prints a status report every three seconds, including heart rate, lead status, and signal quality.
- Activates an output alarm when the estimated heart rate is outside the configured range.

## Hardware

- Arduino microcontroller
- AD8232 ECG analog front-end
- ECG electrodes

## Tools and Technologies

- Arduino IDE
- Embedded C/C++
- ECG signal acquisition
- Serial data visualization
- Basic signal analysis and heart rate estimation

## Project Scope and Limitations

This project was developed as an undergraduate thesis prototype for educational and research purposes. The heart rate estimation and signal quality assessment are basic implementations and have not been clinically validated. The system is not intended for medical diagnosis.

## Confidentiality

Only source code and documentation authorized for public release are included in this repository. Restricted hardware design files are not shared.
