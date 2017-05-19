//*****************************************************************************
//ECEN3360
//Final Project
//Xucheng You
//Yiming Wang
//Description: This code takes in led speed command from UART port and choose
//			   the size of the gap in the led color profile to switch the color
//			   varying speed.
//****************************************************************************

#include "msp.h"
#include <stdio.h>
#include <stdbool.h>

volatile uint8_t rxd_command[3];
volatile uint8_t rxd;
void LED_display(uint32_t GRB);
void configure_serial_port(int baud);
uint8_t get_char();
void uart_putchar(uint8_t tx_data);
void uart_putchar_n(uint8_t *BufferPtr,uint32_t length);
void eUSCIA3IsrHandler(void);
volatile int j;
volatile bool state = 0;
volatile uint8_t ind;

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;           // Stop watchdog timer

    P4DIR |= BIT4;
    P4SEL0 |= BIT4;
    P4SEL1 &= ~BIT4;
    P2DIR = BIT5;
    configure_serial_port(0);

    CSKEY = 0x695A;
    CSCTL0 = DCORES;
    CSCTL0 |= DCORSEL_4;
    CSCTL1 |= DIVS_3|DIVHS_3;
    uint32_t GRB[150];
    uint32_t color_profile[256*6]=0;

    uint16_t i;
    uint16_t j;

    /* Color profile: The following code creates an array
     *of size 1536. It contains the RGB color information
     *of rainbow color from red to violet.
     */
    for(i=0;i<=255;i++){
    	color_profile[i]=0x0000FF00+i*65536;
    }
    for(i=0;i<=255;i++){
        color_profile[i+256]=0x00FFFF00-i*256;
    }
    for(i=0;i<=255;i++){
        color_profile[i+256*2]=0xFF0000+i;
        }
    for(i=0;i<=255;i++){
        color_profile[i+256*3]=0x00FF00FF-i*65536;
    }
    for(i=0;i<=255;i++){
        color_profile[i+256*4]=0x000000FF+i*256;
    }
    for(i=0;i<=255;i++){
        color_profile[i+256*5]=0x0000FFFF-i;
    }

    uint16_t step[7]={
    		1,
    		3,
    		6,
    		12,
    		24,
    		48,
    		96,
    };

    uint8_t speed = 0;
    uint16_t STEP = 1;
    while(1){
    	/*for (j=0;j<=256*6-1;j=j+STEP){
    		for (i=0;i<=149;i++){
    			LED_display(GRB[i]);
    		}
    		for (i=0;i<=149;i++){
    			GRB[i]=color_profile[j];
    		}
    		STEP = step[rxd_command[0]-0x30];
    	}
    	j=0;*/
    	char batt_low [] = "hello world";
        uart_putchar_n((uint8_t*)batt_low,sizeof(batt_low));
        printf("%c\n",rxd);
    }
}

void LED_display(uint32_t display){
	uint8_t i;
	bool bit;
	for(i=24;i > 0;i--){
		bit=(display>>(i-1)&1);
		if(bit){
			P2OUT = BIT5;
			__delay_cycles(12);
			P2OUT = 0;
		}
		else{
			P2OUT = BIT5;
			P2OUT = 0;
			__delay_cycles(12);
		}
	}
}

uint8_t get_char(){
	while(!(UCA3IFG & UCRXIFG));
		return UCA3RXBUF;          // Load data onto buffer
}
void uart_putchar(uint8_t tx_data){
	while(!(UCA3IFG & UCTXIFG));
	UCA3TXBUF = tx_data;          // Load data onto buffer
}

void uart_putchar_n(uint8_t *BufferPtr,uint32_t length){
	while(length != 0){
		while(!(UCA3IFG & UCTXIFG));
		UCA3TXBUF = *BufferPtr;
		BufferPtr++;
		length--;
	}
}

void configure_serial_port(int baud){
	// Configure UART pins, set 2-UART pin as primaryfunction
	P9SEL0 |= (BIT6 | BIT7);
	P9SEL1|= 0;
	// Configure UART
	UCA3CTLW0 |= UCSWRST;       // Put eUSCI in reset
	UCA3CTLW0 |= UCSSEL_2;		// Select Frame parameters and clock source
	if(baud == 0){				//baudrate=9600
		UCA3BRW = 312;
		UCA3MCTLW = 8;
	}
	else if(baud == 1){
		UCA3BRW = 26;				// So we set the same Baud rate here.
		UCA3MCTLW = 0x01<<8;         		// Set first stage modulator bits
	}
	UCA3CTLW0 &= ~UCSWRST;      // Initialize eUSCI
	UCA3IE |= UCRXIE;          	// Enable USCI_A0 RX interrupts
	NVIC_ISER0 = 1 << ((INT_EUSCIA3 -16) & 31); // Enable eUSCIA0 interrupt in NVIC module
}

void eUSCIA3IsrHandler(void){
	if(UCA3IFG & UCRXIFG) {
		// code to handle RX interrupts
		UCA3IFG &= ~UCRXIFG;
		rxd_command[ind] = UCA3RXBUF;
		ind++;
		}
	if(ind >2){
		ind = 0;
	}
	if(UCA3IFG & UCTXIFG) {
		// code to handle TX interrupts
		UCA0IFG &= ~UCTXIFG;
	}
	rxd = UCA3RXBUF;
}
