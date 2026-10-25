#  LPC21xx RFID Access Control System

An embedded C project for the **NXP LPC2148** microcontroller that implements a secure RFID-based access control system with motorized door actuation, LCD feedback, and security alerts.

## Features
* **RFID Authentication:** Reads unique 125kHz tags via UART (EM-18 reader).
* **Motorized Door:** Opens the door (motor forward) upon valid scan and closes it when push button `S1` is pressed.
* **LCD Display:** Shows real-time status (`VALID USER`, `INVALID USER`, `DOOR OPEN`, etc.) in 4-bit mode.
* **Security Alerts:** Triggers an LED and buzzer for unauthorized scans.

##  Hardware Pin Connections (Port 0)
| Component | Pin Name | LPC21xx Pin |
| :--- | :--- | :--- |
| **LCD RS** | `RS` | P0.10 |
| **LCD EN** | `EN` | P0.12 |
| **LCD Data** | `D4`–`D7` | P0.16–P0.19 |
| **Motor Forward** | `mf` | P0.8 |
| **Motor Reverse** | `mr` | P0.4 |
| **Push Button** | `SW` | P0.5 |
| **Status LED** | `LED1` | P0.6 |
| **Buzzer** | `buzz` | P0.7 |
| **UART RX/TX** | RXD0 / TXD0 | P0.1 / P0.0 |

##  How to Run
1. Open **Keil µVision** and create an LPC2148 project.
2. Add `main.c` to your source group.
3. Build the project to generate the `.hex` file.
4. Flash the binary to your development board using **Flash Magic**.
