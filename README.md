# Assignment 3 - Interrupts on dsPIC33EP512MU810

> **Course:** Embedded Systems  
> **MCU:** Microchip dsPIC33EP512MU810  
> **Toolchain:** MPLAB X IDE · XC16 Compiler  
> **Language:** C (bare-metal, no RTOS) 
---

## Overview

This assignment replaces the polling model from previous assignments with **hardware interrupts**, allowing the CPU to respond to events asynchronously. Two interrupt sources are explored: a **timer interrupt** for generating a precise periodic event without blocking the main loop, and an **external pin interrupt** with **software debouncing** for reliable button input.

The central design question answered here is: *when should a task be handled in an ISR vs. the main loop?* The answer shapes every real-time embedded application.

---

## Interrupt Fundamentals on dsPIC33EP

When an interrupt fires, the CPU:
1. Finishes the current instruction.
2. Pushes the Program Counter and status registers onto the hardware stack.
3. Jumps to the **Interrupt Service Routine (ISR)** vector.
4. Returns via `RETFIE`, restoring context.

Three register families control every interrupt source:

| Register family | Role |
|----------------|------|
| `IECx` - Interrupt Enable Control | Arm/disarm a specific interrupt source |
| `IFSx` - Interrupt Flag Status | Set by hardware on event; **must be cleared by software in the ISR** |
| `IPCx` - Interrupt Priority Control | Priority 0-7; higher preempts lower |

> **Critical rule:** Failing to clear `IFSx` at the end of an ISR causes the CPU to immediately re-enter the ISR after returning - an infinite interrupt loop that locks the system.

---

## ISR Declaration Syntax (XC16)

```c
void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void) {
    // ...
    IFS0bits.T1IF = 0;  // always last - clear the flag
}
```

- `interrupt` - marks the function as an ISR; the compiler generates `RETFIE` instead of `RETURN` and saves/restores the necessary registers.
- `no_auto_psv` - disables automatic switching of the Program Space Visibility window. Required here because no program-space constants are accessed inside the ISR; omitting it adds unnecessary overhead on every ISR entry.
- The function name (e.g., `_T1Interrupt`) is the **fixed vector name** defined in the device linker script - it cannot be changed.

---

## Part A - Timer Interrupt (as-1.X)

### Goal
Blink two LEDs at different frequencies - LED1 at 0.5 Hz (2 s period) driven by the main loop, LED2 at 1 Hz (1 s period) driven by a **Timer1 ISR** - without either task blocking the other.

### Design

```
Main loop                     Timer1 ISR (every 1 ms)
──────────────────────        ─────────────────────────
Toggle LED1                   val2++
tmr_wait_ms(TIMER1, 2000)     if val2 >= 1000 → toggle LED2, val2 = 0
(blocks 2 s, but ISR          IFS0bits.T1IF = 0
 still fires every 1 ms)
```

The main loop uses `tmr_wait_ms`, which is a blocking busy-wait. Despite this, LED2 still toggles at 1 Hz because the ISR **preempts** the busy-wait: the `while(IFS0bits.T1IF == 0)` spin in `tmr_wait_period` is regularly interrupted by the Timer1 ISR. The CPU returns to the spin-wait after each ISR exit, but the LED2 logic has already executed.

### volatile keyword

```c
volatile int val2 = 0;
```

`val2` is written in the ISR and read inside the ISR (the `>= 1000` check). The `volatile` qualifier tells the compiler **not to cache this variable in a register** across iterations - without it, the optimiser may assume `val2` can never change inside the spin loop and eliminate the check entirely, causing LED2 to never toggle.

**Rule:** Any variable shared between an ISR and the main execution context must be declared `volatile`.

### Timer1 Configuration for ISR use

```c
IEC0bits.T1IE = 1;   // enable Timer1 interrupt
```

`tmr_setup_period(TIMER1, 1)` inside `tmr_wait_ms` already configures Timer1 for 1 ms periods. Enabling `T1IE` before entering the main loop means every 1 ms period completion fires the ISR. The ISR increments `val2` and toggles LED2 every 1000th call (= 1 second).

---

## Part B - External Interrupt with Debouncing (as-2.X)

### Goal
Toggle LED2 each time Button 1 is pressed, with reliable debounce handling for mechanical contact bounce.

### Peripheral Pin Select (PPS)

The dsPIC33EP uses a **remappable pin system**: most digital peripherals (UART, SPI, external interrupts, etc.) are not hardwired to specific pins and must be mapped at runtime via the `RPINR` registers.

```c
RPINR0bits.INT1R = 0x58;   // map INT1 to RP88 = RE8 (Button 1)
```

`0x58` = 88 decimal, which is the **Remappable Pin Number (RPN)** for `RE8` on this device. This connects the INT1 edge-detector to the physical button pin. Without this, INT1 would have no source and never fire.

```c
INTCON2bits.GIE = 1;    // enable global interrupts
IFS1bits.INT1IF = 0;    // clear flag before enabling (avoid spurious trigger)
IEC1bits.INT1IE = 1;    // arm INT1
```

### Software Debouncing via Timestamp Comparison

Mechanical buttons do not produce a clean single transition. The contact bounces for 5-20 ms, generating dozens of spurious edges. A naive ISR would toggle the LED on every bounce.

**Strategy:** Record the system time of the last accepted press; reject any new trigger that arrives within a **50 ms guard window**.

```c
volatile uint32_t last_press  = 0;
volatile uint32_t system_time = 0;

// Timer1 ISR - increments system_time every 1 ms
void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void) {
    system_time++;
    IFS0bits.T1IF = 0;
}

// INT1 ISR - fires on button edge
void __attribute__((interrupt, no_auto_psv)) _INT1Interrupt(void) {
    if ((system_time - last_press) > 50) {   // ignore bounces < 50 ms apart
        LATGbits.LATG9 = !(LATGbits.LATG9);
        last_press = system_time;
    }
    IFS1bits.INT1IF = 0;
}
```

The 50 ms window is chosen to safely cover the worst-case bounce duration of common tactile switches while still feeling instantaneous to a human (~50 ms is imperceptible).

### Why Two ISRs Instead of One?

| Timer1 ISR | INT1 ISR |
|-----------|---------|
| Fires every 1 ms (periodic, predictable) | Fires on button edge (aperiodic, asynchronous) |
| Maintains `system_time` - the timebase | Uses `system_time` - the consumer |
| Lightweight: one increment + flag clear | Conditional toggle + timestamp update |

Keeping the timebase in a dedicated ISR decouples it from the button logic. If the debounce threshold needed changing, only the `INT1` ISR is modified.

### uint32_t for Timestamps

```c
volatile uint32_t system_time = 0;
```

`uint32_t` holds values up to 4,294,967,295. At 1 increment/ms, this overflows after ~49.7 days. The subtraction `(system_time - last_press)` is correct even across an overflow because unsigned integer subtraction wraps around consistently - a standard technique for timer comparisons in embedded systems.

---

## Interrupt-Driven vs Polling - Summary

| Attribute | Polling | Interrupt-Driven |
|-----------|---------|-----------------|
| CPU usage | 100% (spin-wait) | Idle between events |
| Latency | Up to one full loop iteration | Deterministic (priority-based) |
| Complexity | Simple, linear flow | Requires volatile, flag management |
| Suitability | Simple, single-task sketches | Multi-task, real-time, power-sensitive |

---

## Build & Flash

1. Open `as-1.X` or `as-2.X` in [**MPLAB X IDE**](https://www.microchip.com/en-us/tools-resources/develop/mplab-x-ide).
2. Add `my-libraries/` to: *Project Properties → XC16 → Include directories*.
3. Add `timer_config.c` as a source file in the project.
4. Build (`F11`) and upload the program via [mikroBootloader](https://www.mikroe.com/mikrobootloader).

---

## Key Takeaways

- ISRs must always clear their interrupt flag - omitting this locks the CPU in an infinite ISR re-entry loop.
- `no_auto_psv` reduces ISR overhead when no program-space constants are accessed inside the handler.
- Every variable shared between an ISR and the main context must be `volatile`.
- PPS (Peripheral Pin Select) must be configured before enabling the peripheral - it is not automatically connected.
- Timestamp-based debouncing is non-blocking and integrates naturally into an interrupt-driven architecture.
- `uint32_t` subtraction wraps correctly across overflow - no special handling needed for long uptime.
