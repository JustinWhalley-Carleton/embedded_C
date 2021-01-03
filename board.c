// Libraries
#include "msp.h"
#include <stdint.h>
#include <stdbool.h>

//Macros
#define DEBOUNCE_TIME 1500
#define STATE_1 0
#define STATE_2 1
#define STATE_3 2
#define STATE_4 3

//Function Headers
void PORT1_IRQHandler(void);
void port_1_config(void);
void port_2_config(void);
void handle_state_change(bool next);
void EUSCIA0_IRQHandler(void);
void configure_UART(void);
	
int main(void){
	//Disable Watchdog Timer
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD; 
	
	// Initialization state
	configure_UART();
	port_1_config();
	port_2_config();
	
	/* Main Loop */
	for (;;) {
		__WFI(); // wait for interrupts
	}
  
	return 0;
}

void port_1_config(void){
	/* Configures Port 1 */
	
	//Select Values
	P1SEL0 &= (uint8_t)~(BIT0 | BIT1 | BIT4);
	P1SEL1 &= (uint8_t)~(BIT0 | BIT1 | BIT4);
	
	//Direction Values
	P1DIR &= (uint8_t)~(BIT1 | BIT4);
	P1DIR |= (uint8_t)(BIT0);

	//Resistor Values
	P1REN |= (uint8_t)(BIT1 | BIT4);

	//Output Values
	P1OUT |= (uint8_t)(BIT1 | BIT4);
	P1OUT &= (uint8_t)~(BIT0);

	//Interrupts
	P1IE |= (uint8_t)(BIT1 | BIT4);
	P1IES |= (uint8_t)(BIT1 | BIT4);
	P1IFG &= (uint8_t)~(BIT1 | BIT4);
	P1IE &= (uint8_t)~(BIT0);

	//NVIC Interrupts
	NVIC_SetPriority(PORT1_IRQn,2);
	NVIC_GetPendingIRQ(PORT1_IRQn);
	NVIC_EnableIRQ(PORT1_IRQn);

}

void port_2_config(void){
	/* Configures Port 2 */
		
	//Select Pins
	P2SEL0 &= (uint8_t)~(BIT0 | BIT1 | BIT2);
	P2SEL1 &= (uint8_t)~(BIT0 | BIT1 | BIT2);

	//Direction Pins
	P2DIR |= (uint8_t)(BIT0 | BIT1 | BIT2);

	//Output Pins
	P2OUT &= (uint8_t)~(BIT0 | BIT1 | BIT2);

	//Interrupts
	P2IE &= (uint8_t)~(BIT0 | BIT1 | BIT2);

}

void PORT1_IRQHandler(void){
	/* Handles Port 1 button interupts */
	
	if (P1->IFG & (1<<1)) {
		
		// Debouncing
		int i = 0;
    	for(; i < DEBOUNCE_TIME;i++);
		
		// Move to next state
		if (!(P1->IN & (1<<1))) {
			handle_state_change(true);	
		}
		
		// Clear flag
		P1->IFG &= ~(1<<1);
	}
    
	if (P1->IFG & (1<<4)) {
		
		// Debouncing
    	int i = 0;
    	for(; i < DEBOUNCE_TIME;i++);
		
		// Move to previous state
    	if (!(P1->IN & (1<<4))) {
			handle_state_change(false);
		}		
		
		// Clear flag
		P1->IFG &= ~(1<<4);
	}
}

void EUSCIA0_IRQHandler(void){
/* Handles UART interupt to recieve previous and next actions */
	
	if (EUSCI_A0->IFG & EUSCI_A_IFG_RXIFG){
			
		if(EUSCI_A0->RXBUF == 'P'){
			// received a 'P' from serial go to previous state
			handle_state_change(false);
		} else if(EUSCI_A0->RXBUF == 'N'){
			// received a 'N' from serial go to next state
			handle_state_change(true);
		}
		EUSCI_A0->IFG &= ~EUSCI_A_IFG_RXIFG;    // Clear eUSCI RX interrupt flag
	}
}

void handle_state_change(bool next) {
	static int current_state = STATE_1;
	if(next) {
		switch(current_state){
			case STATE_1:
				current_state = STATE_2;
				break;
						
			case STATE_2:
				current_state = STATE_3;
				break;
						
			case STATE_3:
				current_state = STATE_4;
				break;
						
			case STATE_4:
				current_state = STATE_1;
				break;
		}
	} else {
		switch(current_state){
			case STATE_1:
				current_state = STATE_4;
				break;
						
			case STATE_2:
				current_state = STATE_1;
				break;
						
			case STATE_3:
				current_state = STATE_2;
				break;
						
			case STATE_4:
				current_state = STATE_3;
				break;
		}
	}
	switch(current_state){
		
		case STATE_1:
			//change leds to off
			P1->OUT &= ~(uint8_t)(BIT0);
			P2->OUT &= ~(uint8_t)(BIT0);
			//send '1' through serial
			EUSCI_A0->TXBUF = '1';
			break;
						
		case STATE_2:
			// change p2 led to on and p1 to off
			P1->OUT &= ~(uint8_t)(BIT0);
			P2->OUT |= (uint8_t)(BIT0);
			//send '2' through serial
			EUSCI_A0->TXBUF = '2';
			break;
						
		case STATE_3:
			// change p1 led to on and p2 to off
			P1->OUT |= (uint8_t)(BIT0);
			P2->OUT &= ~(uint8_t)(BIT0);
			//send '3' through serial
			EUSCI_A0->TXBUF = '3';
			break;
						
		case STATE_4:
			// change both leds to on
			P1->OUT |= (uint8_t)(BIT0);
			P2->OUT |= (uint8_t)(BIT0);
			//send '4' through serial
			EUSCI_A0->TXBUF = '4';
			break;
	}
}

void configure_UART(void){
	/* Configures UART port for communication, as well as interupts */
	
	// Configure UART pins
  	P1->SEL0 |= (uint8_t)(BIT2 | BIT3);     // set 2-UART pin as secondary function
	P2->SEL1 &= (uint8_t)~(BIT2 | BIT3);               
	
	// Setting clock to 12 MHz
	CS->KEY = CS_KEY_VAL;                   // Unlock CS module for register access
  	CS->CTL0 = 0;                           // Reset tuning parameters
  	CS->CTL0 = CS_CTL0_DCORSEL_3;           // Set DCO to 12MHz (nominal, center of 8-16MHz range)
  	CS->CTL1 = CS_CTL1_SELA_2 |             // Select ACLK = REFO
        CS_CTL1_SELS_3 |                // SMCLK = DCO
        CS_CTL1_SELM_3;                 // MCLK = DCO
  	CS->KEY = 0;                            // Lock CS module from unintended accesses
	
	// Configure UART
  	EUSCI_A0->CTLW0 |= EUSCI_A_CTLW0_SWRST; // Put eUSCI in reset
  	EUSCI_A0->CTLW0 = EUSCI_A_CTLW0_SWRST | // Remain eUSCI in reset
  	EUSCI_B_CTLW0_SSEL__SMCLK;      				// Configure eUSCI clock source for SMCLK
	
	//Configure Control Registers
	UCA0CTLW0 |= (uint16_t)(BIT0); //Reset UART state
	UCA0CTLW0 &= (uint16_t)~(BITD); //Set first bit to LSB
	UCA0CTLW0 &= (uint16_t)~(BITC); //Configure 8 data bits
	UCA0CTLW0 |= (uint16_t)(BITB);   //Configure 2 stop bits
	UCA0CTLW0 &= (uint16_t)~(BITF);  //Disable parity bit
	UCA0CTLW0 &= (uint16_t)~(BITA | BIT9); //Set UART mode
	UCA0CTLW0 &= (uint16_t)~(BIT8); //Enables asynchronous mode
	UCA0IRCTL &= (uint16_t)~(BIT0); //Disable IrDA encoder
	
	//Configure Baud rate
	UCA0CTLW0 |= (uint16_t)(BIT7);
	UCA0CTLW0 |= (uint16_t)(BIT6);
	UCA0MCTLW |= (uint16_t)(BIT0);
	UCA0BRW = (uint16_t)(78);
	UCA0MCTLW &= (uint16_t)~(BIT7|BIT6|BIT4);
	UCA0MCTLW |= (uint16_t)(BIT5);
	UCA0MCTLW &= (uint16_t)~(BIT8|BITA|BITD);
	UCA0MCTLW &= (uint16_t)~(BIT9|BITB|BITC|BITC|BITE|BITF);
	
	//Set UART state
	UCA0CTLW0 &= (uint16_t)~(BIT0);
	
	//Configure Interupts
	EUSCI_A0->IE |= (uint8_t)(BIT1); //Transmit IE
	EUSCI_A0->IE |= (uint8_t)(BIT0); //Recieve IE
	UCA0IFG &= (uint16_t)~(BIT1); //Transmit Flag Cleared
	UCA0IFG &= (uint16_t)~(BIT0); //Recieve Flag Cleared
	
	//Configure NVIC
	NVIC_SetPriority(EUSCIA0_IRQn,2);
	NVIC_ClearPendingIRQ(EUSCIA0_IRQn);
	NVIC_EnableIRQ(EUSCIA0_IRQn);
}
