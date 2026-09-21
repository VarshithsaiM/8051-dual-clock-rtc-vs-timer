# 8051 Embedded Clock Architectures: Hardware RTC vs. Interrupt-Driven

A comparative embedded systems project evaluating hardware-software trade-offs on the 8051 architecture. This project contrasts an autonomous, peripheral-offloaded Real-Time Clock (RTC) setup against a low-BOM, interrupt-driven software clock controlled via tactile inputs.

Both versions are implemented targeting the **AT89C51 / AT89S52**, developed in **Keil µVision**, and simulated in **SimulIDE**.

---

## 🛠 Tech Stack & Tooling

- **Target MCU:** 8051 (`AT89C51` / `AT89S52`)
- **Languages:** Embedded C (Keil C51) / 8051 Assembly
- **Toolchain:** Keil µVision IDE
- **Simulation Platform:** SimulIDE
- **Peripherals & Protocols:** HD44780 16x2 LCD (8-bit mode), I²C RTC Module (DS1307/DS3231), Hardware Timers, Active-Low Tactile Switches

---

## 📌 Architecture Breakdown & Schematics

### Version 1: Hardware-Assisted RTC Clock

An autonomous timekeeping circuit offloading timing generation to an external I²C RTC module. The MCU executes bit-banged I²C transactions across open-drain Port 0 pins pulled up to VCC with $10\text{ k}\Omega$ bus resistors.

![Version 1: RTC Hardware Schematic](schematics/v1-rtc-clock.png)

#### Pin Connections & Interfacing:
- **Display (HD44780 LCD - 8-Bit Bus):**
  - `P3.0` – `P3.7` $\rightarrow$ `D0` – `D7` (Full 8-bit data bus)
  - `P2.0` $\rightarrow$ `RS` (Register Select)
  - `P2.1` $\rightarrow$ `RW` (Read/Write grounded via $10\text{ k}\Omega$ pull-down)
  - `P2.2` $\rightarrow$ `EN` (Enable pulse)
- **RTC Module (I²C Bus):**
  - `P0.0` $\rightarrow$ `SDA` (Serial Data, pulled up with $10\text{ k}\Omega$)
  - `P0.1` $\rightarrow$ `SCL` (Serial Clock, pulled up with $10\text{ k}\Omega$)
  - `SQW` $\rightarrow$ Unconnected / optional external interrupt input

---

### Version 2: Firmware-Driven Clock (Timer + Push Buttons)

A minimalist BOM design eliminating external timing silicon. Time generation is driven internally by 8051 Timer interrupts, while two active-low tactile switches provide user adjustment (e.g., Mode/Select and Increment).

![Version 2: Timer and Buttons Schematic](schematics/v2-timer-buttons.png)

#### Pin Connections & Interfacing:
- **Display (HD44780 LCD - 8-Bit Bus):**
  - `P3.0` – `P3.7` $\rightarrow$ `D0` – `D7` (Identical pin mapping to V1 for driver reuse)
  - `P2.0` $\rightarrow$ `RS`
  - `P2.1` $\rightarrow$ `RW`
  - `P2.2` $\rightarrow$ `EN`
- **User Inputs (Push Buttons):**
  - `P1.0` $\rightarrow$ Push Button 1 (Active-low with internal pull-up, tied to GND)
  - `P1.1` $\rightarrow$ Push Button 2 (Active-low with internal pull-up, tied to GND)

---

## ⚖️ Engineering Trade-Off Matrix

| Metric | Version 1: RTC Module | Version 2: Timer + Buttons |
| :--- | :--- | :--- |
| **BOM Cost & Complexity** | Higher (RTC IC, crystal, battery, $10\text{ k}\Omega$ pull-ups) | Minimal (MCU, LCD, and 2 passive switches) |
| **I/O Pin Usage** | Port 3 (8-bit data), Port 2 (3 control), Port 0 (2 I²C) | Port 3 (8-bit data), Port 2 (3 control), Port 1 (2 inputs) |
| **Power-Loss Tolerance** | Retains time via coin-cell battery backup | Resets to initial time (`12:00:00`) on reboot |
| **Timing Accuracy** | Independent of main system oscillator | Bound to primary crystal drift and ISR jitter |
| **CPU Overhead** | Low (periodic polling loop to fetch registers) | Continuous (periodic timer ISR servicing & software debouncing) |
| **User Adjustability** | Typically programmed once via software | Manual on-the-fly adjustment via hardware inputs |

---

## 🧮 Timer Calculation (Version 2 Time Base)

Assuming a standard $11.0592\text{ MHz}$ oscillator:
- **Machine Cycle Period:**
  $$T_{MC} = \frac{12}{11.0592\text{ MHz}} \approx 1.08507\ \mu\text{s}$$
- **50 ms Tick Configuration (Timer 0, Mode 1 - 16-bit):**
  $$\text{Count} = \frac{50\text{ ms}}{1.08507\ \mu\text{s}} = 46080$$
  $$\text{Reload Value} = 65536 - 46080 = 19456\ (\text{0x4C00})$$
  $$\text{TH0} = \text{0x4C},\quad \text{TL0} = \text{0x00}$$
- An ISR tick accumulator counts 20 iterations to mark 1 elapsed second.


## 🚀 Build & Simulation Steps

1. **Build Hex Files (Keil µVision):**
   - Open the `.uvproj` file in either `v1-rtc/keil/` or `v2-timer-buttons/keil/`.
   - Build target (`F7`) to generate the firmware `.hex` file.
2. **Run in SimulIDE:**
   - Launch SimulIDE and load the corresponding `.simu` file from `docs/schematics/`.
   - Right-click the **8051** microcontroller $\rightarrow$ click **Load Firmware** $\rightarrow$ select the compiled `.hex` file.
   - Click **Power Circuit** to start the simulation.
