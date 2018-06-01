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
#include "sha2/sha512.h"


#define F_CPU 8000000L
#include <util/delay.h>

const char private_key_hex[64] = "92990788d66bf558052d112f5498111747b3e28c55984d43fed8c8822ad9f1a7";
const char public_key_hex[128] = "f0c863f8e555114bf4882cc787b95c272a7fe4dcddf1922f4f18a494e1c357a1a6c32d07beb576ab601068068f0a9e0160c3a14119f5d426a7955da3e6ed3e81";

const static uint8_t private_key[32] = { 146,153,7,136,214,107,245,88,5,45,17,47,84,152,17,23,71,179,226,140,85,152,77,67,254,216,200,130,42,217,241,167};
const static uint8_t public_key[64] = { 240,200,99,248,229,85,17,75,244,136,44,199,135,185,92,39,42,127,228,220,221,241,146,47,79,24,164,148,225,195,87,161,166,195,45,7,190,181,118,171,96,16,104,6,143,10,158,1,96,195,161,65,25,245,212,38,167,149,93,163,230,237,62,129	};

typedef enum {
	NOTHING,
	SCENARIO1,
	SCENARIO2
} run_t;

volatile run_t run_scenario = NOTHING;
		
		
void buttons_init(){
	DDRE = 0x00;
	PORTE = 1 << PE4 | 1 << PE5 | 1 << PE6 | 1 << PE7;
	EICRB = 1 << ISC40 | 1 << ISC41 | 1 << ISC50 | 1 << ISC51 | 1 << ISC60 | 1 << ISC61 | 1 << ISC70 | 1 << ISC71; // set interrupt on falling edge.
	EIMSK = 1 << INT4 | 1 << INT5 | 1 << INT6 | 1 << INT7;
}


int RNG(uint8_t *dest, unsigned size) {
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
static int RNG2(uint8_t *dest, unsigned size){
	DDRF = 0xff;
	ADMUX |= (1 << REFS0) | (0 << REFS1) | (0 << MUX0) | (0 << MUX1) | (0 << MUX2) | (0 << MUX3) | (0 << MUX4) | (0 << ADLAR);
	ADCSRA = (1 << ADEN) | (0 << ADPS0) | (0 << ADPS1) | (0 << ADPS2);
	while(size){
		ADCSRA |= (1 << ADSC);
		while(!(ADCSRA&(1<<ADIF))); 
		ADCSRA |= (1 << ADIF);
		uint8_t val = ADCL;
		uint8_t val2 = ADCH;
		*dest = val;
		++dest;
		--size;
	}
}
*/
	
ISR(INT4_vect){
	uart_puts("running scenario1");
	run_scenario = SCENARIO1;
}

ISR(INT5_vect){
	uart_puts("running scenario2");
	run_scenario = SCENARIO2;
}

ISR(INT6_vect){

}

ISR(INT7_vect){
	uint8_t target[64] = {0};
	can_send_frame_buffer(target , 8);
	//RNG(target, 64);
	//SendByMOb2();
}



int sign_challenge(uint8_t challenge[64], uint8_t signature[64]){
	uECC_set_rng(&RNG);
	const struct uECC_Curve_t *curve = uECC_secp256r1();
	uint8_t hash[64];
	sha512(hash, challenge, 512);

	if (!uECC_sign(private_key, hash, 64, signature, curve)) {
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
	can_receive_frame_buffer(challenge, 8);
	uart_puts("challenge received");
	uint8_t signature[64];
	sign_challenge(challenge, signature);
	//Send signature.
	can_send_frame_buffer(signature, 8);
	//can_send_frame_buffer(signature, 8);
	uart_puts("signature sent");
	uint8_t message[8];
	//Wait for authentication acknowledgement.
	can_receive_message(0, 0x00, 0x00,  message);
	uart_puts("authenticated!");
	return 0;
}

int run_2(){
	uint8_t result;
	can_send_message(0, 0x00, 0);
	uint8_t public[64];
	can_receive_frame_buffer(public, 8);
	const struct uECC_Curve_t * curve = uECC_secp256r1();
	uint8_t secret_unhashed[32];
	result = uECC_shared_secret(public, private_key, secret_unhashed, curve );
	uint8_t secret2[32];
	uint32_t len2 = 256;
	sha256(secret2, secret_unhashed, len2);
	char secret_hex2[64];
	bytes_to_hex(secret2, 32, secret_hex2);
	uart_puts(secret_hex2);
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
			case SCENARIO2 :
				run_scenario = NOTHING;
				run_2();
				break;
		}
	 }
	 return 0;
 }

