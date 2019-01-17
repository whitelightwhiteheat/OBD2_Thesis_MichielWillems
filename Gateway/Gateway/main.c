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

const char private_key1_hex[64] = "92990788d66bf558052d112f5498111747b3e28c55984d43fed8c8822ad9f1a7";
const char public_key1_hex[128] = "f0c863f8e555114bf4882cc787b95c272a7fe4dcddf1922f4f18a494e1c357a1a6c32d07beb576ab601068068f0a9e0160c3a14119f5d426a7955da3e6ed3e81";

const char private_key2_hex[64] = "48e5ca2a675ab49ca214b884813935024b0c61edc8d1305fe5230df341623348";
const char public_key2_hex[128] = "7bcc5dea40e5325cc62c1b642bdd258dc4897b22b2066e99286215c4d5603027246a7fb31f1181db961ca63657c4eabeedbe67bb02429d6942962452cfc6ec4f";

const char private_key3_hex[64] = "8e2958a1475ed70762340e9797788e0061f21fcebd67889fdd4f4ce2b5f6b2de";
const char public_key3_hex[128] = "ccfd9d101862143f83b3de5caccdf4a3aa55c0cdd8cd848b8416d2b7109488b9da60f3be17ddb00412eb4c64d422b3df3be978c41844195a890ec5e3249efe5e";

const char private_key4_hex[64] = "b2c950abc87a55442cc00f1e3ac38f81b7e95036fd191ea134ff616d9806e10c";
const char public_key4_hex[128] = "bd42371aac680a6603eba1cf0f5e495ace59299a4e2f42aa943b8406c42b66c8182c093637eb1199ec41dee8ba7a2faae07b04857b569033c7b73e318a1f6321";

const char private_key5_hex[64] = "b08039a19079d5218465f6d97552bd70b8867423d67365b8431b6f213a197471";
const char public_key5_hex[128] = "5d19b55cc3528aaf8664bb20c9a199567a2444b549ebfa11c721fd7fdce2d2b31571b5033932b1d14373f6860d5a97f6efe65470e547aa1c663bdbb57977378c";

static const can_id_t default_id = {0x05, 0x05};
static const can_mask_t zero_mask = {0x00, 0x00};
static const can_mask_t default_mask = {255, 255};
static const can_msg_t ack_pos = {ACK_POS};
static const can_msg_t ack_neg = {ACK_NEG};

volatile state_t state = IDLE_S;
volatile event_t event = NULL_E;

void buttons_init(){
	DDRE = 0x00;
	PORTE = 1 << PE4 | 1 << PE5 | 1 << PE6 | 1 << PE7;
	EICRB = 0x00;
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
	uint8_t hash[64];
	sha512(hash, challenge,512);
	uint8_t public[64];
	get_public_key(role, public);
	result = uECC_verify(public, hash, 64, signature, curve);
	return result;
}


int authenticated_key_agreement(role_t role, uint8_t *dest){
	PORTB = _BV(PB2);
	
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
	
	PORTB = 0x00;  
	//PORTB = _BV(PB3);
	
	//Wait for Signature.
	uint8_t signature[64];
	can_id_t idtmp;
	isotpi_receive_multi(default_id, idtmp, 64, signature);
	uart_puts("signature_received");
	
	//PORTB = 0x00;
	PORTB = _BV(PB2);
	
	uint8_t secret[32];
	
	//Verify Signature.
	result = verify_signature(public2, signature, role);
	
	PORTB = 0x00;
	can_msg_t ack;
	if(result==1) {
		uart_puts("signature is valid!");
		
		PORTB = _BV(PB2);
		
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
	memcpy(dest,secret,32); 
	return 0;
}

int message_authentication(role_t role, uint8_t	secret[32]){
	//---Every iteration of this loop equals 1 message being authenticated using the shared secret---.
	while(1){
		can_id_t id;
		can_msg_t message;
		
		PORTB = 0x00;
		
		uint8_t len;
		// Receive the message that the tester wants to send to the network.
		can_receive_message(0, default_id, zero_mask, message, &len);
		can_get_id(0, id);
		
		PORTB = _BV(PB2);
		
		//Check the Permission.
		if(check_permission(id, role) == 0){
			uart_puts("Permission Ok");
			can_send_message(0, default_id, ack_pos, 8);
		}else{
			uart_puts("Permission Failed");
			can_send_message(0, default_id, ack_neg, 8);
			continue;
		}
		
		// Acknowledge permission.
		uint8_t mac[16];
		
		PORTB = 0x00;
		
		// Receive the MAC of the message
		can_receive_frame_buffer(mac,2);
		
		PORTB = _BV(PB2);
		
		uint8_t mac2[32];
		uint16_t klen = 256;
		uint32_t msglen = 64;
		hmac_sha256(mac2, secret, klen ,message , msglen);
		
		//Check the MAC.
		if(memcmp(mac, mac2, 16) == 0){
			uart_puts("Authentication Ok");
			can_send_message(0, default_id, ack_pos, 8);
		}else{
			uart_puts("Authentication Failed");
			can_send_message(0, default_id, ack_neg, 8);
			continue;
		}
		
		PORTB = 0x00;
		
		//Forward the message to the internal vehicle network.
		forward_message(message, id);
	}
	return 0;
	
}

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

/* Alloc IsoTpLink statically in RAM */
static IsoTpLink g_link;

/* Alloc send and receive buffer statically in RAM */
static uint8_t g_isotpRecvBuf[128];
static uint8_t g_isotpSendBuf[128];


 int main()
 {
	uart_init();
	buttons_init();
	can_init();
	init_permissions_table();
	uart_puts("idle");
	clock_Init();
   	
	   /*
	uint8_t payload[28];
	uint8_t out_size;
	uint8_t id[2] = {0,0};
	uint8_t idrec[2];
	uint8_t ret = isotpi_receive_multi(default_id,idrec,28,payload);
	if(ret==0) uart_puts("transmission complete");
	uint8_t id2[2] = {0,0};	
	ret = isotpi_send_multi(default_id,28,payload);
    if(ret==0) uart_puts("receive complete");
	return 0;
	*/
	      
	
	can_msg_t init;
	uint8_t secret[32];
	uint16_t size = 1;
	while(1){
		isotpi_receive(default_id, 7, init);
		if(authenticated_key_agreement(init[0],secret)) return 0;
		if(message_authentication(init[0],secret)) return 0;
	}
	
 }


