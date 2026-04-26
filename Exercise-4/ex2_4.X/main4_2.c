/*
 * File:   main4_2.c
 * Author: boli
 *
 * Created on March 31, 2026, 4:59 PM
 */


#include "xc.h"
#include "timer_config.h"
#include <stdio.h>

#define BUF_SIZE 16    

// the tail and the next must never equal each other because then it will 
// cause overflow. And for this reason we define the buffer size a bit bigger
// than the theoretical maximum number of characters that can be tranmistted
volatile char buffer[BUF_SIZE];
volatile int head = 0;
volatile int tail = 0;

volatile int total_chars = 0;
volatile int overflow = 0;

volatile char reciv_char[3];
volatile int index = 0;


void __attribute__((interrupt, no_auto_psv)) _U1RXInterrupt(void) {
    IFS0bits.U1RXIF = 0;
    
    while (U1STAbits.URXDA == 1) {
        char value = U1RXREG;
        
        int next = (head + 1) % BUF_SIZE;  // '%' is used to wrap around
        if (next != tail) {
            buffer[head] = value;
            head = next;
        } else {
            overflow++;   // buffer full ? drop char
        }   
        
        total_chars++;
    }
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
    
    IFS0bits.U1RXIF = 0;
    IEC0bits.U1RXIE = 1;

    tmr_setup_period(TIMER1, 10);
    
    int led2_flag = 1;
    int led2_count = 0;
    
    int missed=0;
    int button_initial_1 = 0, button_initial_2 = 0;
    int button_now_1, button_now_2;
    
    
    while(1){
        // since the while itself is executed every 100 ms we toggle on and off
        // every 200 ms
        if(led2_count++ == 20){
            if (led2_flag == 1){
                LATGbits.LATG9 = !LATGbits.LATG9;
            }
            led2_count = 0;
        }        
         
        // Handle buttons 
        button_now_1 = PORTEbits.RE8;
        button_now_2 = PORTEbits.RE9;
        
        if (button_now_1 == 1 && button_now_1 != button_initial_1) {
            char buff[10];
            
            sprintf(buff, "C=%d", total_chars);
            int i = 0;
            
            while (buff[i] != '\0'){
                while(U1STAbits.UTXBF == 1);   
                U1TXREG = buff[i++];
            }                       
        }       
        
        if (button_now_2 == 1 && button_now_2 != button_initial_2){
            
            char buff[10];
            int i = 0;
            
            sprintf(buff, "D=%d", missed);
            
            while (buff[i] != '\0'){
                while(U1STAbits.UTXBF == 1);   
                U1TXREG = buff[i++];
            }       
        }
           
        // we update the history of the buttons
        button_initial_1 = button_now_1;
        button_initial_2 = button_now_2;
       
       while (tail != head){
           char tempChar = buffer[tail];
           tail = (tail+1) % BUF_SIZE;
           
           // send back the character
           while(U1STAbits.UTXBF == 1);
           U1TXREG = tempChar;
           
           reciv_char[0] = reciv_char[1];
           reciv_char[1] = reciv_char[2];
           reciv_char[2] = tempChar;
           
            if (index < 3){
                index++;
            }
           
           if (index == 3){            
                if (reciv_char[0] == 'l' && reciv_char[1] == 'd' && 
                        (reciv_char[2] == '1' || reciv_char[2] == '2')){ 

                    // to toggle the LED1
                    if (reciv_char[2] == '1'){
                        LATAbits.LATA0 = !LATAbits.LATA0;
                    }
                    else{
                        led2_flag = !led2_flag;
                    }
                    reciv_char[0] = reciv_char[1] = reciv_char[2] = 0;
                }
            } 
       }              
        algorithm();
        
        // 100Hz and we check if it missed the deadline
        if (tmr_wait_period(TIMER1) == 1){
            missed++;
        }
    }
    
    return 0;
}


