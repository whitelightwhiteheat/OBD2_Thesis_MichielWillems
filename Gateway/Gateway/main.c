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


#include "types.h"
#include "uart_f.h"
#include "hexconv.h"
#include "leds.h"
#include "can.h"
#include "ECC/uECC.h"
#include "permission_table.h"
#include "sha2/sha512.h"
#include "sha2/hmac-sha256.h"
#include <util/delay.h>
#include "ISO-TP/isotp.h"
#include "ISO-TP/isotp_defines.h"
#include "ISO-TP/isotp_user.h"
#include "clock.h"
#include "isotp_interface.h"

const char public_key1_hex[128] = "f0c863f8e555114bf4882cc787b95c272a7fe4dcddf1922f4f18a494e1c357a1a6c32d07beb576ab601068068f0a9e0160c3a14119f5d426a7955da3e6ed3e81";

const char public_key2_hex[128] = "7bcc5dea40e5325cc62c1b642bdd258dc4897b22b2066e99286215c4d5603027246a7fb31f1181db961ca63657c4eabeedbe67bb02429d6942962452cfc6ec4f";

const char public_key3_hex[128] = "ccfd9d101862143f83b3de5caccdf4a3aa55c0cdd8cd848b8416d2b7109488b9da60f3be17ddb00412eb4c64d422b3df3be978c41844195a890ec5e3249efe5e";

const char public_key4_hex[128] = "bd42371aac680a6603eba1cf0f5e495ace59299a4e2f42aa943b8406c42b66c8182c093637eb1199ec41dee8ba7a2faae07b04857b569033c7b73e318a1f6321";

const char public_key5_hex[128] = "5d19b55cc3528aaf8664bb20c9a199567a2444b549ebfa11c721fd7fdce2d2b31571b5033932b1d14373f6860d5a97f6efe65470e547aa1c663bdbb57977378c";

static const can_id_t default_id = {0x05, 0x05};
static const can_mask_t zero_mask = {0x00, 0x00};
static const can_mask_t default_mask = {255, 255};
static const can_msg_t ack_pos = {ACK_POS};
static const can_msg_t ack_neg = {ACK_NEG};

volatile state_t state = IDLE_S;
volatile event_t event = NULL_E;

/*	RNG() function.
Random number generator function.
	
	Inputs:	
		size	- The size of the random data required.
		
	Outputs:
		dest	- The randomly generated data.

Note:	The RNG implemented here is only of demonstrative purposes.
		For use in cryptographic operations, this would have to be implemented
		using some inherently random input (e.g. noise). 
*/
static int RNG(uint8_t *dest, unsigned size) {
	while(size){
		uint8_t val = (uint8_t) rand() + rand();
		*dest = val;
		++dest;
		--size;
	}
	return 1;
}

// Retrieve the appropriate public key from memory
static void get_public_key(uint8_t role, uint8_t public[64]){
	switch (role)
	{
		case ADMIN_ROLE :
		hex_to_bytes(public_key1_hex, 128, public);
		break;
		case OEM_ROLE :
		hex_to_bytes(public_key2_hex, 128, public);
		break;
		case POLICEMAN_ROLE :
		hex_to_bytes(public_key3_hex, 128, public);
		break;
		case REPAIRMAN_ROLE :
		hex_to_bytes(public_key4_hex, 128, public);
		break;
		case OWNER_ROLE :
		hex_to_bytes(public_key5_hex, 128, public);
		break;
	}
}

int verify_signature(uint8_t challenge[64], uint8_t signature[64], role_t role){
	const struct uECC_Curve_t * curve = uECC_secp256r1();
	int result;
	
	//Hash first for added security.
	uint8_t hash[64];
	sha512(hash, challenge,512);
	uint8_t public[64];
	
	//Retrieve appropriate public key.
	get_public_key(role, public);
	
	//Verify ECDSA signature
	result = uECC_verify(public, hash, 64, signature, curve);
	return result;
}

/* authenticated_key_agreement() function.
Implements the authenticated key agreement procedure outlined in thesis.pdf.

	Inputs:
		role	- The role that the user wishes to authenticate as.
	Outputs:
		secret	- The ECDH shared secret that was calculated.
*/
int authenticated_key_agreement(role_t role, uint8_t *secret){
	
	//Start protocol by calculating new private/public key pair for shared secret establishment.
	volatile uint8_t result;
	uart_puts("authentication started");
	uECC_set_rng(RNG);
	volatile uint8_t private2[32];
	volatile uint8_t public2[64];
	const struct uECC_Curve_t * curve = uECC_secp256r1();
	result = uECC_make_key(public2, private2, curve);
	uint8_t secret_unhashed[32];
	uint8_t public[64];
	get_public_key(role, public);
	
	//Send new public key.
	isotpi_send_multi(default_id, 64, public2);
	uart_puts("public key sent");
	
	//Wait for Signature.
	uint8_t signature[64];
	can_id_t idtmp;
	isotpi_receive_multi(default_id, idtmp, 64, signature);
	uart_puts("signature received");
	
	//Verify Signature.
	result = verify_signature(public2, signature, role);
	
	//Handle result of signature verification
	can_msg_t ack;
	if(result==1) {
		uart_puts("signature is valid");
		
		//calculate shared secret.
		result = uECC_shared_secret(public, private2, secret_unhashed, curve);
		uint32_t len = 256;
		sha256(secret, secret_unhashed, len);
		
		isotpi_send(default_id, 7, ack_pos);
	}else{
		uart_puts("signature is false!");
		
		isotpi_send(default_id, 7, ack_neg);
		return 1;
	} 
	return 0;
}

/* message_authentication() function.
Implements the message authentication procedure outlined in thesis.pdf

	Inputs:
		role	- The role that the user wishes to authenticate as.
		secret	- The ECDH shared secret used to secure the session.
*/
int message_authentication(role_t role, uint8_t	secret[32]){
	//Every iteration of this loop equals 1 message being authenticated using the shared secret.
	while(1){
		can_id_t id;
		can_msg_t message;
		uint8_t len;
		// Receive the message that the tester wants to send to the network.
		isotpi_receive_multi(default_id, id, 8, message);
		
		//Check + acknwoledge the permission check.
		if(check_permission(id, role) == 0){
			uart_puts("Permission granted");
			isotpi_send(default_id, 7, ack_pos);
		}else{
			uart_puts("Permission denied");
			isotpi_send(default_id, 7, ack_neg);
			continue;
		}
		
		// Receive the MAC of the message
		can_id_t idtmp;
		uint8_t mac[32];
		isotpi_receive_multi(default_id, idtmp, 32, mac);
		
		// Calculate MAC
		uint8_t mac2[32];
		uint16_t klen = 256;
		uint32_t msglen = 64;
		hmac_sha256(mac2, secret, klen ,message , msglen);
		
		//Check the MAC.
		if(memcmp(mac, mac2, 32) == 0){
			uart_puts("Authentication Ok");
			isotpi_send(default_id, 7, ack_pos);
		}else{
			uart_puts("Authentication Failed");
			isotpi_send(default_id, 7, ack_neg);
			continue;
		}
		
		//Forward the message to the internal vehicle network.
		forward_message(message, id);
	}
	return 0;
	
}

// In a real gateway, this function would forward the message to the appropriate subnetwork.
void forward_message(can_msg_t msg, can_id_t id){
	char idl[2];
	char idh[2];
	bytes_to_hex(id, 1, idl);
	id++;
	bytes_to_hex(id, 1, idh);
	uart_puts("---MESSAGE FORWARDED ONTO NETWORK---");
	uart_puts("IDL:");
	uart_putd(idl,2);
	uart_puts("IDH:");
	uart_putd(idh,2);
	char msghex[16];
	bytes_to_hex(msg, 8, msghex);
	uart_puts("MESSAGE:");
	uart_putd(msghex, 16);
	uart_puts("------------------------------------");
}

 int main()
 {
	uart_init();
	can_init();
	init_permissions_table();
	uart_puts("idle");
	clock_Init();
   	
	can_msg_t init;
	uint8_t secret[32];
	while(1){
		isotpi_receive(default_id, 7, init);
		if(authenticated_key_agreement(init[0],secret)) return 0;
		if(message_authentication(init[0],secret)) return 0;
	}
	
 }


