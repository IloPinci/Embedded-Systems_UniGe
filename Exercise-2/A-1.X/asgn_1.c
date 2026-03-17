/*
 * File:   asgn_1.c
 * Author: boli
 *
 * Created on March 9, 2026, 5:09 PM
 */


#include "xc.h"
#include "timer_config.h"

int main(void) {
    // initialization code
    ANSELEbits.ANSE8 = 0;
    TRISAbits.TRISA0 = 0;  
    LATAbits.LATA0 = 0;
    int val = 1;
    
    
    tmr_setup_period(TIMER1, 200);
    
    while(1){
        // the code to blink the LED
        val = (val+1) % 2;
        LATAbits.LATA0 = val; 
        
        
        tmr_wait_period(TIMER1);
    }
    
    return 0;
}