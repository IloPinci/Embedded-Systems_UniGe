# Assignment 1 - Digital I/O on dsPIC33EP512MU810

> **Course:** Embedded Systems  
> **MCU:** Microchip dsPIC33EP512MU810  
> **Toolchain:** MPLAB X IDE · XC16 Compiler  
> **Language:** C (bare-metal, no RTOS)  
---

## Overview

This assignment explores direct manipulation of General-Purpose I/O (GPIO) on a 16-bit Digital Signal Controller. The two exercises progress from **level-triggered** input reading (polling the current pin state) to **edge-triggered** input detection (detecting a *transition*, not a sustained state), demonstrating a fundamental distinction in digital systems design.

No timers or interrupts are used here. All logic runs in a bare polling loop (`while(1)`), which makes the control flow transparent and easy to reason about at the cost of 100% CPU utilisation.

---

## Hardware

| Signal | Pin / Port | Direction | Notes |
|--------|-----------|-----------|-------|
| Button 1 | `RE8` | Input | Active-HIGH (reads 1 when pressed) |
| Button 2 | `RE9` | Input | Active-HIGH |
| LED 1 | `RA0` | Output | Driven via LAT register |
| LED 2 | `RG9` | Output | Driven via LAT register |

---

## Register-Level GPIO Configuration

The dsPIC33EP uses a three-register model for each I/O port. Understanding each layer is critical before writing a single line of logic:

### 1. `ANSEL` - Analog/Digital Select
Pins on Port E are shared with the ADC peripheral. On reset, they default to **analog mode**, which **disconnects the digital input buffer**. Reading `PORTEbits.RE8` while the pin is in analog mode always returns 0, regardless of physical voltage. This is a frequent source of silent bugs.

```c
ANSELEbits.ANSE8 = 0;   // Switch RE8 to digital mode
ANSELEbits.ANSE9 = 0;   // Switch RE9 to digital mode
```

### 2. `TRIS` — Tri-State (Direction) Register
Controls whether a pin is driven (`0` = output) or high-impedance (`1` = input). The mnemonic is: **1 = Input, 0 = Output** (think of `1` as "hands off").

```c
TRISEbits.TRISE8 = 1;   // RE8 → input
TRISGbits.TRISG9 = 0;   // RG9 → output
```

### 3. `LAT` vs `PORT` — Write vs Read
A common mistake on PIC architectures is using `PORT` for both reading and writing:

| Register | Use case |
|----------|----------|
| `PORTx` | **Read** the actual pin voltage |
| `LATx`  | **Write** the output latch (and read it back safely) |

Writing to `PORTx` causes a **read-modify-write hazard** on slow or capacitively-loaded pins (the pin may not have settled before the read). Always write to `LATx`.

---

## Exercise Breakdown

### Exercise 1 — Level-Triggered (ex-2.X)
The LED mirrors the button state continuously. As long as the button is held, the LED is off; releasing it turns the LED on (inverted logic driven by the hardware pull-up configuration).

```c
while(1) {
    button1 = PORTEbits.RE8;
    LATAbits.LATA0 = (button1 == 1) ? 0 : 1;
}
```

**Behaviour:** The LED reflects the *instantaneous* pin state every loop iteration. There is no memory of previous state.

---

### Exercise 2 — Edge-Triggered / Toggle (ex-3.X)
The LED changes state only on the **rising edge** of the button signal (the moment the button transitions from not-pressed to pressed). This requires storing the previous sample.

```c
uint16_t button_initial = 0;

while(1) {
    uint16_t button_now = PORTEbits.RE8;

    if (button_now == 1 && button_now != button_initial) {
        LATAbits.LATA0 = (val + 1) % 2;
        val = (val + 1) % 2;
    }
    button_initial = button_now;
}
```

**Key concept:** The condition `button_now != button_initial` is a software implementation of edge detection — it fires exactly once per low→high transition, no matter how long the button is held.

> **Limitation - No Debouncing:** Mechanical buttons produce several spurious transitions (contact bounce) within the first ~5–20 ms of a press. Without a debounce strategy (time-based or hysteresis-based), a single physical press may register multiple toggles. This is addressed in Assignment 3.

---

## Build & Flash

1. Open the project folder (`ex-2.X` or `ex-3.X`) in [**MPLAB X IDE**](https://www.microchip.com/en-us/tools-resources/develop/mplab-x-ide).
2. Confirm the selected compiler is **XC16** and the device is set to `dsPIC33EP512MU810`.
3. Add `my-libraries/` to the include path: *Project Properties → XC16 → Include directories*.
4. Build (`F11`) and upload the program via [mikroBootloader](https://www.mikroe.com/mikrobootloader?srsltid=AfmBOooGMKGdqtimcJMwZ_FDSngW3BXKDDjIeBUikH75L_sr6B0-JBkN).

---

## Key Takeaways

- ANSEL must be cleared before any digital read on Port E - defaulting to analog is a hardware-level trap.
- LAT/PORT distinction matters for output correctness under read-modify-write scenarios.
- Edge detection requires state memory; level detection does not.
- Polling consumes the full CPU - acceptable for trivial tasks, unsuitable for concurrent operations (motivation for timers and interrupts in later assignments).
