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

const can_id_t default_id = {0x00, 0x00};
const can_mask_t zero_mask = {0x00, 0x00};
const can_mask_t default_mask = {255, 255};

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
	can_msg_t init;
	init[0] = role;
	can_send_message(0, default_id, init,1);
	uint8_t public[64];
	
	//Receive generated public key.
	can_receive_frame_buffer(public, 8);
	
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
	can_send_frame_buffer(signature, 8);
	uart_puts("signature sent");
	uint8_t ack[8];
	
	//wait for acknowledgment.
	uint8_t len;
	can_receive_message(0, default_id, 0x00, ack, &len);
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
	 
	 isotp_init_link(&g_link, 0x00,
					g_isotpSendBuf, sizeof(g_isotpSendBuf), 
					g_isotpRecvBuf, sizeof(g_isotpRecvBuf));
    
    while(1){
		uint8_t ret;
		uint8_t payload[28] = {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
		
        
        /* And send message with isotp_send */
        ret = isotp_send(&g_link, payload, 28);
        if (ISOTP_RET_OK == ret){
            /* Send ok */
        } else {
            /* Error occur */
        }
		can_msg_t message;
		uint8_t len;
		while(g_link.send_status == ISOTP_SEND_STATUS_INPROGRESS){
			can_receive_message(0,default_id,zero_mask,message,&len);
			isotp_on_can_message(&g_link,message,len);
			_delay_ms(200);
			isotp_poll(&g_link);	
		}
		return len;
		while(1){}
    }
	 
	/*
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
	 */
 }

