/*
 * hexconv.c
 *
 * Created: 5/9/2018 2:19:41 AM
 *  Author: michel
 */ 

#include <avr/io.h>
#include "hexconv.h"


void hex_to_id(char hex[4], can_id_t *dest){
	uint8_t bytes[2];
	hex_to_bytes(hex,4,bytes);
	dest->idh = bytes[1];
	dest->idl = bytes[0];
}

void bytes_to_hex(const uint8_t *src, uint8_t len, char *dest)
{
	static const unsigned char table[] = "0123456789abcdef";

	for (; len > 0; --len)
	{
		unsigned char c = *src++;
		*dest++ = table[c >> 4];
		*dest++ = table[c & 0x0f];
	}
}

void hex_to_bytes(char* src, uint8_t slength, uint8_t* dest) {

	uint8_t index = 0;
	while (index < slength) {
		char c = src[index];
		int value = 0;
		if(c >= '0' && c <= '9')
		value = (c - '0');
		else if (c >= 'A' && c <= 'F')
		value = (10 + (c - 'A'));
		else if (c >= 'a' && c <= 'f')
		value = (10 + (c - 'a'));

		dest[(index/2)] += value << (((index + 1) % 2) * 4);

		index++;
	}
}

