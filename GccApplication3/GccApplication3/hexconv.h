/*
 * hexconv.h
 *
 * Created: 5/9/2018 2:20:07 AM
 *  Author: michel
 */ 


#ifndef HEXCONV_H_
#define HEXCONV_H_

void bytes_to_hex(const unsigned char *src, uint8_t len, unsigned char *dest);

void hex_to_bytes(char* src, uint8_t* dest);

#endif /* HEXCONV_H_ */