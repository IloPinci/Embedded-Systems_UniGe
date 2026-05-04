/* 
 * File:   main_a1.c
 * Author: JT
 *
 * Created on April 26, 2026, 7:01 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include "xc.h"
#include "timer_config.h"
#include <math.h>


#define BUF_SIZE 64 
#define PI 3.14159265358979323846

volatile char receive_buffer[BUF_SIZE];
volatile int receive_head = 0, receive_tail = 0;

volatile char transmit_buffer[BUF_SIZE];
volatile int transmit_head = 0, transmit_tail = 0;

volatile int accel_read_flag = 0;
volatile int accel_count = 0;
volatile int char_index  = 0;

volatile int timer_count = 0;
volatile int ang_count = 0;

volatile int hz_period = 10;  
volatile int hz_count  = 0;


typedef struct{
    int16_t axis_x;
    int16_t axis_y;
    int16_t axis_z;

    float roll;
    float pitch;
} EulerAngles;

// we change the read flag allowing for the reading of the accel values
void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void) {
    accel_read_flag = 1;
    IFS0bits.T3IF = 0; 
    IEC0bits.T3IE = 0;          // also we don't allow interrupts unless the accel_bw is called again
}

// This is for reading from the uart
void __attribute__((interrupt, no_auto_psv)) _U1RXInterrupt(void) {
    IFS0bits.U1RXIF = 0;
    
    while (U1STAbits.URXDA == 1) {
        char value = U1RXREG;
        
        int next = (receive_head + 1) % BUF_SIZE;  // '%' is used to wrap around
        if (next != receive_tail) {
            receive_buffer[receive_head] = value;
            receive_head = next;
        }   
    }
}

// to transmit 
void __attribute__((interrupt, no_auto_psv)) _U1TXInterrupt(void) {
    IFS0bits.U1TXIF = 0;

    if (transmit_tail != transmit_head) {
        U1TXREG = transmit_buffer[transmit_tail];
        transmit_tail = (transmit_tail + 1) % BUF_SIZE;
    } else {
        IEC0bits.U1TXIE = 0;       // nothing left so we disable interrupt
    }
}


void initial_setup(){
    // disable the analog modality
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;
    
    // timers
    tmr_setup_period(TIMER1, 10);
    tmr_setup_period(TIMER2, 10);
    
    // LED2
    TRISGbits.TRISG9 = 0;       // output
    LATGbits.LATG9 = 1;         // initial value
}

void algorithm(){
    tmr_wait_ms(TIMER2, 7);
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
    IFS0bits.U1RXIF = 0;    // interrupt flag
    IEC0bits.U1RXIE = 1;    // interrupt enable
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
    // now we deactivate them bc pulling them up deactivates them
    LATDbits.LATD6 = 1;         // mag
    LATBbits.LATB4 = 1;         // gyro
    LATBbits.LATB3 = 1;         // accel
    
}

unsigned int spi_write(unsigned int data){
    while (SPI1STATbits.SPITBF == 1);    // wait until the t-buffer is emptied 
    // ^ can be occupied by previous iterations so we wait until it is cleared
    
    SPI1BUF = data;                      // write data (also triggers the clock)
    
    while (SPI1STATbits.SPIRBF == 0);    // wait until the whole transmission is complete
    // ^ ensures that the whole word is sent
    
    return SPI1BUF;     // return the data we got from the slave
}

void uart_transmit(const char* message){
    while (*message) {
        int next = (transmit_head + 1) % BUF_SIZE;
        while (next == transmit_tail);  // wait if buffer full
        transmit_buffer[transmit_head] = *message++;
        transmit_head = next;
    }
    // we manually send the first byte to kick off the interrupt chain
    if (transmit_tail != transmit_head) {
        IEC0bits.U1TXIE = 0;                // disable interrupt briefly
        U1TXREG = transmit_buffer[transmit_tail];
        transmit_tail = (transmit_tail + 1) % BUF_SIZE;
        IEC0bits.U1TXIE = 1;                // we enable the uart so the rest is sent by interrupt
    }
}

// for the badwidth of the filter
void accel_bw(int bw){
    
    int delay_ms = 0;
    accel_read_flag = 0;
    
    // it boots in normal mode the moment it is selected
    LATBbits.LATB3 = 0;         // we activate the chip
    spi_write(0x10 & 0x7F);     // the register to configure the bandwidth
    
    switch(bw) {
    // each filtering frequency requires a different time to update before new data can be considered
        case 8:  spi_write(0x08); delay_ms = 64; break;
        case 9:  spi_write(0x09); delay_ms = 32; break;
        case 10: spi_write(0x0A); delay_ms = 16; break;
        case 11: spi_write(0x0B); delay_ms = 8; break;
        case 12: spi_write(0x0C); delay_ms = 4; break;
        case 13: spi_write(0x0D); delay_ms = 2; break;
        case 14: spi_write(0x0E); delay_ms = 1; break;
        case 15: spi_write(0x0F); delay_ms = 1; break;
        default: 
            // if a wrong value is sent we send to uart and return 
            LATBbits.LATB3 = 1;
            uart_transmit("$ERR,1*");
            return;
    }
    LATBbits.LATB3 = 1;  
    
    tmr_setup_period(TIMER3, delay_ms);
    IFS0bits.T3IF = 0;          // we ensure that the flag is clean before we enable the interrupt
    IEC0bits.T3IE = 1;          // we enable the interrupt   
}


// we change the transmit frequency of the data by determining a value that can be used
// inside the main loop 
void uart_frequency_change(int value){

    switch(value) {
        case 0:  hz_period = 0;   break;
        case 1:  hz_period = 100; break;    // 1000ms
        case 2:  hz_period = 50;  break;    // 500ms
        case 5:  hz_period = 20;  break;    // 200ms
        case 10: hz_period = 10;  break;    // 100ms
        default:
            uart_transmit("$ERR,2*");
            break;
    }    
}

// we get the axes at 50Hz and we calculate the roll/pitch. the latter even though 
EulerAngles accel_axis(){
    EulerAngles result;

    uint16_t LSB_part, MSB_part;
    

    // x-axis starts at 0x42 ends at 0x43
    LATBbits.LATB3 = 0;
    spi_write(0x02 | 0x80); 
    LSB_part = spi_write(0x00);
    MSB_part = spi_write(0x00);         // it is auto incremented     

    result.axis_x = ((int16_t)((MSB_part << 8) | LSB_part)) >> 4;

    // y-axis starts at 0x44 ends at 0x45
    LSB_part = spi_write(0x00);
    MSB_part = spi_write(0x00);           
    result.axis_y = ((int16_t)((MSB_part << 8) | LSB_part)) >> 4;

    // z-axis starts at 0x46 ends at 0x47
    LSB_part = spi_write(0x00);
    MSB_part = spi_write(0x00);         
    result.axis_z = ((int16_t)((MSB_part << 8) | LSB_part)) >> 4;   // z-axis has 15 bits
    LATBbits.LATB3 = 1;
    
    double ax, ay, az;
    
    ax = (double) result.axis_x;
    ay = (double) result.axis_y;
    az = (double) result.axis_z;
    
    // angle calculation
    result.roll = atan2(ay, az) * (180.0 / PI);  // roll in degrees
    result.pitch = atan2(-ax, sqrt((ay * ay) + (az * az))) * (180.0 / PI); //pitch
    
    return result;
}


int main() {
    int period_misses = 0, value = 0;

    char reciv_char[7] = {0, 0, 0, 0, 0, 0, 0};;
    char buffer_euler[48];

    EulerAngles euler_angles;

    initial_setup();
    uart_setup();
    spi_setup();

    accel_bw(15);
    
    
    while(1){
        algorithm();

        // we have to get the accelerometer data every 20ms (50Hz) so every two iterations
        if (++accel_count >= 2){    
            accel_count = 0;
            if (accel_read_flag == 1){          // we make sure that the data is filtered correctly
                euler_angles =  accel_axis();
            }
        }

        // send the angle axes (also every ms value of the frequencies are a multiple of 20ms)
        if (hz_period > 0 && ++hz_count >= hz_period) {   // +2 bc one main loop is 10ms but we enter the daq every 20ms
            hz_count = 0;
            sprintf(buffer_euler, "$ACC,%d,%d,%d*",
            (int)euler_angles.axis_x,
            (int)euler_angles.axis_y,
            (int)euler_angles.axis_z);

            uart_transmit(buffer_euler);
            // if the filter is still being normalized we just post the previous values
        }

        // for the angles which are transmitted a fixed 5Hz
        if (++ang_count >= 20) {
            ang_count = 0;
            sprintf(buffer_euler, "$ANG,%.1f,%.1f*", (double) euler_angles.roll, (double) euler_angles.pitch);
            uart_transmit(buffer_euler);

            
            // to check the number of missed periods
//            char dbg[32];
//            sprintf(dbg, "$DBG,%d*", period_misses);
//            uart_transmit(dbg);
        }
        
        while (receive_tail != receive_head){
           char tempChar = receive_buffer[receive_tail];
           receive_tail = (receive_tail+1) % BUF_SIZE;
           
           reciv_char[0] = reciv_char[1];
           reciv_char[1] = reciv_char[2];
           reciv_char[2] = reciv_char[3];
           reciv_char[3] = reciv_char[4];
           reciv_char[4] = reciv_char[5];
           reciv_char[5] = reciv_char[6];
           reciv_char[6] = tempChar;
           
            if (char_index  < 7){
                char_index ++;
            }
           
           if (char_index  == 7){          
                if (reciv_char[0] == '$' && reciv_char[3] == ',' && reciv_char[6] == '*' &&
                   ((reciv_char[1] == 'H' &&  reciv_char[2] == 'Z') ||
                   (reciv_char[1] == 'B' && reciv_char[2] == 'W')) &&
                   (reciv_char[4] >= '0' && reciv_char[4] <= '9') &&
                   (reciv_char[5] >= '0' && reciv_char[5] <= '9')){ 
                    
                    value = (reciv_char[4] - '0') * 10 + (reciv_char[5] - '0'); // we turn chars into numbers
                    // FOR UART DATA FREQUENCY
                    if (reciv_char[1] == 'H' &&  reciv_char[2] == 'Z'){
                        uart_frequency_change(value);
                    }
                    // FOR BANDWIDTH FILTER CONTROL
                    else{
                       accel_bw(value);
                    }
                    reciv_char[0] = reciv_char[1] = reciv_char[2] = reciv_char[3] = 0;
                    reciv_char[4] = reciv_char[5] = reciv_char[6] = 0;
                    char_index = 0;
                }
            } 
       }               
             
        
        if(tmr_wait_period(TIMER1)){
            period_misses++;
        }
        // Led
        if (++timer_count == 50){
            LATGbits.LATG9 = !LATGbits.LATG9;
            timer_count = 0;
        }
    }
    return (EXIT_SUCCESS);
}

