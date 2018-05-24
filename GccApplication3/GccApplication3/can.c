/*
 * can.c
 *
 * Created: 5/15/2018 8:42:23 PM
 *  Author: michel
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "hexconv.h"
#include "uart_f.h"

#define INTR_MASK 0b10000000
#define BXOK_MASK 0b00010000
#define RXOK_MASK 0b00100000


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
	CANGIE = (1 << ENIT) | (1 << ENRX) | (1 << ENTX);
	CANGIT = 0x00;
	
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

		sei();

}


//For testing.
/*
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
	CANIE1 = 0x00 ; 

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

*/

void can_get_frame_buffer( uint8_t *message ){
	uint8_t j;
	for(j=0; j<8; j++){
		can_get_message(j,message);
		message = message + 8;
	}
}

void can_get_message(uint8_t mobnr , uint8_t *message ){
	CANPAGE = mobnr << 4;
	uint8_t j;
	for(j=0; j<8; j++){
		*message = CANMSG;
		message++;
	}
}

void can_print_message(uint8_t mobnr){
	uart_puts("message:");
	uint8_t message[8] = {0};
	can_get_message(mobnr , message);
	char hex[16] = "";
	bytes_to_hex(message, 8, hex);
	uart_puts(hex);
}

void can_init_id ( uint8_t id){
	CANIDT1 = id << 5;
	CANIDT2 = id >> 3;
	//not a remote frame.
	CANIDT4 = 0 << RTRTAG;
}

void can_init_mask_def(){
	CANIDM1 = 255;
	CANIDM2 = 255;
	CANIDM4 = (0 << RTRMSK) | (0 << IDEMSK);
}

void can_init_mask (uint8_t mask){
	CANIDM1 = mask << 5;
	CANIDM2 = mask >> 3;
	// RTRMSK = 0 : We don't use remote frames ever.
	// IDEMSK = 0 : we don't use the extended format.
	CANIDM4 = (0 << RTRMSK) | (0 << IDEMSK);
}

void can_init_message( uint8_t *message ){
	uint8_t j;
	for(j=0; j<8; j++){
		CANMSG = *message;
		message++;
	}
}

int can_send_message( uint8_t mobnr , uint8_t id, uint8_t *message ){
	//select mob.
	CANPAGE = (mobnr << 4);
	//copy ID.
	can_init_id(id);
	//copy message.
	can_init_message(message);
	//enable transmission
	CANCDMOB = (1 << CONMOB0) | (1 << DLC3);
	
	return 0;
}

int can_receive_message( uint8_t mobnr, uint8_t id, uint8_t mask, uint8_t *message){
	CANPAGE = (mobnr << 4);
	can_init_id(id);
	can_init_mask(mask);
	//CAN standard rev 2.0 A (identifiers length = 11 bits)
	CANCDMOB = (1 << CONMOB1) | (1 << DLC3); //enable reception and data length code = 8 bytes
	//wait for interrupt
	while((CANGIT & INTR_MASK) != (1 << CANIT));
	//check if it is the right interrupt.
	if((CANSTMOB & RXOK_MASK) != (1 << RXOK)) return 1;
	//reset mob RXOK flag.
	CANSTMOB = 0x00;
	//reset interrupt register.
	CANGIT = CANGIT;
	//retrieve message.
	can_get_message(mobnr, message);
	return 0;
}

void can_send_frame_buffer( uint8_t *message ){
	uint8_t j;
	for(j=0; j<8; j++){
		can_send_message(j,j,message);
		message = message + 8;
	}
}

void can_receive_frame_buffer( uint8_t *message ){
	//Enable buffer receive interrupt.
	CANGIE |= 1 << ENBX;
	uint8_t j;
	for(j=0; j<8; j++){
		CANPAGE = (j << 4);
		can_init_id(j);
		// Mask = 255
		can_init_mask_def();
		//set mob in buffer receive mode.
		CANCDMOB = (1 << CONMOB0) | (1 << CONMOB1) | (1 << DLC3);
	}
	
	CANPAGE = CANGIT & INTR_MASK;
	CANPAGE = CANGIT & INTR_MASK;
	CANPAGE = CANGIT & INTR_MASK;
	//wait for interrupt.
	while((CANGIT & INTR_MASK) != (1 << CANIT));
	//Check if interrupt is the right one (BXOK).
	if((CANGIT & BXOK_MASK) != (1 << BXOK)) return 1;
	//reset mob RXOK flags
	for(j=0; j<8; j++){
		CANPAGE = (j << 4);
		CANSTMOB = 0x00;
	}
	//Reset interrupt register.
	CANGIT = CANGIT;
	//retrieve message.
	can_get_frame_buffer(message);
	return 0;
}

/*
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

*/
