# Assignment 2 - Hardware Timers on dsPIC33EP512MU810

> **Course:** Embedded Systems  
> **MCU:** Microchip dsPIC33EP512MU810  
> **Toolchain:** MPLAB X IDE · XC16 Compiler  
> **Language:** C (bare-metal, no RTOS)  

---

## Overview

This assignment replaces software delay loops (`for` / `while` spin-waits) with **hardware timer peripherals**, which count independently of the CPU and produce precise, clock-derived delays. The deliverable is a reusable timer abstraction library (`timer_config.h / timer_config.c`) used by all subsequent assignments.

The assignment demonstrates: timer register configuration, prescaler selection, period calculation, and the difference between a **blocking (busy-wait)** timer and a **non-blocking (period-check)** timer - a design choice with significant implications for real-time systems.

---

## Hardware Timer Architecture (dsPIC33EP)

Each hardware timer on the dsPIC33EP consists of:

| Component | Role |
|-----------|------|
| `TMRx` | 16-bit up-counter, incremented each prescaled tick |
| `PRx` | Period register - counter resets to 0 and sets the interrupt flag when `TMRx == PRx` |
| `TxCON` | Control register - clock source, prescaler, enable/disable |
| `IFS0bits.TxIF` | Interrupt flag - set when the period is reached |

The counting equation is:

```
f_tick = Fcy / Prescaler
Period (ticks) = f_tick × t_desired
PR = Period (ticks) - 1        ← subtract 1: the flag fires at the end of the PRx+1 th tick
```

---

## Clock & Prescaler Selection

The CPU clock is configured at **Fcy = 72 MHz** (72,000,000 ticks/second). Timer registers are 16-bit, so the maximum count before overflow is **65,535**.

Without prescaling, the maximum measurable period is:

```
t_max = 65535 / 72,000,000 ≈ 0.91 ms
```

That is far too short for human-scale delays. A **1:256 prescaler** divides the clock down:

```
f_tick = 72,000,000 / 256 = 281,250 Hz
Ticks per ms = 281,250 / 1000 = 281.25
t_max = 65535 / 281.25 ≈ 233 ms
```

This supports delays up to ~233 ms per timer period. Longer delays are achieved by chaining 1 ms periods in a loop (see `tmr_wait_ms`).

```c
#define Fcy         72000000UL
#define Prescaler   256
#define Ticks_per_ms (Fcy / (Prescaler * 1000UL))   // = 281 (integer truncation noted below)
```

> **Precision Note:** `Ticks_per_ms = 72,000,000 / 256,000 = 281.25`. Integer division truncates to **281**, introducing a ~0.089% timing error per millisecond (~89 µs per second of elapsed time). For this assignment the error is negligible; for precision timing applications, fractional accumulation or a different prescaler should be considered.

---

## Library Design

### `tmr_setup_period(int timer, int ms)`
Configures and **starts** a timer to fire after `ms` milliseconds. Steps performed:

1. Disable the timer (`TON = 0`) to safely write configuration registers.
2. Select internal clock source (`TCS = 0` → Fcy).
3. Set prescaler to 1:256 (`TCKPS = 3`).
4. Reset the counter (`TMRx = 0`) and clear the interrupt flag (`TxIF = 0`).
5. Write the period register: `PRx = (Ticks_per_ms × ms) - 1`.
6. Enable the timer (`TON = 1`).

### `tmr_wait_period(int timer)` - Non-Blocking Check
Returns immediately if the flag is already set (timer has already expired), otherwise **busy-waits** until the flag sets. Returns `1` on early exit (deadline missed), `0` on normal completion.

This function is designed for **periodic tasks**: call `tmr_setup_period` once to set the rhythm, then call `tmr_wait_period` at the end of each task body to synchronise to the next period - similar in concept to `vTaskDelayUntil()` in FreeRTOS.

### `tmr_wait_ms(int timer, int ms)` - Blocking Delay
Provides a simple one-shot blocking delay. Internally it iterates `ms` times over 1 ms periods:

```c
void tmr_wait_ms(int timer, int ms) {
    tmr_setup_period(timer, 1);        // configure for 1 ms ticks
    for (int i = 0; i < ms; i++) {
        tmr_wait_period(timer);        // wait for each 1 ms tick
    }
    T1CONbits.TON = 0;                 // stop timer when done
}
```

**Why 1 ms ticks and not one large period?** A single-period approach for e.g. 2000 ms would require `PR = 281.25 × 2000 = 562,500`, which **overflows a 16-bit register**. Iterating over 1 ms units avoids this while staying within the prescaler's range.

---

## Application - LED Blink (Exercise 2)

```c
while(1) {
    val = (val + 1) % 2;
    LATAbits.LATA0 = val;
    tmr_wait_ms(TIMER1, 1000);   // block for exactly 1 second
}
```

LED toggles every 1 second. The timer provides hardware-accurate timing independent of instruction count - something a `for` loop delay cannot guarantee once compiler optimisations are enabled.

---

## Build & Flash

1. Open `Exercise-2/asgn_2.X` in [**MPLAB X IDE**](https://www.microchip.com/en-us/tools-resources/develop/mplab-x-ide).
2. Add `my-libraries/` to: *Project Properties → XC16 → Include directories*.
3. Add `timer_config.c` as a source file in the project.
4. Build (`F11`) and upload the program via [mikroBootloader](https://www.mikroe.com/mikrobootloader).

---

## Key Takeaways

- Hardware timers free the CPU from counting cycles and are immune to compiler optimisation - software delay loops are not.
- Prescaler selection is a trade-off between resolution and maximum period; document the choice explicitly.
- Integer truncation in tick-per-ms calculations introduces systematic drift - quantify it.
- A non-blocking period check (`tmr_wait_period`) enables a rudimentary **cyclic executive** scheduling model.
- Stopping the timer after `tmr_wait_ms` prevents ghost interrupts in future assignments.
