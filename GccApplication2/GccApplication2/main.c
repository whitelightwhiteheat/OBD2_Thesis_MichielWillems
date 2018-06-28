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
const can_mask_t zero_mask = {0x00, 0x00};
const can_mask_t default_mask = {255, 255};

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

volatile authetication_protocol_t auth_protocol = SESSION;

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

int authenticate_single(can_msg_t message, can_id_t id, uint8_t role){
	uart_puts("starting authentication");
	uint8_t challenge[64];
	
	//init authentication.
	can_msg_t init;
	init[0] = role;
	can_send_message(0, default_id, init);
	
	//wait for challenge.
	can_receive_frame_buffer(challenge, 8);
	uart_puts("challenge received");
	
	//Sign the challenge.
	uint8_t signature[64];
	if(run_scenario == SCENARIO1){
		sign_challenge_dummy(challenge, signature, role);
	}else{
		sign_challenge(challenge, signature, role);
	}
	
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

int authenticate_session(can_msg_t *message, can_id_t *id, uint8_t role, uint8_t rounds){
	uart_puts("starting authentication");
	uint8_t challenge[64];
	
	//init authentication.
	can_msg_t init;
	init[0] = role;
	can_send_message(0, default_id, init);
	uint8_t public[64];
	
	//Receive generated public key.
	can_receive_frame_buffer(public, 8);
	
	//Calculate shared secret.
	uint8_t secret[32];
	if(run_scenario == SCENARIO1){
		calculate_shared_secret_dummy(public, role, secret);
	}else{
		calculate_shared_secret(public, role, secret);
	}
	
	while(rounds > 0){
		
		//Send message you want to send to the vehicle network.
		can_send_message(0, *id, *message);
		
		//Wait for acknowledgment.
		can_msg_t ack;
		can_receive_message(0, default_id, 0x00, ack);
		if(ack[0] == ACK_POS){
			uart_puts("permission granted!");
			}else{
			uart_puts("permission denied!");
		}
		
		//Calculate and send Hmac of message.
		uint8_t mac[32];
		uint16_t klen = 256;
		uint32_t msglen = 64;
		hmac_sha256(mac, secret, klen ,message , msglen);
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
		rounds--;
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
					return 0;
					break;
				case SCENARIO2 :
					authenticate_single( msg, id, OWNER_ROLE);
					return 0;
					break;
				case SCENARIO3 :
					authenticate_single( msg, id, TESTER_ROLE);
					return 0;
					break;
			}
		}else{
			uint8_t msgs[3][8] = { {0,0,0,0,0,0,0,0} , {0,0,0,0,0,0,0,0} , {0,0,0,0,0,0,0,0} };
			uint8_t ids[3][2] = {{177,7},{177,7},{177,7}};
			switch(runlcl){
				case NOTHING :
					break;
				case SCENARIO1 :
					authenticate_session(msgs, ids, OWNER_ROLE,3);
					return 0;
					break;
				case SCENARIO2 :
					authenticate_session(msgs, ids, OWNER_ROLE,3);
					return 0;
					break;
				case SCENARIO3 :
					authenticate_session(msgs, ids, TESTER_ROLE,3);
					return 0;
					break;
			}
		}
	 }
	 return 0;
 }

