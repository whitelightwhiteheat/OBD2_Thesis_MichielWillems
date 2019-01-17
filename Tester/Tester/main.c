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
#include "ISO-TP/isotp.h"
#include "ISO-TP/isotp_config.h"
#include "ISO-TP/isotp_defines.h"

#define F_CPU 8000000L
#include <util/delay.h>

const can_id_t default_id = {0x05, 0x05};

typedef enum {
	NOTHING,
	SCENARIO1,
	SCENARIO2,
	SCENARIO3
} run_t;

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
	EIMSK |= 1 << INT7;
}

int authenticate(can_msg_t *message, can_id_t *id, uint8_t role, uint8_t rounds){
	uart_puts("starting authentication");
	
	//init authentication.

	can_msg_t init = {0,0,0,0,0,0,0,0}; 
	init[0]= role;
	isotpi_send(default_id, 7, init);
	uint8_t public[64];
	
	can_id_t id_tmp;
	//Receive generated public key.
	isotpi_receive_multi(default_id, id_tmp ,64, public);
	uart_puts("public key received");
	
	//Calculate shared secret.
	uint8_t secret[32];
	if(run_scenario == SCENARIO1){
		calculate_shared_secret_dummy(public, role, secret);
		_delay_ms(1000);
		}else{
		calculate_shared_secret(public, role, secret);
	}
	
	//Sign the public key.
	uint8_t signature[64];
	if(run_scenario == SCENARIO1){
		sign_challenge_dummy(public, signature, role);
		_delay_ms(1000);
		}else{
		sign_challenge(public, signature, role);
	}
	
	//Send signature.
	isotpi_send_multi(default_id,64,signature);
	uart_puts("signature sent");
	uint8_t ack[8];
	
	//wait for acknowledgment.
	uint8_t len;
	isotpi_receive(default_id,7,ack);
	uart_putd(ack,8);
	if(ack[0] == ACK_POS){
		uart_puts("Successfully authenticated!");
		}else{
		uart_puts("Authentication failed!");
		return 0;
	}
	
	while(rounds > 0){
		rounds--;
		
		//Send message you want to send to the vehicle network.
		can_send_message(0, *id, *message, 8);
		
		//Wait for acknowledgment.
		can_msg_t ack;
		can_receive_message(0, default_id, 0x00, ack, &len);
		if(ack[0] == ACK_POS){
			uart_puts("permission granted!");
			}else{
			uart_puts("permission denied!");
			continue;
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
		can_receive_message(0, default_id, 0x00, ack, &len);
		if(ack[0] == ACK_POS){
			uart_puts("message accepted!");
		}else{
			uart_puts("message denied!");
			continue;
		}
		_delay_ms(500);
		id++;
		message++;
	}
	return 0;
}

static IsoTpLink g_link;

/* Alloc send and receive buffer statically in RAM */
static uint8_t g_isotpRecvBuf[64];
static uint8_t g_isotpSendBuf[64];



 int main()
 {	
	 uart_init();
	 buttons_init();
	 can_init();
	 uart_puts("test");
	 clock_Init();
	 /*
	 uint8_t ret;
	 uint8_t payload[28] = {1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9};
	 ret = isotpi_send_multi(default_id,28,payload);
	 uart_puts("sent");
	 uint8_t out_size;
	 can_id_t id_rec;
	 ret = isotpi_receive_multi(default_id,id_rec,28,payload);
	 uart_puts("received");
	 uart_puts(payload);
	 */
	
	
	 while(1){
		run_t runlcl = run_scenario;
		uint8_t msgs[3][8] = { {0,0,0,0,0,0,0,0} , {0,0,0,0,0,0,0,0} , {0,0,0,0,0,0,0,0} };
		uint8_t ids[3][2] = {{2,1},{4,2},{0,0}};
		switch(runlcl){
			case NOTHING :
				break;
				
			//SCENARIO1: Use wrong key.
			case SCENARIO1 :
				authenticate(msgs, ids, OWNER_ROLE,3);
				return 0;
				break;
				
			//SCENARIO2: Use owner private key (Some messages will be denied).
			case SCENARIO2 :
				authenticate(msgs, ids, OWNER_ROLE,3);
				return 0;
				break;
				
			//SCENARIO3: Use tester private key (all messages accepted).
			case SCENARIO3 :
				authenticate(msgs, ids, ADMIN_ROLE,3);
				return 0;
				break;
		}
	 }
	 return 0;
 }

