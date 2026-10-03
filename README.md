
# ☀️ Arduino Solar Tracker



A single-axis solar tracker built on an **Arduino UNO**, two **LDRs** and a **servo motor**. It compares light on either side of the panel and rotates it toward the brighter side, so the panel keeps facing the sun throughout the day.

![Platform](https://img.shields.io/badge/platform-Arduino-00979D?logo=arduino&logoColor=white)
![Language](https://img.shields.io/badge/language-C%2B%2B-blue)
![Axis](https://img.shields.io/badge/tracking-single--axis-orange)
![License](https://img.shields.io/badge/license-MIT-green)

---

##  Overview

Fixed solar panels only sit at the ideal angle for a short part of the day. Published studies report that single-axis trackers can improve energy capture by roughly **25–30%** over stationary panels, and dual-axis systems by even more.

This project is a low-cost, easy-to-build tracker that:

- Senses the sun's direction using two light-dependent resistors (LDRs)
- Self-calibrates at startup to cancel out LDR mismatch
- Moves the panel smoothly with a variable step size
- Stays still (and silent) when the panel is already well aligned

##  Features

| Feature | Description |
|---|---|
| **Auto-calibration** | Measures the offset between the two LDRs at boot and compensates for it |
| **Noise filtering** | Each reading is the average of 15 ADC samples |
| **Deadband** | Errors below a threshold are ignored, so the panel doesn't hunt back and forth |
| **Variable step size** | Large error → bigger step (3°), small error → fine step (1°) |
| **Jitter-free idle** | The servo is `detach()`ed inside the deadband, which stops PWM and removes buzzing |
| **Safe travel limits** | Angle is clamped between 5° and 175° to protect the servo and mechanics |
| **Serial debug output** | Streams `L`, `R`, error and angle at 9600 baud |

##  Hardware Required

| Component | Qty | Notes |
|---|---|---|
| Arduino UNO (or compatible) | 1 | |
| LDR (photoresistor) | 2 | Mounted on opposite sides of the panel |
| Fixed resistor (~10 kΩ) | 2 | Forms a voltage divider with each LDR |
| Servo motor (e.g. SG90 / MG995) | 1 | Pick one with enough torque for your panel |
| Solar panel (small) | 1 | |
| Divider / shade barrier | 1 | Placed between the two LDRs so each sees a different amount of light |
| Jumper wires, breadboard | – | |
| External 5 V supply | 1 | Recommended for larger servos |

##  Wiring

| Component | Arduino Pin |
|---|---|
| LDR 1 (Left) divider midpoint | `A0` |
| LDR 2 (Right) divider midpoint | `A1` |
| Servo signal | `D9` |
| Servo VCC | 5 V (external supply recommended) |
| Servo GND | GND (**shared** with Arduino) |

**LDR voltage divider** (build one for each LDR):

```
 5V ──┬── LDR ──┬── 10kΩ ── GND
      │         │
      │         └──► A0 (or A1)
```

>  If you power the servo from an external supply, connect its ground to the Arduino's GND. Without a common ground the control signal won't work reliably.

##  How It Works

1. **Calibration (boot):** with both LDRs under the same light, the Arduino reads both and stores `offset = right − left`.
2. **Sensing:** every 300 ms, both LDRs are read (15-sample average each).
3. **Error calculation:** `error = (right − left) − offset`
4. **Decision:**
   - `|error| < 40` → panel is aligned, so the servo is detached and nothing moves.
   - Otherwise the servo is re-attached and the angle moves by a step that depends on the size of the error:

   | `|error|` | Step |
   |---|---|
   | > 300 | 3° |
   | > 150 | 2° |
   | ≤ 150 | 1° |

5. **Actuation:** the new angle is clamped to 5°–175° and written to the servo only if it changed.

```
        ┌────────┐   ┌──────────────┐   ┌──────────────┐   ┌───────┐
 Sun ──►│ 2×LDR  │──►│ Arduino UNO  │──►│  Servo (D9)  │──►│ Panel │
        └────────┘   │ filter+logic │   └──────────────┘   └───────┘
                     └──────────────┘
```

##  Getting Started

### 1. Clone the repository

```bash
git clone https://github.com/<your-username>/<your-repo>.git
cd <your-repo>
```

### 2. Upload the sketch

1. Open the `.ino` file in the **Arduino IDE**.
2. Select **Tools → Board → Arduino UNO** and the correct port.
3. Click **Upload**.

The `Servo` library ships with the Arduino IDE, so nothing extra needs installing.

### 3. Calibrate

Power the board with **both LDRs under the same light** (e.g. shade both, or point the panel straight at a uniform light source) and wait about 2 seconds. Open the Serial Monitor at **9600 baud** to see the measured offset:

```
Offset = 12
```

### 4. Monitor

```
L=512 R=640 Err=116 Ang=90
L=520 R=618 Err=86  Ang=91
```

##  Configuration

All tunable values are constants near the top of the sketch:

| Constant | Default | Purpose |
|---|---|---|
| `MIN_ANGLE` / `MAX_ANGLE` | `5` / `175` | Mechanical travel limits of the servo |
| `SMALL_ERR` | `40` | Deadband. Raise it if the panel keeps twitching, lower it for tighter tracking |
| `UPDATE_TIME` | `300` ms | Time between control decisions |
| Step thresholds | `150`, `300` | Error levels at which the step size increases |
| Filter samples | `15` | Number of ADC samples averaged per reading |

##  Troubleshooting

| Problem | Fix |
|---|---|
| Panel moves **away** from the light | Flip the sign of the error: `int error = (left - right) - offset;` |
| Servo jitters or buzzes | Increase `SMALL_ERR`, check the power supply and make sure grounds are common |
| Arduino resets when the servo moves | Power the servo from a separate 5 V supply |
| Panel never stops moving | Recalibrate under uniform light and check the divider between the LDRs |
| Panel doesn't react | Verify LDR wiring and watch `L` / `R` in the Serial Monitor |

##  Limitations

- **Single axis:** tracks east–west only, with no seasonal elevation adjustment.
- **Light-based:** heavy cloud or reflections can mislead the LDRs.
- **Blocking reads:** each sample burst uses `delay()` (about 90 ms per loop), which is fine for this application but not ideal for adding time-critical tasks.

##  Future Improvements

- [ ] Dual-axis tracking (azimuth + elevation)
- [ ] Temperature sensor to pause movement on overheating
- [ ] Humidity / environmental logging
- [ ] LCD or IoT dashboard for live status
- [ ] Return-to-east routine at night
- [ ] Voltage/current monitoring to quantify the actual energy gain



