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
#include "can.h"

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
	CANGIE =  (1 << ENRX) | (1 << ENTX) | (1 << ENBX);
	CANGIT = 0x00;
	
	int c;
	for (c=0;c<15;c++)
	{
		
		CANPAGE = c << 4;
		CANCDMOB = 0x00;
		CANSTMOB = 0x00;
		int j;
		for(j=0; j<8; j++){
			CANMSG = 0x00;
		}
		
	}
	
	//Disable auto increment (Stopped Working!)
	CANPAGE = 1 << AINC;

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

static void set_page(uint8_t mobnr){
	CANPAGE = (mobnr << 4) | PAGE_MASK;
}

static void set_page_indx(uint8_t indx, uint8_t mobnr){
	CANPAGE = (mobnr << 4) | PAGE_MASK | indx;
}

void can_get_frame_buffer( uint8_t *message , uint8_t buff_len){
	uint8_t j = buff_len;
	for(j=0; j<8; j++){
		can_get_message(j,message);
		message = message + 8;
	}
}

void can_get_message(uint8_t mobnr , uint8_t *message ){
	set_page(mobnr);
	uint8_t j;
	for(j=0; j<8; j++){
		set_page_indx(j, mobnr);
		*message = CANMSG;
		message++;
	}
	set_page_indx(0, mobnr);
}

void can_print_message(uint8_t mobnr){
	uart_puts("message:");
	uint8_t message[8] = {0};
	can_get_message(mobnr , message);
	char hex[16] = "";
	bytes_to_hex(message, 8, hex);
	uart_puts(hex);
}

void can_get_id(uint8_t mobnr, can_id_t id){
	set_page(mobnr);
	id[0] = CANIDT2 >> 5 | CANIDT1 << 3;
	id[1] = CANIDT1 >> 5;
}

void can_init_id ( can_id_t id){
	CANIDT2 = id[0] << 5;
	CANIDT1 = id[0] >> 3 | id[1] << 5;
	//not a remote frame.
	CANIDT4 = 0 << RTRTAG;
}

/*
void can_init_mask_def(){
	CANIDM1 = 255;
	CANIDM2 = 255;
	CANIDM4 = (0 << RTRMSK) | (0 << IDEMSK);
}
*/

void can_init_mask (can_mask_t mask){
	CANIDM2 = mask[0] << 5;
	CANIDM1 = mask[0] >> 3 | mask[1] << 5;
	//RTRMSK = 0 : We don't use remote frames ever.
	//IDEMSK = 0 : we don't use the extended format.
	CANIDM4 = (0 << RTRMSK) | (0 << IDEMSK);
}

void can_init_message( uint8_t *message , uint8_t mobnr){
	set_page(mobnr);
	uint8_t j;
	for(j=0; j<8; j++){
		set_page_indx(j, mobnr);
		CANMSG = *message;
		message++;
	}
	set_page_indx(0, mobnr);
}

int can_send_message( uint8_t mobnr , can_id_t id, can_msg_t message ){
	//select mob.
	set_page(mobnr);
	//copy ID.
	can_init_id(id);
	//copy message.
	can_init_message(message, mobnr);
	//enable transmission.
	CANCDMOB = (1 << CONMOB0) | (1 << DLC3);
	//wait for transmission.
	while(CANSTMOB != (1 << TXOK));
	//reset mob.
	CANSTMOB = 0x00;
	CANCDMOB = 0x00;
	return 0;
}

int can_receive_message( uint8_t mobnr, can_id_t id, can_mask_t mask, can_msg_t message){
	set_page(mobnr);
	CANIE2 = (1 << mobnr);
	can_init_id(id);
	can_init_mask(mask);
	//CAN standard rev 2.0 A (identifiers length = 11 bits)
	CANCDMOB = (1 << CONMOB1) | (1 << DLC3); //enable reception and data length code = 8 bytes
	//wait for interrupt
	while((CANGIT & INTR_MASK) != (1 << CANIT));
	//check if it is the right interrupt.
	if((CANSTMOB & RXOK_MASK) != (1 << RXOK)) return 1;
	//reset mob.
	CANSTMOB = 0x00;
	CANCDMOB = 0x00;
	//reset interrupt enable
	CANIE2 = 0x00;
	//reset interrupt register.
	CANGIT = CANGIT;
	//retrieve message.
	can_get_message(mobnr, message);
	return 0;
}

int can_send_frame_buffer( uint8_t *message, uint8_t buff_len ){
	uint8_t j = 0;
	for(j=0; j<buff_len; j++){
		can_id_t id = {j , 0x00};
		can_send_message(j,id,message);
		message = message + 8;
	}
	return 0;
}

int can_receive_frame_buffer( uint8_t *message , uint8_t buff_len){
	//Enable buffer receive interrupt.
	CANGIE |= (1 << ENBX);
	uint8_t j;
	for(j=0; j<buff_len; j++){
		set_page(j);
		can_id_t id = { j , 0};
		can_init_id(id);
		// Mask = 255
		can_mask_t mask = { 255, 255 };
		can_init_mask(mask);
		//set mob in buffer receive mode.
		CANCDMOB = (1 << CONMOB0) | (1 << CONMOB1) | (1 << DLC3);
	}
	//wait for interrupt.
	while((CANGIT & INTR_MASK) != (1 << CANIT));
	//Check if interrupt is the right one (BXOK).
	if((CANGIT & BXOK_MASK) != (1 << BXOK)) return 1;
	//reset mob RXOK flags
	for(j=0; j<buff_len; j++){
		set_page(j);
		CANCDMOB = 0x00;
		CANSTMOB = 0x00;
	}
	//Reset interrupt register.
	CANGIT = CANGIT;
	//retrieve message.
	can_get_frame_buffer(message, buff_len);
	return 0;
}

