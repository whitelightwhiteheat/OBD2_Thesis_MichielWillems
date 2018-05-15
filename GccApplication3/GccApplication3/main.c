/*
 * GccApplication2.c
 *
 * Created: 3/28/2018 12:30:25 AM
 * Author : michel
 */ 
#include <avr/io.h>
#include <string.h>
#include <stdlib.h>
#include <avr/interrupt.h>

#define F_CPU 8000000L

#include "uart_f.h"
#include "hexconv.h"
#include "leds.h"


volatile uint8_t data[8];

static char private_key_hex[64] = "92990788d66bf558052d112f5498111747b3e28c55984d43fed8c8822ad9f1a7";
static char public_key_hex[128] = "54619a4980a83e9199cc42d811ef07dcd8608c43929e1a3e443aa04deae8ff89e46154a1a074ae932b6d1395e565fcfb19dd392271d4ebedd1feadae2df9158d";

typedef enum {
	IDLE_S,
	SEND_CHALLENGE_S,
	VERIFY_SIGNATURE_S,
	AUTHENTICATED_S
} state_t;

typedef enum {
	NULL_E,
	INIT_RECEIVED,
	SIGNATURE_RECEIVED,
} event_t;

volatile state_t state = IDLE_S;
volatile event_t event;

void CAN_INIT(void){

	cli();

	// no overload frame request
	// no listening mode
	// no test mode
	// standby mode
	// software reset request
	CANGCON = (1 << SWRES);
	
	// enable receive interrupt
	// enable general interrupt
	CANGIE = (1 << ENIT) | (1 << ENRX);

	// MOb 1 interrupt enable
	// MOb======> Message object
	CANIE2 = (1 << IEMOB1);

	// MOb 8 to 14 interrupt disable
	CANIE1 = 0x00 ; /*MOb 8~14*/

	// can general interrupt register
	CANGIT = CANGIT;

	// MOb initialization
	int c;
	for (c=0;c<15;c++)
	{
		
		CANPAGE = c << 4;
		CANCDMOB = 0x00;
		CANSTMOB = 0x00;
		
	}

	// CAN bit timing registers
	// set the timing (baud) ? 125 KBaud with 8 Mhz clock
	CANBT1 = 0x06 ;    // Baud Rate Prescaler
	CANBT2 = 0x0c ;    // Re-Synchronization & Propagation
	CANBT3 = 0x37 ;    // Phase Segments & Sample Point(s)
	// CAN Timer Clock Period: 0.500 us
	CANTCON = 0x00 ;

	// enter enable mode
	CANGCON = 0b00000010 ;    // (1 << ENA/STB)

	// MOb 1 initialization

	CANPAGE = 0b00010000;    // (1 << MOBNB0)
	CANIDT1 = 0x00; // ID = 1000
	CANIDT2 = 0x00;
	CANIDT3 = 0x00;
	CANIDT4 = 0x00;
	CANIDM1 = 0x00;  // no mask
	CANIDM2 = 0x00;
	CANIDM3 = 0x00;
	CANIDM4 = 0x00;

	//CAN standard rev 2.0 A (identifiers length = 11 bits)
	// (1 << CONMOB1) | (1 << DLC3)
	CANCDMOB = 0b10001000; //enable reception and data length code = 8 bytes

	sei();

}

void ReceiveByMOb1(void){

	int j;

	CANPAGE = 0b00010000;  // (1 << MOBNB0)
	CANIDT1 = 0b01111101; // ID = 1000
	CANIDT2 = 0x00;
	CANIDT3 = 0x00;
	CANIDT4 = 0x00;
	CANIDM1 = 0b11111111;  // no mask
	CANIDM2 = 0b11100000;
	CANIDM3 = 0x00;
	CANIDM4 = 0x00;


	//CAN standard rev 2.0 A (identifiers length = 11 bits)
	// (1 << CONMOB1) | (1 << DLC3)
	CANCDMOB = 0b10001000; //enable reception and data length code = 8 bytes

}


void SendByMOb2(void){

	int j;
	
	// If the last MOb2 Tx failed to complete, just skip it since we should not
	// write to MOb2 CANCDMOB with a new command while MOb2 is still busy.
	if ((CANEN2 & (1 << ENMOB2)) != 0) {
		
		return;
	}
	
	CANPAGE = 0b00100000;  // (1 << MOBNB1)
	CANIDT1 = 0b00101001; // ID = 0x14a
	CANIDT2 = 0b01000000;
	CANIDT3 = 0x00;
	CANIDT4 = 0x00;
	CANIDM1 = 0x00;  // no mask
	CANIDM2 = 0x00;
	CANIDM3 = 0x00;
	CANIDM4 = 0x00;
	
	for(j=0; j<8; j++){
		//CANMSG = data[j];
		CANMSG = j;
	}

	//  (1 << CONMOB0) | (1 << DLC3)
	CANCDMOB = (1 << CONMOB0) | (1 << DLC3); //enable transmission!! , data length =8

	CANSTMOB = 0x00;      // clear flag

}

ISR (CANIT_vect){
	
	 CANSTMOB=0x00;
	 char target[] = "message received";
	 uart_puts(target);
	 
	 int j;
	 
	 for(j=0; j<8; j++){
		 data[j]= CANMSG;
	 }
	 
	 CAN_INIT();
	
}

 int main()
 {	
	uart_init();
	CAN_INIT();
	while(1){
		
	}
 }


