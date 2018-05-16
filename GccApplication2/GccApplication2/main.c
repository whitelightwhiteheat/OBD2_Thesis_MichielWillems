/*
 * GccApplication2.c
 *
 * Created: 3/28/2018 12:30:25 AM
 * Author : michel
 */ 
#include <avr/io.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <interrupt.h>
#include "uECC.h"
#include "leds.h"
#include "uart_f.h"
#include "hexconv.h"
#include "can.h"


#define F_CPU 8000000L
#include <util/delay.h>

static char private_key_hex[64] = "92990788d66bf558052d112f5498111747b3e28c55984d43fed8c8822ad9f1a7";
static char public_key_hex[128] = "54619a4980a83e9199cc42d811ef07dcd8608c43929e1a3e443aa04deae8ff89e46154a1a074ae932b6d1395e565fcfb19dd392271d4ebedd1feadae2df9158d";

typedef enum {
	IDLE_S,
	INIT_AUTHENTICATION_S,
	SIGN_CHALLENGE_S,
	AUTHENTICATED_S
} state_t;
	
typedef enum {
	NULL_E,
	AUTH_E,
	CHALLENGE_RECEIVED_E,
	AUTHENTICATION_ACK_E,
} event_t;

volatile state_t state = IDLE_S;
volatile event_t event;
		
		
void buttons_init(){
	DDRE = 0x00;
	PORTE = 1 << PE4 | 1 << PE5 | 1 << PE6 | 1 << PE7;
	EICRB = 1 << ISC40 | 1 << ISC41 | 1 << ISC50 | 1 << ISC51 | 1 << ISC60 | 1 << ISC61 | 1 << ISC70 | 1 << ISC71; // set interrupt on falling edge.
	EIMSK = 1 << INT4 | 1 << INT5 | 1 << INT6 | 1 << INT7;
}


static int RNG(uint8_t *dest, unsigned size) {
	while(size){
		uint8_t val = (uint8_t) rand();
		*dest = val;
		++dest;
		--size;
	}

	// NOTE: it would be a good idea to hash the resulting random data using SHA-256 or similar.
	return 1;
}




//For testing purposes


// Send a remote frame to start the authentication protocol.
// ID is still arbitrary (TODO)
void init_authentication(){
	CANPAGE = (1 << MOBNB1);
	int j;
	for(j=0; j<8; j++){
		//CANMSG = data[j];
		CANMSG = 0b11111111;
	}
	CANIDT4 = (1 << RTRTAG);
	CANCDMOB = (1 << CONMOB0);
	light_led(1);
}

//MOB1-8 in Buffer receive mode.
void receive_challenge(){
	cli();
	
	uint8_t j = 0;
	for(j=0; j<8; j++){
		CANPAGE = (1 << j);
		CANIDT1 = j;
		CANIDT2 = 0x00;
		CANIDT3 = 0x00;
		CANIDT4 = 0x00;
		CANIDM1 = 0x00;  // no mask
		CANIDM2 = 0x00;
		CANIDM3 = 0x00;
		CANIDM4 = 0x00;
		CANCDMOB = (1 << CONMOB0) | (1 << CONMOB1) | (1 << DLC3);
	}
	
	
}

	
void ecc_test(){
	uint8_t private[32] = {0};
	uint8_t public[64] = {0};
	uint8_t hash[32] = {0};
	uint8_t sig[64] = {0};
		
	const struct uECC_Curve_t *curve = uECC_secp256k1();
	
	#if uECC_SUPPORTS_secp256k1
	curve = uECC_secp256k1();
	#endif

	uECC_set_rng(&RNG);
	
	/*
	if (!uECC_make_key(public, private, curve)) {
		printf("uECC_make_key() failed\n");
		return 1;
	}
	
	*/
	
	uint8_t private2[32] = {0};
	uint8_t public2[64] = {0};
	
	hex_to_bytes(private_key_hex, private2);
	hex_to_bytes(public_key_hex, public2);
	if( memcmp(private,private2,32) ) uart_puts("pr ok!");
	_delay_ms(1000);
	if( memcmp(public,public2,64) ) uart_puts("pu ok!");
	memcpy(hash, public, sizeof(hash));
		
			
	if (!uECC_sign(private, hash, sizeof(hash), sig, curve)) {
		uart_puts("sign failed");
		return;
		
	}

	if (!uECC_verify(public, hash, sizeof(hash), sig, curve)) {
		uart_puts("verify failed");
		return;
	}
	printf("\n");
	uart_puts("Keys Match!");
}


	
ISR(INT4_vect){
	event = AUTH_E;
	//char target[] = "4";
	//uart_puts(target);
	uint8_t test[64] = { 0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 1 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 2 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 3 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 
						4 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 5 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 6 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 7 , 1 , 2 , 3 , 4 , 5 , 6 , 7 };
	uint8_t id[4] = { 1 , 1 , 0 , 0 };
	can_send_frame_buffer(test);
	//can_send_message(0 , id , test );
}

ISR(INT5_vect){
	char target[] = "5";
	uart_puts(target);
	//SendByMOb2();
}

ISR(INT6_vect){
	char target[] = "6";
	uart_puts(target);
	//SendByMOb2();
}

ISR(INT7_vect){
	char target[] = "7";
	uart_puts(target);
	//SendByMOb2();
}

 int main()
 {	
	 uart_init();
	 buttons_init();
	 can_init();
	 
	 while(1){
		 /*
		 switch (event)
		 {
		 case AUTH_E :
			init_authentication();
		 	break;
		 }
		 */
	 }
	 return 0;
 }

