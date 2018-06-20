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

#include "types.h"
#include "uart_f.h"
#include "hexconv.h"
#include "leds.h"
#include "can.h"
#include "ECC/uECC.h"
#include "permission_table.h"
#include "sha2/sha512.h"
#include "sha2/hmac-sha256.h"

volatile uint8_t data[8];

static char private_key_hex[64] = "92990788d66bf558052d112f5498111747b3e28c55984d43fed8c8822ad9f1a7";
static char public_key_hex[128] = "f0c863f8e555114bf4882cc787b95c272a7fe4dcddf1922f4f18a494e1c357a1a6c32d07beb576ab601068068f0a9e0160c3a14119f5d426a7955da3e6ed3e81";

const static uint8_t private_key[32] = { 146,153,7,136,214,107,245,88,5,45,17,47,84,152,17,23,71,179,226,140,85,152,77,67,254,216,200,130,42,217,241,167};
const static uint8_t public_key[64] = { 240,200,99,248,229,85,17,75,244,136,44,199,135,185,92,39,42,127,228,220,221,241,146,47,79,24,164,148,225,195,87,161,166,195,45,7,190,181,118,171,96,16,104,6,143,10,158,1,96,195,161,65,25,245,212,38,167,149,93,163,230,237,62,129	};


volatile state_t state = IDLE_S;
volatile event_t event = NULL_E;

void buttons_init(){
	DDRE = 0x00;
	PORTE = 1 << PE4 | 1 << PE5 | 1 << PE6 | 1 << PE7;
	EICRB = 1 << ISC40 | 1 << ISC41 | 1 << ISC50 | 1 << ISC51 | 1 << ISC60 | 1 << ISC61 | 1 << ISC70 | 1 << ISC71; // set interrupt on falling edge.
	EIMSK = 1 << INT4 | 1 << INT5 | 1 << INT6 | 1 << INT7;
}

static int RNG(uint8_t *dest, unsigned size) {
	while(size){
		uint8_t val = (uint8_t) rand() + rand();
		*dest = val;
		++dest;
		--size;
	}
	return 1;
}

int verify_signature(uint8_t challenge[64], uint8_t signature[64]){
	const struct uECC_Curve_t * curve = uECC_secp256r1();
	int result;
	uint8_t hash[64];
	sha512(hash, challenge,512);
	result = uECC_verify(public_key, hash, 64, signature, curve);
	return result;
}

int run_scenario1(permissions_t role)
{
	volatile uint8_t result;
	uart_puts("authentication started");
	can_buff_512_t challenge = { 0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 1 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 2 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 3 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 4 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 5 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 6 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 7 , 1 , 2 , 3 , 4 , 5 , 6 , 7 };
	can_send_frame_buffer(challenge , 8);
	uart_puts("challenge sent");
	can_msg_t signature;
	can_receive_frame_buffer(signature, 8);
	result = verify_signature(challenge, signature);
	if(result==1) {
		uart_puts("signature is valid!");
	}else{
		uart_puts("signature is false!");
	}
	can_msg_t ack;
	ack[0] = role;
	can_send_message(0, 0x00, ack);
	return 0;
}

int run_scenario2(permissions_t role){
	volatile uint8_t result;
	
	//---Shared Secret Establishment Starts Here---//
	uart_puts("authentication started");
	uECC_set_rng(RNG);
	volatile uint8_t private[32];
	volatile uint8_t public[64];
	const struct uECC_Curve_t * curve = uECC_secp256r1();
	result = uECC_make_key(public, private, curve);
	uint8_t secret_unhashed[32];
	result = uECC_shared_secret(public_key, private, secret_unhashed, curve);
	uint8_t secret[32];
	uint32_t len = 256;
	sha256(secret, secret_unhashed, len);
	//---Secret Value Establishment Ends Here---//
	
	can_send_frame_buffer(public, 8);
	uart_puts("secret established");
	can_init();
	//---Every iteration of this loop equals 1 message being authenticated using the shared secret---.
	while(1){
		can_id_t *id = malloc(sizeof(can_id_t));
		can_msg_t message;
		// Receive the message that the tester wants to send to the network.
		can_receive_message(0, 0x00, 0x00, message);
		can_get_id(0, id);
		// Acknowledge Correct Reception of the message by retransmitting it.
		can_send_message(0, 0x00, message);
		uint8_t mac[16];
		// Receive the MAC of the message
		can_receive_frame_buffer(mac,2);
		uint8_t mac2[32];
		hmac_sha256(mac2, secret, 265 ,message , 64);
		//Check the MAC.
		if(memcmp(mac, mac2, 16) == 0){
			uart_puts("Authentication Ok");
		}else{
			uart_puts("Authentication Failed");
		}
		//Check the Permission.
		if (check_permission(id, role) == 0){
			uart_puts("Permission Ok");
		}else{
			uart_puts("Permission Failed");
		}
	}
	return result;
	
}




 int main()
 {
	uart_init();
	uart_puts("test");
	buttons_init();
	can_init();
	init_permissions_table();
	volatile uint8_t id[2] = {0 , 0};
	char input[] = "0726";
	uart_puts(input);
	hex_to_bytes("0726", 4, id);
	bytes_to_hex(id, 2 , input);
	uart_puts(input);
	volatile int result;
	result = check_permission(id, OWNER_ROLE); 
	return result;
	//uart_puts("idle");
	can_msg_t init;
	//initial message value is used to determine what ROLE is used to authenticate.
	can_receive_message(0, 0x00, 0x00, init);
	switch((int) init[0]){
		case OWNER_ROLE :
			run_scenario2(OWNER_ROLE);
			break;
		case REPAIRSHOP_ROLE :
			run_scenario2(REPAIRSHOP_ROLE);
			break;
		case POLICEMAN_ROLE :
			run_scenario2(POLICEMAN_ROLE);
			break;
		case TESTER_ROLE :
			run_scenario2(TESTER_ROLE);
			break;	
		default : 
			break;
	}
 }


