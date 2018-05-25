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
#include "can.h"


volatile uint8_t data[8];

static char private_key_hex[64] = "92990788d66bf558052d112f5498111747b3e28c55984d43fed8c8822ad9f1a7";
static char public_key_hex[128] = "54619a4980a83e9199cc42d811ef07dcd8608c43929e1a3e443aa04deae8ff89e46154a1a074ae932b6d1395e565fcfb19dd392271d4ebedd1feadae2df9158d";

typedef enum {
	IDLE_S,
	WAIT_FOR_SIGNATURE_S,
	AUTHENTICATED_S
} state_t;

typedef enum {
	NULL_E,
	INIT_RECEIVED_E,
	SIGNATURE_RECEIVED_E,
	MESSAGE_RECEIVED_E
} event_t;

volatile state_t state = IDLE_S;
volatile event_t event = NULL_E;

void buttons_init(){
	DDRE = 0x00;
	PORTE = 1 << PE4 | 1 << PE5 | 1 << PE6 | 1 << PE7;
	EICRB = 1 << ISC40 | 1 << ISC41 | 1 << ISC50 | 1 << ISC51 | 1 << ISC60 | 1 << ISC61 | 1 << ISC70 | 1 << ISC71; // set interrupt on falling edge.
	EIMSK = 1 << INT4 | 1 << INT5 | 1 << INT6 | 1 << INT7;
}



ISR (CANIT_vect){
	
	
	
		/*
	 char target[] = "buffer received";
	 uart_puts(target);
	 uint8_t message[64];
	 can_get_frame_buffer(message);
	 char hex[129];
	 bytes_to_hex(message, 64, hex);
	 hex[128] = '\0';
	 uart_puts(hex);
	 
	 CANSTMOB=0x00;
	 CANGIT = CANGIT;
	 */
}

int run()
{
	uart_puts("idle");
	uint8_t message[8];
	can_receive_message(0, 0x00, 0x00, message);
	uart_puts("authentication started");
	uint8_t challenge[64] = { 0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 1 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 2 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 3 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 4 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 5 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 6 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 7 , 1 , 2 , 3 , 4 , 5 , 6 , 7 };
	can_send_frame_buffer(challenge);
	uart_puts("challenge sent");
	uint8_t signature[64];
	can_receive_frame_buffer(signature);
	uart_puts("signature received");
	can_send_message(0, 0x00, message);
	return 0;
}


 int main()
 {	
	uart_init();
	buttons_init();
	can_init();
	run();
 }


