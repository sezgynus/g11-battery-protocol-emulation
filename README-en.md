![MIT License](https://img.shields.io/badge/license-MIT-blue.svg)
![ESP32](https://img.shields.io/badge/platform-ESP32-green)
![Reverse Engineering](https://img.shields.io/badge/domain-Reverse%20Engineering-orange)
![Hardware](https://img.shields.io/badge/type-Hardware%20%2B%20Firmware-critical)

# Language Selection
[English](README-en.md) | [Türkçe](README.md)

# G11 Battery Protocol Emulation


## Project Overview

This project reverse engineers the closed communication protocol between the Xiaomi G11 smart battery and the vacuum, decodes the protocol, and implements an ESP32-based emulator.

The objective was not merely to capture traffic, but to characterize the system to a reproducible level: from the physical layer to packet framing, addressing, checksum behavior, and functional payload fields. The resulting protocol implementation was validated with a working ESP32 emulator accepted by the original vacuum electronics.

### Outcome

- The closed battery interface was electrically characterized.
- The physical layer was identified as **single-wire, half-duplex, inverted UART**.
- Communication was decoded as **9600 baud, 8N1**.
- The vacuum was identified as the master/polling side and the battery as the slave/response side.
- Packet framing, source/target addressing, and the 16-bit additive checksum were decoded.
- The checksum algorithm was verified across approximately **6500 packets**.
- Across the three primary packet types, **29 / 37 bytes (~78%)** were characterized.
- The functional fields required for the demonstrated emulation were identified and an ESP32 protocol emulator was successfully validated.
- A battery-controller hardware design and physical battery enclosure were produced.

## Protocol at a Glance

| Property | Result |
|---|---|
| Topology | Single-wire |
| Duplex | Half-duplex |
| Encoding | Inverted UART |
| UART | 9600 baud, 8N1 |
| Master | Vacuum |
| Slave | Battery |
| Bus amplitude | ~24–25 V |
| Vacuum → Battery | 0xFB → 0xFC |
| Battery → Vacuum | 0xFC → 0xFB |
| Data integrity | 16-bit additive checksum |
| Characterized fields | 29 / 37 bytes (~78%) |
| Emulation platform | ESP32 |

```text
Start | Source ID | Target ID | Payload ... | Checksum_L | Checksum_H | End
```

```text
checksum = SUM(data bytes) & 0xFFFF
```

The checksum is the sum of packet bytes excluding the start/end delimiters and checksum field itself, and is carried little-endian as `Checksum_L`, `Checksum_H`.

### Confidence of Identified Fields

| Confidence | Fields |
|---|---|
| Verified | Packet framing, source/target ID, checksum, battery level, charger status, trigger status, motor active/inactive status |
| Strongly supported | Power consumption, commanded motor velocity |
| Partially characterized | Current field; behavioral correlation is strong but the unit remains uncertain |
| Unknown | Payload bytes that remained constant during testing or were not required for emulation |

> Payload coverage is a byte-count metric. Despite the unidentified fields, all fields required for the battery emulation demonstrated in this project were identified.

## Reverse Engineering Workflow

```text
Connector and pin identification
        ↓
Electrical characterization
        ↓
Safe signal acquisition
        ↓
Physical-layer identification
        ↓
Packet framing and addressing
        ↓
Checksum verification
        ↓
Controlled usage scenario
        ↓
Payload correlation
        ↓
ESP32 protocol emulation
```

The sections below preserve the measurements, hypotheses, and validation steps used to reach these results.

## Reverse Engineering Methodology

This project followed the deterministic engineering steps below:

<details>
<summary><strong>1️⃣ Interface Characterization</strong></summary>

The goal was to perform the analysis without opening the battery pack or the vacuum body.
Therefore, connector pin functions were identified using indirect and non-invasive methods.

## Reference PCB Images

Since the model is relatively new, teardown material is limited.

As a result of research:

- An image of the battery PCB was found in a video review,
- An image of the vacuum-side PCB was found on an online spare-parts platform.

> There are no markings indicating pin functions on the battery PCB.  
> The connector pin names are labeled on the vacuum-side PCB.

📌 Reference Images:

### Battery PCB Photo

![Battery PCB Reference](ASSETS/battery_pcb_reference.jpg)

### Vacuum PCB Photo

![Vacuum PCB Reference](ASSETS/vacuum_pcb_reference.png)

---

## Connector Pin Layout

The pin names on the 10-pin connector are:

P- | P- | P- | UI- | S | KEY | UI+ | P+ | P+ | P+

However, since it could not be confirmed that the referenced PCB belonged to exactly the same revision,
all pin functions were electrically verified.

---

## Electrical Verification

The battery and vacuum connectors were exposed using jumper wires,
and measurements were taken while the device was operating.

### Power Lines

- P- / P+ → Constant 24–25V DC  
  → Confirmed as the main power rail.

### UI Lines

- UI- / UI+ → 24–25V only while the display is active  
  → Identified as the display power supply lines.

### KEY Line

- Trigger pressed → 24–25V  
- Trigger released → 0V  

→ Confirmed as the user input line.

### S Line

- While the device is running → Periodic square waves with 24–25V amplitude  
- While the device is off → Constant 0V  

This behavior strongly indicates that the line is the communication line.

---

### Pin Mapping Labeling

To prevent connection errors in later analysis and to ensure measurement repeatability,
the identified pin mapping was physically labeled on both the battery and vacuum connectors.

This ensured that:

- Measurement points were standardized.
- The risk of incorrect connections was minimized.
- Reference confusion during data acquisition was prevented.

### Labeled Connector Images

<img src="ASSETS/battery_connector_labeled.jpg" alt="Battery Connector Labeling" width="500"> <img src="ASSETS/vacuum_connector_labeled.jpg" alt="Vacuum Connector Labeling" width="400">

---

## Conclusion

- Power lines were isolated.
- The user input line was verified.
- The communication line was identified.
- Signal amplitude was measured at ~24–25V.

Since the logic level is 24V, a direct logic analyzer connection is not possible.
A suitable level-shifting solution is required for the next stage.
</details>

<details>
<summary><strong>2️⃣ Data Acquisition and Protocol Discovery Attempts</strong></summary>

### Level Shifter Images

#### 1. Perfboard Implementation
<img src="ASSETS/level_shifter_photo1.jpg" alt="" width="300"> <img src="ASSETS/level_shifter_photo2.jpg" alt="" width="300">

#### 2. Schematic
![Level Shifter Schematic](ASSETS/bidirectonal_level_shifter_schemetic.jpg)

After the battery connector and level-shifter circuit were assembled,
a jumper was taken from the S line and connected to the logic analyzer input.  
This allowed the line to be monitored safely.

---

## Initial Analysis: 1-Wire Hypothesis

Because the communication used a single line, the first assumption was that it used the **1-Wire protocol**.

- Some meaningful bytes were observed,
- but there were many framing errors and uncorrelated byte sequences.

Capture file and screenshot:

[Capture File (Session 0.sal)](DOCUMENT/Session%200.sal)

<img src="ASSETS/Logic_Sesion0.png" alt="Logic Analyzer Capture" width="400">

These observations strengthened the possibility that the line was **not using the standard 1-Wire protocol**.

---

## Second Analysis: Half-Duplex Single-Wire UART Hypothesis

The next hypothesis was that the line might be **half-duplex single-wire UART**.

- The signal was analyzed using the most common standard baud rates.
- There were still many framing errors and no correlation was observed.

---

## Solution: Signal Inversion and Correct Parameters

As a final attempt, the signal was inverted and analyzed using:

- **8N1**
- **9600 baud**
- **Inverted signal**

With these parameters, the **frames aligned perfectly**.

Capture file and screenshots:

[Capture File (Session 1.sal)](DOCUMENT/Session%201.sal)

<img src="ASSETS/Logic_Sesion1.png" alt="Logic Analyzer Capture" width="400"> <img src="ASSETS/valid_uart_configuration.png" alt="Protocol Analyzer Configuration" width="400">

The byte sequences now began to show **stable and repeating correlations**.

## Master/Slave Identification

After the bit frames were captured correctly, it was necessary to determine which side was the master (querying side) and which side was the slave (responding side) for byte-level analysis and packet decoding.

Since the protocol uses a single line:

- One side continuously remains in listening mode.
- The other side performs polling.

To determine which side was the master:

1. The communication line was temporarily disconnected.
2. The vacuum was powered on.
3. The first communication attempt from each side was monitored separately.

### Result

- **Master / Polling side:** Vacuum
- **Slave / Responding side:** Battery

This finding provides a solid basis for correctly analyzing the dataset and for the next stage: **field identification**.

## Packet Start and End Conditions

When the byte stream captured by the logic analyzer was examined, a repeating pattern was observed:

- **0xFB** → Packet start
- **0xFC** → Packet end

Notable observations:

- 0xFC appears 12 bytes after 0xFB.
- 0xFB appears again 8 or 9 bytes after 0xFC.

These patterns were assumed to represent packet start and end conditions.

---

## Exporting to an Excel Table

According to these packet start/end conditions, an example communication flow can be separated as:

- Every 0xFB…0xFC packet → Vacuum to battery
- Every 0xFC…0xFB packet → Battery to vacuum

These packets were separated into rows and transferred into an **Excel table**.
Even though the meaning of every byte was not yet known, repeating fields could already be observed.

- **Rows with yellow background** → Packets sent from vacuum to battery
- **Rows with blue background** → Packets sent from battery to vacuum

### Example [Excel](DOCUMENT/G11_protocol_analyze.xlsx) View

<img src="ASSETS/example_packet_table.png" alt="Excel Packet Table Example" width="800">

## Intra-Packet Correlation and Initial Byte Analysis

When the Excel table was examined carefully, several meaningful correlations emerged.

Example vacuum → battery packet:

| Byte # |0    | 1   | 2  | 3  | 4 | 5| 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 |
|--------|-----|-----|----|---|---|---|---|---|---|----|----|----|----|----|
| Packet | FB  | 41  | 45 | 0B| 00| 00| 00| 00| 00| 09 | 00 | 9A | 00 | FC |

Corresponding battery → vacuum packet:

| Byte # |0    | 1   | 2   | 3  | 4  | 5  | 6  | 7  | 8  | 9  | 10 | 11 | 12 | 13 |
|--------|-----|-----|----|----|----|----|----|----|----|----|----|----|----|----|
| Packet | FC  | 45  | 41 | 44 | 64 | 64 | 00 | 92 | 01 | FB |    |    |    |    |

### First Correlation Finding

- **Byte 1 (0x41)** → Source ID
- **Byte 2 (0x45)** → Destination ID

These values were observed to swap correspondingly between the packet sent by the vacuum and the response packet sent by the battery.

This correlation provided the first clues about the **master/slave and addressing mechanism**.

### Checksum / Data Integrity Verification

Because the **source/destination ID correlation** was valid without exception across the packets,
the remaining **last two bytes** became more meaningful.

As in many serial communication protocols, the G11 battery protocol contains a
**checksum or CRC-like data-integrity verification field**.

#### Initial Assumption

- The **last two bytes** of each packet were assumed to be the checksum field.
- Packet start and packet end conditions were excluded from the calculation.
- The assumption could be applied even when packet length varied.

#### Verification

- Checksum calculations were performed on selected sample packets.
- Calculation method: checksum = SUM(all bytes except packet start/end conditions and checksum bytes)
- In all tested packets, the calculated checksum **exactly matched** the last two bytes in the packet.

#### Example Packet and Checksum

| Packet (vacuum → battery) | Byte 0 | Byte 1 | … | Byte n-2 (Checksum_L) | Byte n-1 (Checksum_H) | Byte n |
|----------------------------|--------|--------|---|----------|----------|--------|
| Example 1                  | 0xFB   | 0x41   | … | 0x9A     | 0x00     | 0xFC   |

Calculated Checksum: 0x41 + 0x45 + 0x0B + 0x09 = 0x009A

> The last two bytes exactly match the checksum contained in the packet.

### Checksum Verification Across All Packets

Since testing a single packet would not provide sufficient evidence,
checksum verification was applied to the **entire dataset**:

- In the Excel table, the last two bytes of each packet were assumed to be the checksum, excluding packet start and end conditions.
- The **checksum field contained in the packet** was compared with the **calculated checksum**.
- A formula-based column was created to automate this comparison.

#### Result

- Verification was performed on approximately **6500 packets**.
- Not a single packet violated the formula.

> The checksum field was therefore verified across the entire dataset of approximately 6500 packets.

#### Checksum Calculation

The formula below determines which bytes are included in the checksum according to the packet type and calculates the checksum:

```
=IF([@1]=41,
    DEC2HEX(SUM(HEX2DEC([@1]),HEX2DEC([@2]),HEX2DEC([@3]),HEX2DEC([@4]),HEX2DEC([@5]),HEX2DEC([@6]),HEX2DEC([@7]),HEX2DEC([@8]),HEX2DEC([@9]),HEX2DEC([@10])),4),
IF([@1]=45,
    DEC2HEX(SUM(HEX2DEC([@1]),HEX2DEC([@2]),HEX2DEC([@3]),HEX2DEC([@4]),HEX2DEC([@5]),HEX2DEC([@6])),4),
IF([@1]=42,
    DEC2HEX(SUM(HEX2DEC([@1]),HEX2DEC([@2]),HEX2DEC([@3]),HEX2DEC([@4]),HEX2DEC([@5]),HEX2DEC([@6]),HEX2DEC([@7])),4)
)))
```

In the **Checksum OK?** column, the following formula returns OK or ERROR depending on whether the calculated checksum matches the value in the checksum field:

```
=IF([@Checksum]=
IF([@1]=41,
  DEC2HEX(BITOR(BITLSHIFT(HEX2DEC([@12]),8),HEX2DEC([@11])),4),
IF([@1]=45,
  DEC2HEX(BITOR(BITLSHIFT(HEX2DEC([@8]),8),HEX2DEC([@7])),4),
IF([@1]=42,
  DEC2HEX(BITOR(BITLSHIFT(HEX2DEC([@9]),8),HEX2DEC([@8])),4),0
))),"OK","ERROR")
```

#### Excel Checksum Field Verification

<img src="ASSETS/excel_checksum_validation.png" alt="Excel Checksum Validation" width="800">
</details>

<details>
<summary><strong>3️⃣ Field Identification (Payload Analysis)</strong></summary>

To identify the fields in the payload, an approximately **4-minute usage scenario** was prepared.
The operations performed at each point in time were recorded in a table:

| Time     | Event |
|----------|-------|
| 00:04:00 | Power on |
| 00:07:00 | Mode change: AUTO → TURBO |
| 00:11:00 | Mode change: TURBO → ECO |
| 00:13:00 | Trigger pressed – motor started (ECO) |
| 00:20:00 | Trigger released – motor stopped (ECO) |
| 00:21:00 | Trigger pressed – motor started (ECO) |
| 00:27:00 | Trigger released – motor stopped (ECO) |
| 00:29:00 | Mode change: ECO → AUTO |
| 00:30:00 | Trigger pressed – motor started (AUTO) |
| 00:36:00 | Trigger released – motor stopped (AUTO) |
| 00:38:00 | Trigger pressed – motor started (AUTO) |
| 00:42:00 | Trigger released – motor stopped (AUTO) |
| 00:43:00 | Mode change: AUTO → TURBO |
| 00:45:00 | Trigger pressed – motor started (TURBO) |
| 00:52:00 | Trigger released – motor stopped (TURBO) |
| 00:53:00 | Trigger pressed – motor started (TURBO) |
| 01:04:00 | Trigger released – motor stopped (TURBO) |
| 01:05:00 | Trigger lock enabled |
| 01:08:00 | Trigger pressed – motor started (TURBO) |
| 01:08:00 | Trigger released |
| 01:15:00 | Vacuum inlet blocked |
| 01:16:00 | Vacuum inlet unblocked |
| 01:17:00 | Vacuum inlet blocked |
| 01:18:00 | Vacuum inlet unblocked |
| 03:03:00 | Trigger pressed – motor stopped (TURBO) |
| 03:04:00 | Trigger released |
| 03:06:00 | Trigger pressed – motor started (TURBO) |
| 03:06:00 | Trigger released |
| 03:25:00 | Mode change: TURBO → ECO |
| 03:29:00 | Mode change: ECO → AUTO |
| 03:31:00 | Mode change: AUTO → TURBO |
| 03:34:00 | Mode change: TURBO → ECO |
| 03:40:00 | Trigger pressed – motor stopped (TURBO) |
| 03:40:00 | Trigger released |
| 03:47:00 | Charger connected (63%) |
| 03:52:00 | Charger disconnected (63%) |
| 03:56:00 | END |

---

## Payload Correlation and Battery Level

> **Note:** Byte and bit indices in this document are zero-based.

- During the usage scenario, data was captured simultaneously with the **logic analyzer**.
- Packets were transferred to the Excel table using the previously created formulas and columns.
- The vacuum display shows the **battery charge level** in real time, so it was assumed that packets coming from the battery must contain a **Battery Level** field.

### Battery Level Field Identification

- Battery-to-vacuum packets (starting with 0xFC and ending with 0xFB) were filtered.
- The **Byte 4** field of packets with source ID **0x45** showed a decreasing trend over time:
  - Decimal 100 at the beginning
  - Decimal 63 at the end of the usage scenario
- This byte was identified as **Battery Level (%)**.

#### Excel Formula and Graph

- A column calculating **Battery Level** from Byte 4 was added for all packets.
- The value was plotted over time:

<img src="ASSETS/battery_level_graph.png" alt="Battery Level Over Time" width="800">

<img src="ASSETS/battery_level_table.png" alt="" width="400"> <img src="ASSETS/battery_level_table2.png" alt="" width="400">

### Charger Status Field Identification

Another piece of information expected from the battery side is the **charger connection status**.

- Since this state is shown instantly on the vacuum display, a field containing this information had to exist in the battery-originated packets.
- The byte and bit corresponding to charger insertion/removal events in the usage scenario were searched:
  - Source ID: **0x45**
  - Byte: **Byte 3**
  - Bit: **Bit 3**

The state of this bit was interpreted as:

- **1 → Charger connected**
- **0 → Charger disconnected**

A new column was added to the Excel table to calculate this bit for all packets.
The results matched the events in the usage scenario exactly.

#### Charger Status Image

<img src="ASSETS/charger_status.png" alt="Charger Status Column in Excel" width="600">

### Power Mode and Motor Power Field Analysis

During the usage scenario, the vacuum was intentionally switched between **power modes**, stopped and restarted, and briefly loaded by blocking the airflow.
The purpose was to analyze changes in protocol packets corresponding to those exact moments.

- The vacuum has **3 power levels**: low → medium → high.
- Changes in **motor speed, motor voltage, and current** are expected during mode transitions.

#### Packet with Source ID 0x45

- There were **2 remaining unidentified bytes** in this packet.
- These values were assumed to be BMS error/status flags. Since the battery was healthy and these states could not be generated, they were left outside the scope of the current analysis.

#### Packet with Source ID 0x42

- There was a **5-byte payload area** still waiting to be decoded.
- For analysis, **Byte 3 and Byte 4 were concatenated into a 16-bit numeric value**.
- All 0x42 packets were filtered, added to the Excel table, and plotted as a line graph.

#### Result

- Peaks reaching a value of approximately **500 around the 45th second** were observed.
- The behavior before and after these peaks matched vacuum start/stop operations and fluctuations caused by blocking the airflow.
- According to the device specifications, the vacuum is rated at **500 W**.
- Taken together, these findings strongly support the interpretation that this **16-bit field represents power consumption in watts**.

### Power Consumption Graph

<img src="ASSETS/power.png" alt="Power Consumption Table" width="600"> <img src="ASSETS/wattage.png" alt="Power Consumption Graph" width="600">

### Current Field Identification

- Byte 5 and Byte 6 of packets with source ID 0x42 were concatenated into a 16-bit column.
- A line graph of this column was generated and analyzed.

The graph showed:

- Clear peaks when the vacuum inlet was blocked.
- This is consistent with electric motor behavior: when the motor is mechanically loaded, its current draw increases.
- This behavior is clearly visible in the graph.
- The values appear somewhat low compared with the nominal power of the device, so the unit is not certain. The field may contain a raw ADC value. Alternatively, if the motor operates at a higher voltage than initially assumed, interpreting the values in amperes may be reasonable.
- The patterns observed during blocking and mode transitions are consistent with this interpretation; therefore, the field was treated as **current data**.

### Current Graph

<img src="ASSETS/current_table.png" alt="Current Table" width="600"> <img src="ASSETS/current.png" alt="Current Graph" width="600">

### Derived Voltage and Validation of the Current Field

To better understand the current and power data and to support the current-field identification:

- A derived **voltage column** was created:
  - Formula: \( P = V × I \)
  - The power column was divided by the assumed current column.
- A line graph of the resulting voltage column was generated.

Graph analysis showed:

- **Voltage drops when the vacuum inlet was blocked** → voltage sag under load is expected behavior for an electric motor system.
- **Voltage peaks when the motor starts from zero speed** were plausible and consistent with motor-system behavior.
- These observations support the internal consistency of the previously identified **current and power fields**.
- These results provide additional evidence supporting the current-field interpretation.

<img src="ASSETS/calculated_voltage.png" alt="Voltage Table" width="600"> <img src="ASSETS/derivative_voltage.png" alt="Voltage Graph" width="600">

### Motor Active/Inactive State (Motor Status)

Only **Byte 7** in the packet with source ID 0x42 remained unidentified at this stage.

When this byte was examined:

- Motor running → 1
- Motor stopped → 0

This behavior was consistent throughout the entire usage scenario.
Therefore, without needing a graph, this byte was directly identified as **Motor Active/Inactive (Motor Status)**.

## Vacuum Packets: Source ID 0x41

The **only packet type sent by the vacuum**, with source ID 0x41, was analyzed.

- The fields that changed throughout the dataset were Byte 3, Byte 4, Byte 5, and Byte 6.
- Since Byte 6 could only take the values 0 and 1, it was initially left for later analysis.

### Byte 3, Byte 4, and Byte 5 – Motor Speed / Commanded Velocity

- Byte 3, Byte 4, and Byte 5 were concatenated into a 24-bit value.
- The graph showed a minimum value of 0 and a maximum value of 128000.
- The pattern was consistent with the current and power graphs.

> These values suggested that the field represented motor speed, since approximately 128000 RPM is a plausible high rotational speed for a vacuum motor.  
> However, the graph was extremely stable and did not show the fluctuations visible in the current and power fields. This suggests that the value is not **actual velocity**, but rather the **commanded velocity sent to the motor driver**.

<img src="ASSETS/motor_speed.png" alt="Motor Commanded Velocity Graph" width="600"> <img src="ASSETS/motor_rpm.png" alt="Motor Commanded Velocity Graph" width="600">

#### Byte 6 – Trigger Status

- **Byte 6** completely represents the **trigger pressed/released state**.  
  → All changes were verified throughout the 4-minute usage scenario.

No changes were observed in the other bytes during the usage scenario.

---

</details>

## Protocol Reference

This section consolidates the packet definitions obtained at the end of the reverse-engineering process. Byte and bit indices are zero-based.

### Result: Decoded Packets and Fields

#### 1️⃣ Packet Sent from Vacuum to Battery (Source ID 0x41)

| Byte | Field | Description |
|------|------|-------------|
| 0    | Packet Start | 0xFB |
| 1    | Source ID | 0x41 |
| 2    | Target ID | 0x42/0x43/0x44/0x45 |
| 3    | Motor Speed Low Byte | Motor speed value, 16–24 bit |
| 4    | Motor Speed Mid Byte | Motor speed value, 16–24 bit |
| 5    | Motor Speed High Byte | Motor speed value, 16–24 bit |
| 6    | Trigger Status | 0: Not pressed, 1: Pressed |
| 7    | Unknown | Not yet decoded |
| 8    | Unknown | Not yet decoded |
| 9    | Unknown | Not yet decoded |
| 10   | Unknown | Not yet decoded |
| 11   | Checksum_L | Packet checksum |
| 12   | Checksum_H | Packet checksum |
| 13   | Packet End | 0xFC |

---

#### 2️⃣ Packet Sent from Battery to Vacuum (Source ID 0x45)

| Byte | Field | Description |
|------|------|-------------|
| 0    | Packet Start | 0xFC |
| 1    | Source ID | 0x45 |
| 2    | Target ID | 0x41 |
| 3    | Charger Status | 3rd bit (0: Disconnected, 1: Connected) |
| 4    | Battery Level | Charge level in % |
| 5    | Unknown | Not yet decoded |
| 6    | Unknown | Not yet decoded |
| 7    | Checksum_L | Packet checksum |
| 8    | Checksum_H | Packet checksum |
| 9    | Packet End | 0xFB |

---

#### 3️⃣ Packet Sent from Battery to Vacuum (Source ID 0x42)

| Byte | Field | Description |
|------|------|-------------|
| 0    | Packet Start | 0xFC |
| 1    | Source ID | 0x42 |
| 2    | Target ID | 0x41 |
| 3    | Power_L | Power consumption in Watts (16-bit) |
| 4    | Power_H | Power consumption in Watts (16-bit) |
| 5    | Current_L | Current data (16-bit, unit estimated) |
| 6    | Current_H | Current data (16-bit, unit estimated) |
| 7    | Motor Active Flag | 1: Motor running, 0: Motor stopped |
| 8    | Unknown | Not yet decoded |
| 9    | Unknown | Not yet decoded |
| 10   | Checksum_L | Packet checksum |
| 11   | Checksum_H | Packet checksum |
| 12   | Packet End | 0xFB |

### Payload Coverage

| Packet | Total Bytes | Analyzed Bytes | Coverage |
|--------|-------------|----------------|----------|
| 0x41   | 14          | 10             | 71%      |
| 0x45   | 10          | 8              | 80%      |
| 0x42   | 13          | 11             | 85%      |
| **Total** | 37       | 29             | **~78%** |

> The undecoded bytes are either constant/padding fields or protocol fields that were not active during the tests.  
> The extracted data is sufficient to emulate the battery and operate the vacuum with all demonstrated functions.

<details>
<summary><strong>4️⃣ Protocol Emulation</strong></summary>

For the protocol emulation stage, the **ESP32** was selected as the microcontroller.
Its easy availability, convenient development environment, and integrated Wi-Fi make it an ideal platform for preparing the project for future IoT scenarios.

The **logic-level shifter** previously built and used for the logic analyzer connection was reused for the ESP32 UART connection.
Since both RX and TX share the same line in the single-wire UART topology, a connection through **GPIO16** alone was sufficient.

On the software side, a **communication interface class** was developed. This class:

- Switches the single-wire UART port to RX while listening and to TX while transmitting.
- While in listening mode, reads the **Target ID** from each packet received from the master (vacuum) and selects the appropriate response from its **data buffer**.
- Switches to TX mode and keeps the protocol operating continuously.
- When the `begin()` function is called, starts its own **FreeRTOS thread**, and the protocol runs continuously inside the thread loop.

For testing:

- **GPIO33** ADC input → Battery voltage field simulation
- **GPIO26** digital input → Charger status field simulation

With this architecture, the application layer only needs to start the **communication interface** and read or write the relevant bytes in the **data buffer**.

The firmware is available [here](SOFTWARE/G11_Battery_Controller).

After testing and validating the communication interface, the first application-layer test continuously incremented the **Battery Level** field and wrapped it back to 0 after exceeding 100.
Protocol emulation tests were started with this implementation, and the **protocol was successfully emulated on the first attempt**.

![Battery Level Test GIF](ASSETS/VID_20240125_152934-ezgif.com-video-to-gif-converter.gif)

In the second stage, the **Battery Level** field was manipulated using a potentiometer connected to the previously defined **ADC input (GPIO33)**, while the **Charger Status** field was manipulated through the **digital input (GPIO26)**.
This allowed the battery state to be controlled with the potentiometer and the charger state to be changed using the digital input.
These tests were also successful.

<img src="ASSETS/pot_test.gif" alt="" width="400"> <img src="ASSETS/charger.gif" alt="" width="400">

Since data from the vacuum could already be received, the software infrastructure was considered sufficient for further development and the firmware was left at this stage.
When the required **BMS circuit** and other peripherals are added, this infrastructure can provide full emulation of the original battery.

</details>

<details>
<summary><strong>5️⃣ Hardware Interface Design</strong></summary>

The next stage toward producing a replacement battery is to establish the foundations of the hardware design.

The original battery does not normally provide voltage on the previously described **UI+ / UI- pins**.
When the vacuum's **trigger is pressed and the KEY signal is generated**, the vacuum enables the **UI supply** for a specific timeout period and starts communication once power is available.
If there is no activity, the supply is turned off to save power.
For this reason, the UI lines cannot simply be connected directly to 24V.

As a solution, a **switching circuit for controlling the UI lines** was implemented and connected to the ESP32 I/O pins.
For additional safety, the **entire system power was also made switchable** using a separate switching circuit together with a **DC-DC buck converter** for the MCU supply.
These arrangements can be seen on the **power page of the schematic**.

<img src="DOCUMENT/power.png" alt="" width="400">

In addition, to allow the MCU to measure the battery level in real time and report it to the vacuum, the **5S series battery voltage was connected to an ADC input through a voltage divider that scales it to the 0–3.3V range**.
This allows the battery voltage to be measured.

<img src="DOCUMENT/mcu.png" alt="" width="400">

The **bidirectional level-shifter** topology used during the tests was also integrated into the schematic.

<img src="DOCUMENT/comm.png" alt="" width="400">

To detect charger connection, the charger input was connected to a digital I/O through another **voltage divider**.

<img src="DOCUMENT/conn.png" alt="" width="400">

These additions not only provide all the functions of the original battery, but also add **Wi-Fi capability**, turning the battery into a device capable of advanced IoT functionality.

The complete schematic and PCB project are available [here](HARDWARE/G11%20Battery%20Controller).
</details>

<details>
<summary><strong>🎁 Bonus: 3D Battery Case Design</strong></summary>

<img src="ASSETS/case1.png" alt="" width="400"> <img src="ASSETS/case2.png" alt="" width="400">

The 3D design files for the physical battery enclosure are available [here](3D/g11%20battery%20case).

</details>

## Repository Structure

| Directory | Contents |
|---|---|
| `ASSETS/` | Photos, graphs, test images, and GIFs |
| `DOCUMENT/` | Logic-analyzer captures, Excel protocol analysis, and schematic images |
| `SOFTWARE/` | ESP32 protocol emulator |
| `HARDWARE/` | Battery-controller schematic and PCB project |
| `3D/` | Battery enclosure design files |

## Project Status

This is a **completed reverse-engineering and proof-of-concept emulation project**.

- Reverse engineering: **Completed**
- Protocol emulation: **Completed**
- Hardware design: **Completed**
- Firmware proof of concept: **Completed**
- Further protocol analysis: **Not planned**

Some payload bytes remain unidentified because they did not change during the available tests or were not required for the demonstrated emulation. The functional emulation targeted by this project was validated with the available data and implementation.
