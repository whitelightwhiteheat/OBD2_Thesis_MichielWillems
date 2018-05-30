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

static char private_key_hex[64] = "92990788d66bf558052d112f5498111747b3e28c55984d43fed8c8822ad9f1a7";
static char public_key_hex[128] = "f0c863f8e555114bf4882cc787b95c272a7fe4dcddf1922f4f18a494e1c357a1a6c32d07beb576ab601068068f0a9e0160c3a14119f5d426a7955da3e6ed3e81";

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

int ecc_test2(){
 int i, c;
 uint8_t private[32] = {0};
 uint8_t public[64] = {0};
 uint8_t hash[64] = {0};
 uint8_t sig[64] = {0};
	 
 uECC_set_rng(&RNG);

 const struct uECC_Curve_t * curves[5];
 int num_curves = 0;
 /*
 #if uECC_SUPPORTS_secp160r1
 curves[num_curves++] = uECC_secp160r1();
 #endif
 #if uECC_SUPPORTS_secp192r1
 curves[num_curves++] = uECC_secp192r1();
 #endif
 #if uECC_SUPPORTS_secp224r1
 curves[num_curves++] = uECC_secp224r1();
 #endif
 */
 /*
 #if uECC_SUPPORTS_secp256r1
 curves[num_curves++] = uECC_secp256r1();
 #endif
 */
 
 #if uECC_SUPPORTS_secp256k1
 curves[num_curves++] = uECC_secp256k1();
 #endif
 
 
 //printf("Testing 256 signatures\n");
 //for (c = 0; c < num_curves; ++c) {
		 //printf("KZNUDVZIL");
		 //fflush(stdout);
		 
		 if (!uECC_make_key(public, private, curves[0])) {
			 printf("uECC_make_key() failed\n");
			 return 1;
		 }
		 
		 volatile uint8_t private2[32] = {0};
		volatile uint8_t public2[64] = {0};
	
		hex_to_bytes(private_key_hex, 64, private2);
		hex_to_bytes(public_key_hex, 128, public2);
		if( !memcmp(private,private2,32) ){
			uart_puts("pr ok!");
		}
	
		//_delay_ms(1000);
		if( !memcmp(public,public2,64) ) uart_puts("pu ok!");
	

		 
		 //memcpy(hash, public, sizeof(hash));
		 sha512(hash, public, 512);
		 
		 if (!uECC_sign(private, hash, sizeof(hash), sig, curves[0])) {
			 printf("uECC_sign() failed\n");
			 return 1;
		 }
		volatile int result = uECC_verify(public, hash, sizeof(hash), sig, curves[0]);
		/*
		 if (!uECC_verify(public, hash, sizeof(hash), sig, curve)) {
			 //printf("uECC_verify() failed\n");
			 uart_puts("kaka");
			 return 1;
		 }
		 */
 //}
 
 return 0;
 }
	


void ecc_test(){
	uint8_t private[32] = {0};
	uint8_t public[64] = {0};
	uint8_t hash[32] = {0};
	uint8_t sig[64] = {0};
		
	const struct uECC_Curve_t *curve;
	
	#if uECC_SUPPORTS_secp256k1
	curve = uECC_secp256k1();
	#endif

	uECC_set_rng(&RNG);
	
	
	if (!uECC_make_key(public, private, curve)) {
		printf("uECC_make_key() failed\n");
		return 1;
	}
	
	
	
	volatile uint8_t private2[32] = {0};
	volatile uint8_t public2[64] = {0};
	
	hex_to_bytes(private_key_hex, 64, private2);
	hex_to_bytes(public_key_hex, 128, public2);
	if( !memcmp(private,private2,32) ){
		uart_puts("pr ok!");
	}
	
	//_delay_ms(1000);
	if( !memcmp(public,public2,64) ) uart_puts("pu ok!");
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
	uart_puts("running scenario");
	run_scenario = SCENARIO1;
}

ISR(INT5_vect){
	char target[] = "5";
	uart_puts(target);
	//SendByMOb2();
}

ISR(INT6_vect){
	uint8_t signature[64];
	uint8_t challenge[64];
	//memcpy(challenge, public, sizeof(hash));
	sign_challenge(challenge, signature);
}

ISR(INT7_vect){
	uint8_t target[64];
	RNG(target, 64);
	//SendByMOb2();
}



int sign_challenge(uint8_t challenge[64], uint8_t signature[64]){
	uECC_set_rng(&RNG);
	const struct uECC_Curve_t *curve = uECC_secp256r1();
	uint8_t hash[64];
	uint8_t private2[32];
	uint8_t public2[64];
	hex_to_bytes(private_key_hex, 64, private2);
	hex_to_bytes(public_key_hex, 128, public2);
	sha512(hash, challenge, 512);
	//memcpy(hash, challenge, sizeof(hash));
	//volatile int result; //= uECC_valid_public_key(public2, curve);
	
	//uint8_t public[64] = {0};
	//result = uECC_compute_public_key(private2, public, curve);
	
	//result = memcmp(public, public2, 64);
	
	//if(!memcmp(public, public2, 64)) uart_puts("the same");
	
	if (!uECC_sign(private2, hash, 64, signature, curve)) {
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
	uint8_t message[8];
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

