/*
 * File:   main.c
 * Author: boli
 *
 * Created on March 17, 2026, 10:07 PM
 */


#include "xc.h"
#include "timer_config.h"

volatile uint32_t last_press = 0;
volatile uint32_t system_time = 0;


void __attribute__((interrupt, no_auto_psv)) _INT1Interrupt(void){  
    
    if ((system_time - last_press) > 50){
        LATGbits.LATG9 = !(LATGbits.LATG9); // we change the value of the led
        last_press = system_time;
    }
    
    IFS1bits.INT1IF = 0;      // we reset the flag
}

// to update the system time so the debouncing is done
void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void) {
    system_time++;
    IFS0bits.T1IF = 0; 
}


int main(void) {
    
    ANSELEbits.ANSE8 = 0;   // we disable the analog modality for first button
    ANSELEbits.ANSE9 = 0;   // same but for the second button
    
    //Define that the buttons will be input
    TRISEbits.TRISE8 = 1;  
    TRISEbits.TRISE9 = 1;  
    //Define that the LED will be output
    TRISAbits.TRISA0 = 0;   
    TRISGbits.TRISG9 = 0;
    
    // We define the original value of the LEDs
    LATAbits.LATA0 = 0;     // first LED
    LATGbits.LATG9 = 0;     // second LED
    
    
    // We do the mapping of the button T2(first) to the INT1
    RPINR0bits.INT1R = 0x58;
    INTCON2bits.GIE = 1;    // set global interrupt as enablable
    IFS1bits.INT1IF = 0;    // we clear the interrupt flag
    IEC1bits.INT1IE = 1;    // now we enable the interrupt
    
    IEC0bits.T1IE = 1;  // for the timer
    
    
    while(1){
        
        LATAbits.LATA0 = !(LATAbits.LATA0);
        
        // we call the timer for the  first led. the second one will be handled by the isr
        tmr_wait_ms(TIMER1, 500);            
    }
    
    
    return 0;
}
