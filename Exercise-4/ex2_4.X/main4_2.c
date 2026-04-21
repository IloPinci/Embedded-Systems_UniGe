/*
 * File:   main4_2.c
 * Author: boli
 *
 * Created on March 31, 2026, 4:59 PM
 */


#include "xc.h"
#include "timer_config.h"
#include <stdio.h>



void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void) {
    IFS0bits.T3IF = 0;      // we clear so we can redo the whole thing again
    LATGbits.LATG9 = !LATGbits.LATG9;   //the switching    
}


void algorithm(){   
    tmr_wait_ms(TIMER2, 7);
}

int main(void) {
    
    // we disable the analog modality
    ANSELD = 0;
    ANSELA = 0;
    ANSELE = 0;
    
    // define the i/o
    TRISDbits.TRISD11 = 1;      // 1 -> input for uart
    TRISDbits.TRISD0 = 0;       // 0 -> output for uart
    TRISAbits.TRISA0 = 0;       // output for the LED1
    TRISGbits.TRISG9 = 0;       // output for the LED2
    TRISEbits.TRISE8 = 1;       // input for button1
    TRISEbits.TRISE9 = 1;       // input for button2
    
    
    // initial conditions
    LATAbits.LATA0 = 0;     // LED1
    LATGbits.LATG9 = 0;     // LED2
    
    
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
    
    tmr_setup_period(TIMER3, 200);
    IFS0bits.T3IF = 0;      // we clean the flag just in case
    IEC0bits.T3IE = 1;      // we enable the interrupts
    
    tmr_setup_period(TIMER1, 10);
    
    int count = 0;
    int missed=0;
    int index = 0;
    int button_initial_1 = 0, button_initial_2 = 0;
    int button_now_1, button_now_2;
    char reciv_char[3];
    
    
    while(1){
        
        // Handle buttons 
        button_now_1 = PORTEbits.RE8;
        button_now_2 = PORTEbits.RE9;
        
        if (button_now_1 == 1 && button_now_1 != button_initial_1) {
            
            char buff[3];
            int i = 0;
            
            sprintf(buff, "C=%d", count);
            
            while (buff[i] != '\0'){
                while(U1STAbits.UTXBF == 1);   
                U1TXREG = buff[i++];
            }
            
            algorithm();        // to battle debouncing
            
            button_now_1 = PORTEbits.RE8;   // just to make sure that the button has the same state
                        
        }       
        else if (button_now_2 == 1 && button_now_2 != button_initial_2){
            
            char buff[3];
            int i = 0;
            
            sprintf(buff, "V=%d", missed);
            
            while (buff[i] != '\0'){
                while(U1STAbits.UTXBF == 1);   
                U1TXREG = buff[i++];
            }
            
            algorithm();        // to battle debouncing
            
            button_now_2 = PORTEbits.RE8;         
        }
        else{
           algorithm(); 
        }
        
        // we update the history
        button_initial_1 = button_now_1;
        button_initial_2 = button_now_2;
       
              
        
        // if i write long words (more than 4 chars) it gets messed up so i just don't care 
        // and i continue to receive with the cost that the word is lost
        if (U1STAbits.OERR == 1) {
            U1STAbits.OERR = 0;   // clear the error so i can read again
            index = 0;            // just to make sure
        }
        
        
        while (U1STAbits.URXDA == 1){
            count++;
            
            char tempChar = U1RXREG;
            
            while(U1STAbits.UTXBF == 1);    // we send it back just for debug
            U1TXREG = tempChar;
            
            if (index < 3){
                reciv_char[index] = tempChar;
                index++;
            }
        }
        
        if (index >= 3){            
            if (reciv_char[0] == 'l' && reciv_char[1] == 'd' && 
                    (reciv_char[2] == '1' || reciv_char[2] == '2')){ 
                
                // to toggle the LED1
                if (reciv_char[2] == '1'){
                    LATAbits.LATA0 = !LATAbits.LATA0;
                }
                // to stop the blinking we stop just the timer interrupt
                else{
                    IFS0bits.T3IF = 0;                  // the flag becomes 0 always bc we dont want a trigger
                    IEC0bits.T3IE = !IEC0bits.T3IE;     // since stop/resume toggle the interrupt
                    T3CONbits.TON = !T3CONbits.TON;     // same idea for the timer
                    TMR3 = 0;                           // for good measure we reset it                    
                }
            }
            index = 0;
        }
        
        if (tmr_wait_period(TIMER1) == 1){
            missed++;
        }
    }
    
    return 0;
}