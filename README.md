# Assignment 4 - UART Communication on dsPIC33EP512MU810

> **Course:** Embedded Systems  
> **MCU:** Microchip dsPIC33EP512MU810  
> **Toolchain:** MPLAB X IDE · XC16 Compiler  
> **Language:** C (bare-metal, no RTOS) 
---

## Overview

This assignment introduces **UART (Universal Asynchronous Receiver-Transmitter)** serial communication. Two exercises progress from a minimal polling-based echo server to a full interrupt-driven system featuring: circular ring buffers, a custom ASCII command protocol, SPI sensor integration, and a cyclic executive scheduler with period-miss detection.

The core design challenge is handling **concurrent I/O**: the CPU must read an accelerometer over SPI on a 20 ms schedule *and* process incoming UART commands *and* respect a 10 ms main loop period - all without dropping data or missing deadlines.

---

## UART Fundamentals

UART is an **asynchronous** protocol: there is no shared clock line between sender and receiver. Instead, both ends agree on a **baud rate** (symbols per second) in advance. Each frame consists of:

```
[IDLE=1] [START=0] [D0] [D1] [D2] [D3] [D4] [D5] [D6] [D7] [STOP=1] [IDLE=1]
```

Because there is no clock, the receiver samples each bit at the centre of its expected window. If sender and receiver baud rates differ by more than ~3-4%, the sampling point drifts into the wrong bit by the last data bit, causing framing errors.

---

## Baud Rate Configuration

The dsPIC33EP UART uses the following formula in **standard speed mode** (`BRGH = 0`, 16x oversampling):

```
BRG = Fcy / (16 × BaudRate) - 1
BRG = 72,000,000 / (16 × 9600) - 1 = 468.75 - 1 → 468  (integer truncation)
```

Resulting actual baud rate:

```
ActualBaud = 72,000,000 / (16 × (468 + 1)) = 72,000,000 / 7,504 ≈ 9,595.7 baud
Error ≈ -0.04%  ✓  (well within the ±3% tolerance)
```

`BRGH = 0` selects 16x oversampling, which provides better noise immunity than the 4x high-speed mode at the cost of a lower maximum baud rate.

---

## Pin Remapping (PPS)

UART1 is not hardwired to specific pins; it must be connected via the **Peripheral Pin Select** system:

```c
// RD11 (RPN = 75) → UART1 RX input
RPINR18bits.U1RXR = 75;

// RD0 (RP64) → UART1 TX output (function code 0x01 = U1TX)
RPOR0bits.RP64R = 0x01;
```

`RPINR18` maps *input* functions to a physical pin number. `RPOR0` maps a physical pin to an *output* function. The asymmetry - input registers keyed by peripheral, output registers keyed by pin - is intentional: it prevents two peripherals from accidentally driving the same pin.

---

## Exercise 1 - Polling Echo (ex-1_4.X)

The simplest possible UART loop: wait for a received byte, echo it back.

```c
while(1){
    while(U1STAbits.URXDA == 0);   // block until RX data available
    reciv_char = U1RXREG;          // read from hardware buffer (clears URXDA)

    while(U1STAbits.UTXBF == 1);   // block until TX buffer not full
    U1TXREG = reciv_char;          // write to TX hardware register
}
```

**`URXDA`** - *UART Receive Data Available*: set by hardware when at least one byte is in the 4-level RX FIFO. **`UTXBF`** - *UART Transmit Buffer Full*: set when the TX register is occupied. Writing to `U1TXREG` while `UTXBF = 1` silently discards the byte.

**Limitation:** The CPU spins at 100% utilisation. Any computation between the two `while` loops risks missing an incoming byte if the sender transmits back-to-back characters faster than the loop iterates - the 4-level hardware FIFO offers some buffer, but it is not sufficient for sustained throughput.

---

## Exercise 2 - Interrupt-Driven Architecture (ex2_4.X)

### Circular Ring Buffers

The fundamental fix for the polling limitation is to **decouple data arrival from data processing** using software ring buffers:

```c
#define BUF_SIZE 16

volatile char receive_buffer[BUF_SIZE];
volatile int  receive_head = 0;   // ISR writes here
volatile int  receive_tail = 0;   // main loop reads here
```

The buffer is empty when `head == tail`. It is full when `(head + 1) % BUF_SIZE == tail` (one slot is intentionally sacrificed to distinguish full from empty without a separate counter).

The **RX ISR** drains the hardware FIFO into the ring buffer immediately on arrival:

```c
void __attribute__((interrupt, no_auto_psv)) _U1RXInterrupt(void) {
    IFS0bits.U1RXIF = 0;
    while (U1STAbits.URXDA == 1) {      // drain all bytes in FIFO
        char value = U1RXREG;
        int next = (receive_head + 1) % BUF_SIZE;
        if (next != receive_tail) {      // only write if not full (drop if full)
            receive_buffer[receive_head] = value;
            receive_head = next;
        }
    }
}
```

The inner `while` is important: a single `U1RXInterrupt` can fire when the FIFO has multiple bytes. Reading only once would leave data unread and potentially delay the next interrupt.

The **TX ISR** feeds from the transmit ring buffer one byte at a time, disabling itself when the buffer empties:

```c
void __attribute__((interrupt, no_auto_psv)) _U1TXInterrupt(void) {
    IFS0bits.U1TXIF = 0;
    if (transmit_tail != transmit_head) {
        U1TXREG = transmit_buffer[transmit_tail];
        transmit_tail = (transmit_tail + 1) % BUF_SIZE;
    } else {
        IEC0bits.U1TXIE = 0;   // nothing left - disable self
    }
}
```

This **self-disabling ISR** pattern avoids the CPU re-entering the TX ISR on every instruction when the buffer is empty. To send a new message, the main loop writes to the transmit buffer and re-enables `U1TXIE`, which immediately fires the ISR to kick off transmission.

---

### Cyclic Executive with Period Monitoring

The main loop follows a **cyclic executive** structure around a 10 ms tick:

```c
tmr_setup_period(TIMER1, 10);   // 10 ms base period

while(1){
    algorithm();                // fixed computation (~7 ms budget)

    // ... sensor reads, command parsing ...

    if (!tmr_wait_period(TIMER1)) {   // 0 = on-time, 1 = period missed
        period_misses++;
    }
}
```

`tmr_wait_period` returns `1` (early exit) if the flag was already set when called - meaning the computation took longer than 10 ms and the deadline was missed. Counting misses is a basic form of **real-time health monitoring**. An LED blinks every 500 ms (50 × 10 ms iterations) as a watchdog indicator.

---

### Command Protocol - Sliding Window Parser

Incoming UART commands follow a fixed 7-character ASCII frame:

```
$ C C , D D *
│ │ │ │ │ │ └── terminator
│ │ │ │ └─┴── two-digit decimal value
│ │ │ └── separator
│ └─┴── command code: 'HZ' (frequency) or 'BW' (bandwidth)
└── start marker
```

Examples: `$HZ,10*` sets frequency to 10 Hz. `$BW,15*` sets accelerometer bandwidth to filter index 15.

The parser uses a **7-element sliding window** over the receive buffer:

```c
// Shift window
reciv_char[0..5] = reciv_char[1..6];
reciv_char[6] = tempChar;

// Validate all positions simultaneously
if (reciv_char[0] == '$' && reciv_char[3] == ',' && reciv_char[6] == '*' && ...)
```

This approach avoids a state machine entirely: every received character triggers a full structural check on the last 7 bytes. It is O(1) per character and has no partial-match state to corrupt. The trade-off is that it cannot handle variable-length frames.

> **Initialisation hazard:** `reciv_char[7]` is declared but not zero-initialised. Garbage values at startup could match the protocol pattern and trigger a spurious command before any real data arrives. `memset(reciv_char, 0, sizeof(reciv_char))` should be called before the main loop.

---

### Accelerometer Bandwidth Management

`accel_bw(int bw)` configures the accelerometer's digital low-pass filter over SPI and then schedules a **one-shot Timer3 interrupt** to signal when the filter has settled:

```c
tmr_setup_period(TIMER3, delay_ms);
IFS0bits.T3IF = 0;
IEC0bits.T3IE = 1;       // arm once
```

The Timer3 ISR sets `accel_read_flag = 1` and immediately **disarms itself** (`IEC0bits.T3IE = 0`). The main loop only reads accelerometer data when this flag is set, preventing reads during filter settling time. This is a clean use of a one-shot interrupt for deferred readiness signalling.

---

## Build & Flash

1. Open `ex-1_4.X` or `ex2_4.X` in [**MPLAB X IDE**](https://www.microchip.com/en-us/tools-resources/develop/mplab-x-ide).
2. Add `my-libraries/` to: *Project Properties → XC16 → Include directories*.
3. Add `timer_config.c` as a source file.
4. Build (`F11`) and upload the program via [mikroBootloader](https://www.mikroe.com/mikrobootloader).
5. Use a serial terminal at **9600 8N1** (e.g. MPLAB Data Visualizer, PuTTY, or `minicom`) on the assigned COM port.

---

## Key Takeaways

- Baud rate error below ±3% is required; document the calculated actual rate vs target.
- PPS requires separate `RPINR` (input) and `RPOR` (output) register writes - direction matters.
- Polling UART is sufficient for very low data rates; interrupt-driven ring buffers are necessary for any non-trivial throughput.
- The TX ISR self-disable pattern avoids busy-loop ISR re-entry on an empty buffer.
- Sliding window parsers are a simple, robust approach for fixed-length framed protocols.
- Local arrays must never be returned by value; pass a caller-owned buffer instead.
