# Assignment 5 - SPI Communication on dsPIC33EP512MU810

> **Course:** Embedded Systems  
> **MCU:** Microchip dsPIC33EP512MU810  
> **Toolchain:** MPLAB X IDE · XC16 Compiler  
> **Language:** C (bare-metal, no RTOS) 
---

## Overview

This assignment introduces **SPI (Serial Peripheral Interface)**, a synchronous full-duplex bus used to communicate with external sensors. The target device is a **BMM150 3-axis Magnetometer** from Bosch. Two exercises progress from a basic device power-up and chip ID verification, to a continuous X-axis magnetometer data stream formatted as a human-readable NMEA-like ASCII message.

---

## SPI Protocol Fundamentals

SPI uses **four signals**:

| Signal | Direction | Function |
|--------|-----------|----------|
| `SCK` | Master → Slave | Shared clock - both sides sample on the same edge |
| `MOSI` | Master → Slave | Master Out Slave In - command/data from MCU |
| `MISO` | Slave → Master | Master In Slave Out - response from sensor |
| `CS` (active-LOW) | Master → Slave | Chip Select - frames a transaction; slave ignores bus when CS = HIGH |

Unlike UART, SPI is **synchronous**: the master generates the clock, so there is no baud rate error. A full-duplex transaction is always simultaneous: while the master clocks out a byte on MOSI, the slave simultaneously clocks back a byte on MISO. For register reads, the first byte (the command) produces a dummy response on MISO; the useful data arrives on the following byte(s).

---

## SPI Clock Mode

SPI has four operating modes defined by two parameters:

| Parameter | Meaning | This project |
|-----------|---------|-------------|
| `CKP` (Clock Polarity) | Idle state of SCK | `CKP = 1` → idle HIGH |
| `CKE` (Clock Edge) | Which edge clocks data | default (`CKE = 1`) → data sampled on falling edge |

Together, `CKP=1, CKE=1` corresponds to **SPI Mode 3** (CPOL=1, CPHA=1 in standard notation). The BMM150 datasheet specifies support for SPI Mode 0 and Mode 3, so Mode 3 is valid here.

---

## SPI Clock Rate

The SPI clock is derived from `Fcy` through two cascaded prescalers:

```
SCK = Fcy / (PrimaryPrescaler × SecondaryPrescaler)
```

Register values set in this project:

| Field | Value | Prescaler |
|-------|-------|-----------|
| `PPRE` = 3 (0b11) | Primary | 1:1 |
| `SPRE` = 3 (0b011) | Secondary | 5:1 |

```
SCK = 72 MHz / (1 × 5) = 14.4 MHz
```

> **Clock speed concern:** The BMM150 datasheet specifies a maximum SPI clock of **10 MHz**. At 14.4 MHz, the device is operated out of specification. Setting `PPRE = 0b10` (4:1 primary) and `SPRE = 0b110` (2:1 secondary) yields 9 MHz, safely within spec:
> ```
> SCK = 72 MHz / (4 × 2) = 9 MHz ✓
> ```
> The current prescaler values should be corrected before any production use.

---

## BMM150 Magnetometer

The BMM150 is a 3-axis Hall-effect magnetometer from Bosch. It communicates over SPI using a **1-byte address frame** where the MSB selects read (`1`) or write (`0`), and the lower 7 bits carry the register address:

```
Write frame:  CS↓  [0 | ADDR[6:0]]  [DATA]  CS↑
Read frame:   CS↓  [1 | ADDR[6:0]]  [0x00]  CS↑
                                      ↑ dummy byte; response arrives here on MISO
```

### Power-Up Sequence

The BMM150 starts in **suspend mode** (lowest power, no oscillator running). It cannot transition directly to normal operation - it must pass through **sleep mode** first:

```
Suspend → Sleep → Normal
```

```c
// Step 1: Suspend → Sleep (set power control bit in register 0x4B)
LATDbits.LATD6 = 0;
spi_write(0x4B & 0x7F);   // 0x7F clears MSB → write command
spi_write(0x01);           // bit0 = 1 → exit suspend
LATDbits.LATD6 = 1;
tmr_wait_ms(TIMER1, 3);    // wait ≥ 3 ms for oscillator to stabilise

// Step 2: Sleep → Normal (operation mode bits in register 0x4C)
LATDbits.LATD6 = 0;
spi_write(0x4C & 0x7F);   // write to OpMode register
spi_write(0x00);           // bits[2:1] = 00 → Normal mode
LATDbits.LATD6 = 1;
tmr_wait_ms(TIMER1, 3);
```

The 3 ms delays are required by the BMM150 datasheet; the oscillator and internal references need time to settle before data registers contain valid readings.

---

## Exercise 1 - Chip ID Verification (5_1ex.X)

After power-up, read register `0x40` (chip ID) and transmit the raw byte over UART:

```c
LATDbits.LATD6 = 0;
spi_write(0x40 | 0x80);                  // 0x80 sets MSB → read command
unsigned int chipID = spi_write(0x00);   // dummy byte; response in MISO
LATDbits.LATD6 = 1;

while (U1STAbits.UTXBF == 1);
U1TXREG = chipID;
```

The expected chip ID for BMM150 is **0x32**. Verifying it before reading data registers is good practice: a wrong ID (or `0x00` / `0xFF`) indicates a wiring fault, PPS misconfiguration, or wrong SPI mode - all of which are diagnosed before investing effort in data interpretation.

> **Note:** In Exercise 1 the full power-up sequence runs inside `while(1)`, resetting the chip on every iteration. This is functional for a single chip ID check but would be incorrect for a data acquisition loop. Exercise 2 correctly moves the initialisation sequence outside the main loop.

---

## Exercise 2 - X-Axis Magnetometer Streaming (5_2ex.X)

### Multi-Byte Burst Read

The BMM150 supports **address auto-increment** in SPI burst mode: asserting CS once and issuing a read command at the starting register, then clocking out additional dummy bytes advances the internal pointer automatically. The X-axis value spans two registers:

| Register | Content |
|----------|---------|
| `0x42` | X data LSB (bits[7:3] are data, bits[2:0] are status flags) |
| `0x43` | X data MSB (bits[7:0] are data) |

```c
LATDbits.LATD6 = 0;
spi_write(0x42 | 0x80);           // read starting at 0x42
LSB_part = spi_write(0x00);       // receives 0x42
MSB_part = spi_write(0x00);       // auto-increments to 0x43
LATDbits.LATD6 = 1;
```

### Bit-Field Reconstruction

The raw X-axis value is a **13-bit two's complement** integer packed across two bytes:

```c
LSB_part = LSB_part & 0xF8;       // mask off the 3 status bits [2:0]
MSB_part = MSB_part << 8;
result = MSB_part | LSB_part;
result = result >> 3;             // arithmetic right-shift to sign-extend
```

The right-shift by 3 extracts the 13 payload bits. Because `result` is declared as `signed int`, the compiler produces an **arithmetic shift** (sign-extending the MSB), correctly preserving negative values in two's complement.

> **Why `signed int` and not `int16_t`?** On dsPIC33, `int` is 16 bits, so `signed int` and `int16_t` are equivalent here. However, using `int16_t` from `<stdint.h>` makes the width intent explicit and is preferred for portability.

### NMEA-Like Output Format

The result is formatted as a human-readable ASCII string and sent over UART:

```c
sprintf(buff, "$MAGX=%d*", result);
```

This mimics the **NMEA 0183** framing convention (`$` start, `*` end) familiar from GPS receivers. It makes the output parseable by any terminal or PC-side script without a custom binary parser. The formatted string is transmitted byte-by-byte with a `UTXBF` check - the polling TX used here would benefit from the interrupt-driven transmit buffer from Assignment 4 for higher data rates.

### Timing

```c
tmr_setup_period(TIMER2, 100);   // 100 ms period

while(1){
    // ... read and transmit ...
    tmr_wait_period(TIMER2);     // pace to 10 Hz
}
```

The magnetometer is read and transmitted at 10 Hz (one sample every 100 ms). `tmr_wait_period` returns early with `1` if the SPI + UART operations took longer than 100 ms, signalling a deadline miss.

---

## Chip Select Management

Three separate sensors share the SPI bus, each with an independent active-LOW chip select:

| Sensor | CS Pin | Default State |
|--------|--------|--------------|
| Accelerometer | `RB3` | HIGH (deactivated) |
| Gyroscope | `RB4` | HIGH (deactivated) |
| Magnetometer | `RD6` | HIGH (deactivated) |

All CS lines are initialised HIGH at startup. Before any transaction, exactly one CS is asserted LOW; it is deasserted HIGH immediately after the final byte is clocked. Leaving multiple CS lines asserted simultaneously would cause bus contention - two slaves driving MISO concurrently.

---

## `spi_write` Function Design

```c
unsigned int spi_write(unsigned int data) {
    while (SPI1STATbits.SPITBF == 1);   // wait: TX buffer occupied
    SPI1BUF = data;                      // write triggers clock generation
    while (SPI1STATbits.SPIRBF == 0);   // wait: full word not yet received
    return SPI1BUF;                      // reading clears SPIRBF
}
```

The function is blocking and synchronous. Reading `SPI1BUF` at the end is mandatory even for write-only operations: failing to read causes `SPIROV` (receive overflow) to set on the next transaction, which corrupts subsequent reads.

---

## Build & Flash

1. Open `5_1ex.X` or `5_2ex.X` in [**MPLAB X IDE**](https://www.microchip.com/en-us/tools-resources/develop/mplab-x-ide).
2. Add `my-libraries/` to: *Project Properties → XC16 → Include directories*.
3. Add `timer_config.c` as a source file.
4. Build (`F11`) and upload the program via [mikroBootloader](https://www.mikroe.com/mikrobootloader).
5. Open a serial terminal at **9600 8N1** to observe chip ID bytes (Ex. 1) or `$MAGX=...` strings (Ex. 2).

---

## Key Takeaways

- SPI is synchronous and full-duplex; every write is simultaneously a read - always read `SPI1BUF` to prevent overflow.
- `CKP` and `CKE` must match the slave's datasheet (SPI mode); a mismatch produces valid-looking but bit-shifted data.
- Always verify clock speed against the slave's maximum rating; prescaler choices must be documented and justified.
- The BMM150 power-up sequence (suspend → sleep → normal) is mandatory; reading data registers before completing it yields undefined results.
- Multi-byte burst reads reduce CS toggle overhead and are essential when reading multi-register values atomically.
- Bit-field extraction from sensor registers requires careful masking and sign-aware shifting; use `signed` types and arithmetic shift for two's complement values.
- NMEA-like framing (`$...*`) makes debug output immediately interpretable by humans and generic tools alike.
