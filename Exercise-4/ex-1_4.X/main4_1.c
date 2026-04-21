/*
 * File:   main4_1.c
 * Author: boli
 *
 * Created on March 31, 2026, 4:59 PM
 */


#include "xc.h"
#include "timer_config.h"
#include <stdio.h>

int main(void) {
    
    // We are doing the configuration!
    
    // we disable the analog modality for the D ports
    ANSELD = 0;
    
    
    // we define the R11 register as an input and R0 as output
    TRISDbits.TRISD11 = 1;      // 1 -> input
    TRISDbits.TRISD0 = 0;       // 0 -> output
    
    // now we remap the pins    
    
    // remap for the input which is supposed to be the RD11 pin
    RPINR18bits.U1RXR = 75;         // for input
    
    // remap for the output which is supposed to be the RD0
    RPOR0bits.RP64R = 0x01;         // for output
    
    
    U1MODEbits.BRGH = 0;    // to select the 16 divisor
    U1BRG = 468;            // we load it so we can get 9600
    
    // Power on module and enable transmitter
    U1MODEbits.UARTEN = 1;
    U1STAbits.UTXEN = 1;
    
    
    int count = 0;
    char reciv_char;
    
    tmr_setup_period(TIMER1, 10);
    
    while(1){
        
        while(U1STAbits.URXDA == 0);
        reciv_char = U1RXREG;    // to read from the hardware buffer
        
        
        while(U1STAbits.UTXBF == 1);
        U1TXREG = reciv_char;
        
        count++;
             
    }
    
    return 0;
}
