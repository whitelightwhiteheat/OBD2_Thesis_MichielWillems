/*
 * key_api.c
 *
 * Created: 6/21/2018 4:24:52 AM
 *  Author: michel
 */

#include "key_api.h"

/*	The key_api mimics the interface of the central server.
	The secret keys are stored here.
	Users should be able to authenticate by providing the appropriate credentials,
	thereby gaining access to the appropriate ECC functions.
*/


const char private_key1_hex[64] = "92990788d66bf558052d112f5498111747b3e28c55984d43fed8c8822ad9f1a7";

const char private_key2_hex[64] = "48e5ca2a675ab49ca214b884813935024b0c61edc8d1305fe5230df341623348";

const char private_key3_hex[64] = "8e2958a1475ed70762340e9797788e0061f21fcebd67889fdd4f4ce2b5f6b2de";

const char private_key4_hex[64] = "b2c950abc87a55442cc00f1e3ac38f81b7e95036fd191ea134ff616d9806e10c";

const char private_key5_hex[64] = "b08039a19079d5218465f6d97552bd70b8867423d67365b8431b6f213a197471";


int RNG(uint8_t *dest, unsigned size){
	while(size){
		uint8_t val = (uint8_t) rand();
		*dest = val;
		++dest;
		--size;
	}
	return 1;
}

static void get_private_key(uint8_t role, uint8_t private[32]){
	switch (role)
	{
		case ADMIN_ROLE :
			hex_to_bytes(private_key1_hex, 64, private);
			break;
		case OEM_ROLE :
			hex_to_bytes(private_key2_hex, 64, private);
			break;
		case POLICEMAN_ROLE :
			hex_to_bytes(private_key3_hex, 64, private);
			break;
		case REPAIRMAN_ROLE :
			hex_to_bytes(private_key4_hex, 64, private);
			break;
		case OWNER_ROLE :
			hex_to_bytes(private_key5_hex, 64, private);
			break;
	}
} 

int sign_challenge(uint8_t challenge[64], uint8_t signature[64], uint8_t role){
	
	// Set RNG.
	uECC_set_rng(&RNG);
	const struct uECC_Curve_t *curve = uECC_secp256r1();
	uint8_t hash[64];
	
	// Hash the challenge first for extra security.
	sha512(hash, challenge, 512);
	uint8_t private[32];
	
	// get the appropriate private key.
	get_private_key(role, private);

	if (!uECC_sign(private, hash, 64, signature, curve)) {
		uart_puts("sign failed");
		return 1;
	}
	
	return 0;
}


int sign_challenge_dummy(uint8_t challenge[64], uint8_t signature[64], uint8_t role){
	// Send back the challange as dummy signature.
	memcpy(signature,challenge,64);
	return 0;
}

int calculate_shared_secret(uint8_t public[64], uint8_t role, uint8_t secret[32]){
	const struct uECC_Curve_t * curve = uECC_secp256r1();
	uint8_t secret_unhashed[32];
	
	//Generate Shared Secret.
	uint8_t private[32];
	
	//Use the appropriate private key
	get_private_key(role, private);
	if(!uECC_shared_secret(public, private, secret_unhashed, curve)){
		uart_puts("shared secret creation failed");
		return 1;
	}
	uint32_t len = 256;
	
	//Hash the secret for extra security.
	sha256(secret, secret_unhashed, len);
	return 0;
}

int calculate_shared_secret_dummy(uint8_t public[64], uint8_t role, uint8_t secret[32]){
	//Send back the first part of the public key as dummy secret.
	memcpy(secret, public, 32);
	return 0;
}

