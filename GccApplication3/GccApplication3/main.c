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
#include "ECC/uECC.h"

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

int ecc_test2(){
 int i, c;
 uint8_t private[32] = {0};
 uint8_t public[64] = {0};
 uint8_t hash[32] = {0};
 uint8_t sig[64] = {0};
	 
 //uECC_set_rng(&RNG);

 const struct uECC_Curve_t * curve;
 int num_curves = 1;
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
 curve = uECC_secp256k1();
 #endif
 
 //printf("Testing 256 signatures\n");
 //for (c = 0; c < num_curves; ++c) {
		 printf("KZNUDVZIL");
		 //fflush(stdout);
		 
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
			 printf("uECC_sign() failed\n");
			 return 1;
		 }
		volatile int result = uECC_verify(public, hash, sizeof(hash), sig, curve);
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

int verify_signature(uint8_t challenge[64], uint8_t signature[64]){
	const struct uECC_Curve_t * curve = uECC_secp256k1();
	volatile uint8_t public[64] = {0};
	hex_to_bytes(public_key_hex, 128, public);
	int result = uECC_verify(public, challenge, 64, signature, curve);
	return result;
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
	//verify_signature(challenge, signature);
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


