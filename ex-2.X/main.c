/*
 * File:   main.c
 * Author: JT
 *
 * Created on March 5, 2026, 3:32 PM
 */


#include "xc.h"
#include <stdint.h>

int main(void) {
    // we disable the analog modality
    ANSELEbits.ANSE8 = 0;
    
    // lets define the output (LED)
    TRISAbits.TRISA0 = 0;     
    // define the inputs (buttons)
    TRISEbits.TRISE8 = 1;
    
    // give the initial state to our LED
    LATAbits.LATA0 = 1; 
    
    // now we declare some variables
    uint16_t button1;
    

    while(1) {
        button1 = PORTEbits.RE8;
        
        if (button1 == 1){
            LATAbits.LATA0 = 0; 
        }
        else{
            LATAbits.LATA0 = 1;
        }  
    }

    return 0;
}

