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
#include "ECC/uECC.h"
#include "leds.h"
#include "uart_f.h"
#include "hexconv.h"
#include "can.h"


#define F_CPU 8000000L
#include <util/delay.h>

static char private_key_hex[64] = "92990788d66bf558052d112f5498111747b3e28c55984d43fed8c8822ad9f1a7";
static char public_key_hex[128] = "54619a4980a83e9199cc42d811ef07dcd8608c43929e1a3e443aa04deae8ff89e46154a1a074ae932b6d1395e565fcfb19dd392271d4ebedd1feadae2df9158d";

typedef enum {
	NOTHING,
	SCENARIO1
} run_t;

volatile run_t run_scenario = NOTHING;
		
		
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
	

/*	
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
	
	
	if (!uECC_make_key(public, private, curve)) {
		printf("uECC_make_key() failed\n");
		return 1;
	}
	
	
	
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

*/



	
ISR(INT4_vect){
	uart_puts("running scenario");
	run_scenario = SCENARIO1;
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

int sign_challenge(uint8_t challenge[64], uint8_t signature[64]){
	const struct uECC_Curve_t *curve = uECC_secp256k1();
	uint8_t hash[64];
	uint8_t private2[32] = {0};
	uint8_t public2[64] = {0};
	hex_to_bytes(private_key_hex, 64, private2);
	hex_to_bytes(public_key_hex, 128, public2);
	memcpy(hash, public2, sizeof(hash));
	if (!uECC_sign(private2, hash, sizeof(hash), signature, curve)) {
		uart_puts("sign failed");
		return 1;
	}
	
	return 0;
}

int run_1(){
	uart_puts("starting authentication");
	uint8_t challenge[64];
	//init authentication.
	can_send_message(0, 0x00, 0);
	//wait for challenge.
	can_receive_frame_buffer(challenge);
	uart_puts("challenge received");
	uint8_t signature[64];
	sign_challenge(challenge, signature);
	//Send signature.
	can_send_frame_buffer(signature);
	uart_puts("signature sent");
	uint8_t message[64];
	//Wait for authentication acknowledgement.
	can_receive_message(0, 0x00, 0x00,  message);
	uart_puts("authenticated!");
	return 0;
}

 int main()
 {	
	 
	 uart_init();
	 buttons_init();
	 can_init();
	 
	 while(1){
		run_t runlcl = run_scenario;
		switch(runlcl){
			case NOTHING :
				break;
			case SCENARIO1 :
				run_scenario = NOTHING;
				run_1();
				break;
		}
	 }
	 return 0;
	 
 }

