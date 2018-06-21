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
#include "types.h"
#include "key_api.h"

#define F_CPU 8000000L
#include <util/delay.h>

const can_id_t default_id = {0x00, 0x00};

//const static uint8_t private_key[32] = { 146,153,7,136,214,107,245,88,5,45,17,47,84,152,17,23,71,179,226,140,85,152,77,67,254,216,200,130,42,217,241,167};
//const static uint8_t public_key[64] = { 240,200,99,248,229,85,17,75,244,136,44,199,135,185,92,39,42,127,228,220,221,241,146,47,79,24,164,148,225,195,87,161,166,195,45,7,190,181,118,171,96,16,104,6,143,10,158,1,96,195,161,65,25,245,212,38,167,149,93,163,230,237,62,129	};

typedef enum {
	NOTHING,
	SCENARIO1,
	SCENARIO2,
	SCENARIO3
} run_t;

typedef enum {
	SINGLE,
	SESSION
} authetication_protocol_t;

volatile authetication_protocol_t auth_protocol = SINGLE;

volatile run_t run_scenario = NOTHING;
		
void buttons_init(){
	DDRE = 0x00;
	PORTE = 1 << PE4 | 1 << PE5 | 1 << PE6 | 1 << PE7;
	EICRB = 0x00;
	//EICRB = 1 << ISC40 | 1 << ISC41 | 1 << ISC50 | 1 << ISC51 | 1 << ISC60 | 1 << ISC61 | 1 << ISC70 | 1 << ISC71; // set interrupt on falling edge.
	EIMSK = 1 << INT4 | 1 << INT5 | 1 << INT6 | 1 << INT7;
	
}
	
ISR(INT4_vect){
	EIMSK &= ~(1 << INT4);
	_delay_ms(500);
	uart_puts("running scenario 1");
	run_scenario = SCENARIO1;
	EIMSK |= 1 << INT4;
}

ISR(INT5_vect){
	EIMSK &= ~(1 << INT5);
	_delay_ms(500);
	uart_puts("running scenario 2");
	run_scenario = SCENARIO2;
	EIMSK |= 1 << INT5;
}

ISR(INT6_vect){
	EIMSK &= ~(1 << INT6);
	_delay_ms(500);
	uart_puts("running scenario 3");
	run_scenario = SCENARIO3;
	EIMSK |= 1 << INT6;
}

ISR(INT7_vect){
	EIMSK &= ~(1 << INT7);
	_delay_ms(500);
	switch(auth_protocol){
		case SINGLE :
			auth_protocol = SESSION;
			uart_puts("Switched to session Authentication protocol.");
			break;
		case SESSION :
			auth_protocol = SINGLE;
			uart_puts("Switched to single Authentication protocol.");
			break;
	}
	EIMSK |= 1 << INT7;
}

/*
void test(){
	uECC_set_rng(&RNG);
	const struct uECC_Curve_t *curve = uECC_secp256r1();
	
	while(1){
		
		volatile uint8_t private[32];
		volatile uint8_t public[64];
		hex_to_bytes(private_key3_hex, 64, private);
		hex_to_bytes(public_key3_hex, 128, public);
		
		uint8_t challenge[64];
		RNG(challenge, 64);
		uint8_t hash[64];
		sha512(hash, challenge, 512);
		uint8_t signature[64];

		if (!uECC_sign(private, hash, 64, signature, curve)) {
			uart_puts("sign failed");
		}
		
		if (!uECC_verify(public, hash, 64, signature, curve)) {
			uart_puts("verify failed");
		}
		
		uart_puts("success");

		uint8_t private[32];
		uint8_t public[64];
		volatile uint8_t result = uECC_make_key(public, private, curve);
		uart_puts(result);
		char private_hex[64] = "";
		char public_hex[128] = "";
		bytes_to_hex(private, 32, private_hex);
		bytes_to_hex(public, 64, public_hex);
		uart_putsl(private_hex, 64);
		uart_putsl(public_hex, 128);
	
		
	}
}
*/

int authenticate_single(can_msg_t message, can_id_t id, uint8_t role){
	uart_puts("starting authentication");
	uint8_t challenge[64];
	//init authentication.
	can_msg_t init;
	init[0] = role;
	can_send_message(0, default_id, role);
	//wait for challenge.
	can_receive_frame_buffer(challenge, 8);
	uart_puts("challenge received");
	uint8_t signature[64];
	sign_challenge(challenge, signature, role);
	//Send signature.
	can_send_frame_buffer(signature, 8);
	uart_puts("signature sent");
	uint8_t ack[8];
	//Wait for authentication acknowledgment.
	can_receive_message(0, default_id, 0x00,  ack);
	if(ack[0] == ACK_POS){
		uart_puts("authenticated!");
	}else{
		uart_puts("authentication failed!");
		return 1;
	}
	//Send message
	can_send_message(0, id, message);
	//Wait for acknowledgment.
	can_receive_message(0, default_id, 0x00, ack);
	if(ack[0] == ACK_POS){
		uart_puts("message accepted!");
	}else{
		uart_puts("message denied!");
		return 1;
	}
	return 0;
}

int authenticate_shared_secret(can_msg_t *message, can_id_t *id, uint8_t role){
	uart_puts("starting authentication");
	uint8_t challenge[64];
	//init authentication.
	can_msg_t init;
	init[0] = role;
	can_send_message(0, default_id, role);
	uint8_t public[64];
	//Receive generated public key.
	can_receive_frame_buffer(public, 8);
	uint8_t secret[32];
	calculate_shared_secret(public, role, secret);
	/*
	char secret_hex2[64];
	bytes_to_hex(secret2, 32, secret_hex2);
	uart_puts(secret_hex2);
	*/
	while(message != NULL && id != NULL){
		//Send message you want to send to the vehicle network.
		can_send_message(0, *id, *message);
		//Wait for acknowledgment.
		can_msg_t ack;
		can_receive_message(0, default_id, 0x00, ack);
		//Calculate and send Hmac of message.
		uint8_t mac[32];
		hmac_sha256(mac, secret, 265 ,message , 64);
		uint8_t mac2[16];
		memcpy(mac2, mac ,16);
		can_send_frame_buffer(mac2, 2);
		//wait for acknowledgment.
		can_receive_message(0, default_id, 0x00, ack);
		if(ack[0] == ACK_POS){
			uart_puts("message accepted!");
		}else{
			uart_puts("message denied!");
		}
		id++;
		message++;
	}
	return 0;
}

 int main()
 {	
	 
	 uart_init();
	 buttons_init();
	 can_init();
	 
	 
	 while(1){
		run_t runlcl = run_scenario;
		authetication_protocol_t protocol = auth_protocol;
		
		if(protocol == SINGLE){
			can_msg_t msg = {0, 0, 0, 0, 0, 0, 0, 0};
			can_id_t id = {177, 7};
			switch(runlcl){
				case NOTHING :
					break;
				case SCENARIO1 :
					authenticate_single( msg, id, OWNER_ROLE);
					break;
				case SCENARIO2 :
					authenticate_single( msg, id, OWNER_ROLE);
					break;
				case SCENARIO3 :
					authenticate_single( msg, id, TESTER_ROLE);
					break;
			}
		}else{
			switch(runlcl){
				case NOTHING :
					break;
				case SCENARIO1 :
					//run_1();
					break;
				case SCENARIO2 :
					//run_2();
					break;
				case SCENARIO3 :
					//run_2();
					break;
			}
		}
	 }
	 return 0;
 }

