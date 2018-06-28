/*
 * key_api.h
 *
 * Created: 6/21/2018 4:35:15 AM
 *  Author: michel
 */ 


#ifndef KEY_API_H_
#define KEY_API_H_

#include <avr/io.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "ECC/uECC.h"
#include "hexconv.h"
#include "sha2/sha512.h"
#include "types.h"

int RNG(uint8_t *dest, unsigned size);

int sign_challenge(uint8_t challenge[64], uint8_t signature[64], uint8_t role);

int sign_challenge_dummy(uint8_t challenge[64], uint8_t signature[64], uint8_t role);

int calculate_shared_secret(uint8_t public[64], uint8_t role, uint8_t secret[32]);

int calculate_shared_secret_dummy(uint8_t public[64], uint8_t role, uint8_t secret[32]);


#endif /* KEY_API_H_ */