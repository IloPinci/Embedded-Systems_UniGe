/*
 * File:   main.c
 * Author: boli
 *
 * Created on March 17, 2026, 8:16 PM
 */


#include "xc.h"
#include "timer_config.h"

volatile int val2 = 0;


void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void){
    val2++;
    
    // here we check if it is 250 bc in our timer we count up to 1ms 250 times
    // therefore timer1 interrupt is executed every 1ms
    if (val2 >= 1000){
        LATGbits.LATG9 = !(LATGbits.LATG9); // we change the value of the led
        val2 = 0;
    }   
    IFS0bits.T1IF = 0;      // we reset the flag
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
    
    // Lets allow the interrupt
    IEC0bits.T1IE = 1;
    
    
    
    while(1){
        
        LATAbits.LATA0 = !(LATAbits.LATA0);
        
        // we call the timer for the  first led. the second one will be handled by the isr
        tmr_wait_ms(TIMER1, 2000);            
    }
    
    return 0;
}
