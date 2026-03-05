/*
 * File:   main.c
 * Author: JT
 *
 * Created on March 5, 2026, 3:34 PM
 */


#include "xc.h"
#include <stdint.h>

int main(void) {
    // we disable the analog modality
    ANSELEbits.ANSE8 = 0;
    
    // lets define the output (LED)
    TRISAbits.TRISA0 = 0;     
    // define the input (button)
    TRISEbits.TRISE8 = 1;
    
    // give the initial state to our LED
    LATAbits.LATA0 = 1; 
    
    // now we declare some variables
    uint16_t button_initial = 0; // bc the button is on when pressed off when normal
    int val = 1;    // for calc purposes
    

    while(1) {
        uint16_t button_now = PORTEbits.RE8;
        
        if (button_now == 1 && button_now != button_initial){
            LATAbits.LATA0 = (val+1) % 2; 
            val = (val+1) % 2;
        }
        
        button_initial = button_now;
    }

    return 0;
}