/*
 * can.c
 *
 * Created: 5/15/2018 8:42:23 PM
 *  Author: michel
 */ 

#include <avr/io.h>
#include <interrupt.h>
#include "hexconv.h"
#include "uart_f.h"

void can_init(){

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
	//CANIE2 = (1 << IEMOB1);

	// MOb 8 to 14 interrupt disable
	//CANIE1 = 0x00 ; /*MOb 8~14*/

	// can general interrupt register
	CANGIT = 0x00;

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
	CANGCON = 0b00000010 ;    //(1 << ENA/STB)

	// MOb 1 initialization
	
	/*
	CANPAGE = 0x00;    // (1 << MOBNB0)
	CANIDT1 = 0x00; // ID = 1000
	CANIDT2 = 0x00;
	CANIDT3 = 0x00;
	CANIDT4 = 0x00;
	CANIDM1 = 0x00;  // no mask
	CANIDM2 = 0x00;
	CANIDM3 = 0x00;
	CANIDM4 = 0x00;

	//CAN standard rev 2.0 A (identifiers length = 11 bits)
	CANCDMOB = (1 << CONMOB1) | (1 << DLC3); //enable reception and data length code = 8 bytes
	
	*/

	sei();

}


//For testing.
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
	CANGIT = 0x00;

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
	CANGCON = 0b00000010 ;    //(1 << ENA/STB)

	// MOb 1 initialization

	CANPAGE = 0x00;    // (1 << MOBNB0)
	CANIDT1 = 0x00; // ID = 1000
	CANIDT2 = 0x00;
	CANIDT3 = 0x00;
	CANIDT4 = 0x00;
	CANIDM1 = 0x00;  // no mask
	CANIDM2 = 0x00;
	CANIDM3 = 0x00;
	CANIDM4 = 0x00;

	//CAN standard rev 2.0 A (identifiers length = 11 bits)
	CANCDMOB = (1 << CONMOB1) | (1 << DLC3); //enable reception and data length code = 8 bytes

	sei();

}

void get_message(uint8_t mobnr , uint8_t *message ){
	CANPAGE = 1 << mobnr;
	
	uint8_t j;
	for(j=0; j<8; j++){
		*message = CANMSG;
		message++;
	}
}

void print_message(uint8_t mobnr){
	uart_puts("message:");
	uint8_t message[8] = {0};
	get_message(mobnr , message);
	char hex[17] = "";
	bytes_to_hex(message, 8, hex);
	hex[17] = "/n";
	uart_puts(hex);
}

void send_message( uint8_t mobnr , uint8_t *id, uint8_t *message ){
	
	//select mob.
	CANPAGE = 1 << mobnr;
	
	//copy ID.
	CANIDT1 = *id;
	CANIDT2 = *(++id);
	CANIDT3 = *(++id);
	CANIDT4 = *(++id);
	
	//copy message.
	uint8_t j;
	for(j=0; j<8; j++){
		CANMSG = *message;
		message++;
	}
	
	print_message(mobnr);
	
	//enable transmission
	CANCDMOB = (1 << CONMOB0) | (1 << DLC3);
	
}


//For testing
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
		CANMSG = 0b11111111;
	}

	//  (1 << CONMOB0) | (1 << DLC3)
	CANCDMOB = (1 << CONMOB0) | (1 << DLC3); //enable transmission!! , data length =8

	CANSTMOB = 0x00;      // clear flag

}
