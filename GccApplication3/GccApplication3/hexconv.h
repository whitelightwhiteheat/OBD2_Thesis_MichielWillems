/*
 * hexconv.h
 *
 * Created: 5/9/2018 2:20:07 AM
 *  Author: michel
 */ 

#include <stdlib.h>

#ifndef HEXCONV_H_
#define HEXCONV_H_

void bytes_to_hex(const uint8_t *src, uint8_t len, char *dest);

void hex_to_bytes(char* src, uint8_t slength, uint8_t* dest);

#endif /* HEXCONV_H_ */