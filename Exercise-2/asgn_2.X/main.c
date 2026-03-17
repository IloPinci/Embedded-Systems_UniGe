/*
 * File:   main.c
 * Author: boli
 *
 * Created on March 11, 2026, 12:42 AM
 */


#include "xc.h"
#include "timer_config.h"

int main(void) {
    // initialization code
    ANSELEbits.ANSE8 = 0;
    TRISAbits.TRISA0 = 0;  
    LATAbits.LATA0 = 0;
    int val = 0;
    
    while(1){
        // the code to blink the LED
        val = (val+1) % 2;
        LATAbits.LATA0 = val; 
        
        if(val == 1){
            tmr_wait_ms(TIMER1, 1000);
        }
        else if(val == 0){
            tmr_wait_ms(TIMER1, 1000);
        }
    }
    
    return 0;
}
