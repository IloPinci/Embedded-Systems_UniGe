/*
 * File:   6_1main.c
 * Author: boli
 *
 * Created on May 13, 2026, 2:18 PM
 */


#include <stdio.h>
#include <stdlib.h>
#include "xc.h"
#include "uart.h"
#include "timer_config.h"
#include "adc.h"
#include "spi.h"


// setup
void initial_setup(){
    // Disable analog inputs 
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;
    
    TRISBbits.TRISB11 = 1;      //define as input
    ANSELBbits.ANSB11 = 1;      // set AN11 -> RB11 battery pin to analog (as we want to read voltages)
    
    TRISBbits.TRISB4 = 0;       // set as output
    TRISBbits.TRISB5 = 1;       // set as input
    LATBbits.LATB4 = 1;         // enable
    ANSELBbits.ANSB5 = 1;      // set AN11 -> RB11 battery pin to analog (as we want to read voltages)
    
    // onetime setup
    uart_setup();     // configurable baudrate with adjusted buffersizes
    spi_setup();
    
    
     // 1. Part
//    adc_setup(BIT10, MANUAL, MANUAL, 11);
     
    
    
    // 2. Part
//    adc_setup(BIT10, MANUAL, AUTO, 5);
    
    
    /* 3. Part */
    adc_scan_setup(BIT10);
}

int main(void) {
    
    tmr_wait_ms(TIMER1, 500);       // wait for capacitor to charge
    
    initial_setup();
    
    float read_raw_bat;
    float bat_Vsense;
    float battery_V;
    
    float raw_ir_data;
    float voltage;
    float result;
    
    int counter = 0;                   // count for hz
    
    char buffer[48];                 // TODO idk what size
    
    while(1){
        
    
        //1. Part of assignment 
         
//        read_raw_bat = adc_read(MANUAL, MANUAL);
//        bat_Vsense = 3.3 * read_raw_bat / 1023.0;
//        battery_V = 3 * bat_Vsense;
//        
//        sprintf(buffer, "$SENS,%.2f*", battery_V);
//        
//        uart_transmit(buffer);
         
         
        
        
        //2. Part
         
//        raw_ir_data = adc_read(MANUAL, AUTO);
//        
//        voltage = 3.3 * raw_ir_data / 1023.0;
//        
//        result = 2.34f
//                 - 4.74f * voltage
//                 + 4.06f * (voltage * voltage)
//                 - 1.60f * (voltage * voltage * voltage)
//                 + 0.24f * (voltage * voltage * voltage * voltage);
//        
//        sprintf(buffer, "$IR,%.2f*", result);
//        
//        uart_transmit(buffer);

        
        
        /* 3. Part */
        
        // 1 kHz acquisition wait 1ms then read latest scan result
        tmr_wait_ms(TIMER1, 1);
        adc_scan_read(&raw_ir_data, &read_raw_bat);
        
        counter++;
        
        // 10 Hz -> every 100ms
        if(counter >= 100){
            // convert ir data
            voltage = 3.3 * raw_ir_data / 1023.0;
        
            result = 2.34f
                     - 4.74f * voltage
                     + 4.06f * voltage * voltage
                     - 1.60f * voltage * voltage * voltage
                     + 0.24f * voltage * voltage * voltage * voltage;

            // convert battery
            bat_Vsense = 3.3 * read_raw_bat / 1023.0;
            battery_V = 3 * bat_Vsense;
            
            // transform string and send over uart
             sprintf(buffer, "$SENS,%.2f,%.2f*", result, battery_V);
            uart_transmit(buffer);
        }
        
        tmr_wait_ms(TIMER1, 100);
    }
}