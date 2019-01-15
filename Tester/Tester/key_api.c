/*
 * key_api.c
 *
 * Created: 6/21/2018 4:24:52 AM
 *  Author: michel
 */

#include "key_api.h"

const char private_key1_hex[64] = "92990788d66bf558052d112f5498111747b3e28c55984d43fed8c8822ad9f1a7";
const char public_key1_hex[128] = "f0c863f8e555114bf4882cc787b95c272a7fe4dcddf1922f4f18a494e1c357a1a6c32d07beb576ab601068068f0a9e0160c3a14119f5d426a7955da3e6ed3e81";

const char private_key2_hex[64] = "48e5ca2a675ab49ca214b884813935024b0c61edc8d1305fe5230df341623348";
const char public_key2_hex[128] = "7bcc5dea40e5325cc62c1b642bdd258dc4897b22b2066e99286215c4d5603027246a7fb31f1181db961ca63657c4eabeedbe67bb02429d6942962452cfc6ec4f";

const char private_key3_hex[64] = "8e2958a1475ed70762340e9797788e0061f21fcebd67889fdd4f4ce2b5f6b2de";
const char public_key3_hex[128] = "ccfd9d101862143f83b3de5caccdf4a3aa55c0cdd8cd848b8416d2b7109488b9da60f3be17ddb00412eb4c64d422b3df3be978c41844195a890ec5e3249efe5e";

const char private_key4_hex[64] = "b2c950abc87a55442cc00f1e3ac38f81b7e95036fd191ea134ff616d9806e10c";
const char public_key4_hex[128] = "bd42371aac680a6603eba1cf0f5e495ace59299a4e2f42aa943b8406c42b66c8182c093637eb1199ec41dee8ba7a2faae07b04857b569033c7b73e318a1f6321";

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
		case OWNER_ROLE :
			hex_to_bytes(private_key1_hex, 64, private);
			break;
		case REPAIRSHOP_ROLE :
			hex_to_bytes(private_key2_hex, 64, private);
			break;
		case POLICEMAN_ROLE :
			hex_to_bytes(private_key3_hex, 64, private);
			break;
		case TESTER_ROLE :
			hex_to_bytes(private_key4_hex, 64, private);
			break;
	}
} 

int sign_challenge(uint8_t challenge[64], uint8_t signature[64], uint8_t role){
	uECC_set_rng(&RNG);
	const struct uECC_Curve_t *curve = uECC_secp256r1();
	uint8_t hash[64];
	sha512(hash, challenge, 512);
	uint8_t private[32];
	get_private_key(role, private);

	if (!uECC_sign(private, hash, 64, signature, curve)) {
		uart_puts("sign failed");
		return 1;
	}
	
	return 0;
}


int sign_challenge_dummy(uint8_t challenge[64], uint8_t signature[64], uint8_t role){
	memcpy(signature,challenge,64);
	return 0;
}

int calculate_shared_secret(uint8_t public[64], uint8_t role, uint8_t secret[32]){
	const struct uECC_Curve_t * curve = uECC_secp256r1();
	uint8_t secret_unhashed[32];
	//Generate Shared Secret.
	uint8_t private[32];
	get_private_key(role, private);
	if(!uECC_shared_secret(public, private, secret_unhashed, curve)){
		uart_puts("shared secret creation failed");
		return 1;
	}
	uint32_t len = 256;
	sha256(secret, secret_unhashed, len);
	return 0;
}

int calculate_shared_secret_dummy(uint8_t public[64], uint8_t role, uint8_t secret[32]){
	memcpy(secret, public, 32);
	return 0;
}

