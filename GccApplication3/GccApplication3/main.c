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


ISR (CANIT_vect){
	
	 char target[] = "buffer received";
	 uart_puts(target);
	 CANSTMOB=0x00;
	 CANGIT = CANGIT;
}

 int main()
 {	
	uart_init();
	can_init();
	can_receive_frame_buffer();
	while(1){
		
	}
 }


