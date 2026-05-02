/*
 * File:   5_1main.c
 * Author: boli
 *
 * Created on May 2, 2026, 8:05 PM
 */


#include "xc.h"
#include "timer_config.h"
#include <stdio.h>

void initial_setup(){
    // disable the analog modality
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;
}

void uart_setup(){
    // I/O for Uart
    TRISDbits.TRISD11 = 1;      // in
    TRISDbits.TRISD0 = 0;       // out   
    
    // Pin remapping
    RPINR18bits.U1RXR = 75;         // for input which is supposed to be the RD11 pin        
    RPOR0bits.RP64R = 0x01;         //  for output which is supposed to be the RD0        
    
    // Baud Rate Setup
    U1MODEbits.BRGH = 0;    // to select the 16 divisor
    U1BRG = 468;            // we load it so we can get 9600
    
    // Power on the module
    U1MODEbits.UARTEN = 1;
    U1STAbits.UTXEN = 1;
    
    // Enable transmitter
    IFS0bits.U1RXIF = 0;   
    IEC0bits.U1RXIE = 1;
}

void spi_setup(){
    SPI1STATbits.SPIEN = 0;             // we disable before we do changes (just to be sure)
    
    // I/O setup
    TRISAbits.TRISA1 = 1;               // RA1 corresponds to RPI17 and we set it as input for master-in
    TRISFbits.TRISF12 = 0;              // RF12 corresponds to RP108 SCK and we set it as output for the clock
    TRISFbits.TRISF13 = 0;              // RF13 corresponds to RP109 and we set it as output for master-out
    
    // Pin remapping
    RPINR20bits.SDI1R = 0b0010001;      // MISO (SDI1) - RPI17 [RA1]
    RPOR12bits.RP109R = 0b000101;       // MOSI (SDO1) - RP109 [RF13]  
    RPOR11bits.RP108R = 0b000110;       // SCK1 [RF12] - RP108 [RF12]

    // Modality setup
    SPI1CON1bits.MSTEN = 1;     // enable master
    SPI1CON1bits.MODE16 = 0;    // 8bit modality
    SPI1CON1bits.CKP = 1;       // set the clock idle to 1
    
    // to match the operating frequency of the slave
    SPI1CON1bits.PPRE = 3;       // primary presacler 1:1
    SPI1CON1bits.SPRE = 3;       // secondary prescaler 3:1
    
    SPI1STATbits.SPIROV = 0;    // clear overflow flag 
    SPI1STATbits.SPIEN = 1;     // enable spi  
    
    
    // Chip Select Setup
    
    // define them as output
    TRISDbits.TRISD6 = 0;       // for magnetometer     CS3
    TRISBbits.TRISB4 = 0;       // for gyroscope        CS2
    TRISBbits.TRISB3 = 0;       // for accelerometer    CS1
    // now we deactivate them 
    LATDbits.LATD6 = 1;         // bc pulling them up deactivates them
    LATBbits.LATB4 = 1;
    LATBbits.LATB3 = 1;
    
}

unsigned int spi_write(unsigned int data){
    while (SPI1STATbits.SPITBF == 1);    // wait until the t-buffer is emptied 
    // ^ can be occupied by previous iterations so we wait until it is cleared
    
    SPI1BUF = data;                      // write data (also triggers the clock)
    
    while (SPI1STATbits.SPIRBF == 0);    // wait until the whole transmission is complete
    // ^ ensures that the whole word is sent
    
    return SPI1BUF;     // return the data we got from the slave
}



int main(void) {
    tmr_setup_period(TIMER1, 10);
    
    initial_setup();
    uart_setup();
    spi_setup();
    
    while(1){
        // put to sleep the magnetometer (the power control bit must be set to 1)
        LATDbits.LATD6 = 0;         // we activate the chip
        // we send the command that dictates that we will write to the command register
        // 0x4B -> the address of the command register 
        //0x7F -> forces the MSB to 0 without changing the address of the register
        spi_write(0x4B & 0x7F);     

        spi_write(0x01);        // we write 1 to the control bit of the control register
        LATDbits.LATD6 = 1;      // we deactivate it

        tmr_wait_ms(1, 3);


        // Now that it is in sleep mode we can switch to active mode. (from idle to active cannot be done)
        LATDbits.LATD6 = 0;
        // 0x4C is inaccesible without activating 0x4B
        spi_write(0x4C & 0x7F);
        spi_write(0x00);
        LATDbits.LATD6 = 1;

        tmr_wait_ms(1, 3);

        // now we read the chip id
        LATDbits.LATD6 = 0;
        spi_write(0x40 | 0x80);     // the chip ID is in this register and we force to read from it
        unsigned int chipID = spi_write(0x00);
        LATDbits.LATD6 = 1;

        // send the id to the uart for display
        while (U1STAbits.UTXBF == 1);   // wait until the transmit buffer is empty
        U1TXREG = chipID;  
    }
    
    return 0;
}