/*
 * GccApplication1.c
 *
 * Created: 3/1/2018 9:38:21 PM
 * Author : michel
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>


#include "uECC.h"


volatile uint8_t data[8];
volatile uint8_t read =0;
 
void CAN_INIT(void){

cli();

// no overload frame request
// no listening mode
// no test mode
// standby mode
// software reset request
CANGCON = (1 << SWRES);
   
// enable receive interrupt
// enable general interrupt
CANGIE = (1 << ENIT) | (1 << ENRX);

// MOb 1 interrupt enable
// MOb======> Message object  
CANIE2 = (1 << IEMOB1);     

// MOb 8 to 14 interrupt disable
CANIE1 = 0x00 ; /*MOb 8~14*/

// can general interrupt register
CANGIT = CANGIT;

// MOb initialization
int c;
for (c=0;c<15;c++) 
   { 
       
         CANPAGE = c << 4; 
         CANCDMOB = 0x00; 
         CANSTMOB = 0x00; 
    
   } 

// CAN bit timing registers
// set the timing (baud) ? 125 KBaud with 16 Mhz clock 
CANBT1 = 0x0e ;    // Baud Rate Prescaler
CANBT2 = 0x0c ;    // Re-Synchronization & Propagation 
CANBT3 = 0x37 ;    // Phase Segments & Sample Point(s) 
// CAN Timer Clock Period: 0.500 us 
CANTCON = 0x00 ;

// enter enable mode
CANGCON = 0b00000010 ;    // (1 << ENA/STB)

// MOb 1 initialization

int j;

CANPAGE = 0b00010000;    // (1 << MOBNB0)
CANIDT1 = 0b01111101; // ID = 1000 
CANIDT2 = 0x00; 
CANIDT3 = 0x00; 
CANIDT4 = 0x00; 
CANIDM1 = 0b11111111;  // no mask 
CANIDM2 = 0b11100000; 
CANIDM3 = 0x00; 
CANIDM4 = 0x00; 

for(j=0; j<8; j++){
   data[j]= CANMSG;
   } 

//CAN standard rev 2.0 A (identifiers length = 11 bits) 
// (1 << CONMOB1) | (1 << DLC3)
CANCDMOB = 0b10001000; //enable reception and data length code = 8 bytes

sei();

}

void ReceiveByMOb1(void){

int j;

CANPAGE = 0b00010000;  // (1 << MOBNB0)
CANIDT1 = 0b01111101; // ID = 1000 
CANIDT2 = 0x00; 
CANIDT3 = 0x00; 
CANIDT4 = 0x00; 
CANIDM1 = 0b11111111;  // no mask 
CANIDM2 = 0b11100000; 
CANIDM3 = 0x00; 
CANIDM4 = 0x00; 

for(j=0; j<8; j++){
   data[j]= CANMSG;
   } 

//CAN standard rev 2.0 A (identifiers length = 11 bits) 
// (1 << CONMOB1) | (1 << DLC3)
CANCDMOB = 0b10001000; //enable reception and data length code = 8 bytes 

}

void SendByMOb2(void){

int j;
   
    // If the last MOb2 Tx failed to complete, just skip it since we should not 
   // write to MOb2 CANCDMOB with a new command while MOb2 is still busy. 
   if ((CANEN2 & (1 << ENMOB2)) != 0) { 
      
      return; 
   } 
    
   CANPAGE = 0b00100000;  // (1 << MOBNB1)
   CANIDT1 = 0b00101001; // ID = 0x14a 
   CANIDT2 = 0b01000000; 
   CANIDT3 = 0x00; 
   CANIDT4 = 0x00; 
   CANIDM1 = 0x00;  // no mask 
   CANIDM2 = 0x00; 
   CANIDM3 = 0x00; 
   CANIDM4 = 0x00;
    
   for(j=0; j<8; j++){
   //CANMSG = data[j];
   CANMSG = j;
   } 

   //  (1 << CONMOB0) | (1 << DLC3)
   CANCDMOB = 0b01001000; //enable transmission!! , data length =8

   CANSTMOB = 0x00;      // clear flag

}

ISR (CANIT_vect){

// I put this line to check if i get a Rx ok or not
// but i don't (Led doesn't turn on) 
// and the code doesn't enter the ISR() 
PORTA &= 0b01111111;   // turn led on pin 7 on

uint8_t ctemp; 
ctemp=CANPAGE; // Save the pre-ISR CANPAGE value 
CANPAGE=0b00010000;  // (1 << MOBNB0) ,Select MOb1 
read=1; 
CANSTMOB=0x00; // Clear the MOb1 RXOK interrupt flag.
//CANEN2.ENMOB1 is automatically cleared by the MOb1 RXOK. 
CANPAGE=ctemp; // Restore the pre-ISR CANPAGE value 

}

void LightLed(void){
	DDRA |= (1 << DDA0) | (1 << DDA1); // set pin 0 and 1 for output
	PORTA |= (1 << PA0) | (1 << PA1);  // leds 0 & 1 on
}

static int RNG(uint8_t *dest, unsigned size) {
	while(size){
		uint8_t val = (uint8_t) rand();
		*dest = val;
		++dest;
		--size;
	}

	// NOTE: it would be a good idea to hash the resulting random data using SHA-256 or similar.
	return 1;
}


/****************************************************************************
 * The main program
 ****************************************************************************/
int main(void) {
	
	printf("kaka");

    CAN_INIT();
	  
    //sei();  // enable interrupt globally
	
	/*
	while(1){
 
    if(read==1){
    
    read=0;
    SendByMOb2();
	ReceiveByMOb1();

	}
    
	}
	*/
	
	uint8_t i, c;
	uint8_t private[32] = {0};
	uint8_t public[64] = {0};
	uint8_t hash[32] = {0};
	uint8_t sig[64] = {0};

	const struct uECC_Curve_t *curves[5];
	uint8_t num_curves = 0;
	#if uECC_SUPPORTS_secp160r1
	curves[num_curves++] = uECC_secp160r1();
	#endif
	#if uECC_SUPPORTS_secp192r1
	curves[num_curves++] = uECC_secp192r1();
	#endif
	#if uECC_SUPPORTS_secp224r1
	curves[num_curves++] = uECC_secp224r1();
	#endif
	#if uECC_SUPPORTS_secp256r1
	curves[num_curves++] = uECC_secp256r1();
	#endif
	#if uECC_SUPPORTS_secp256k1
	curves[num_curves++] = uECC_secp256k1();
	#endif
	
	uECC_set_rng(&RNG);
	
	printf("Testing 256 signatures\n");
	for (c = 0; c < num_curves; ++c) {
		for (i = 0; i < 256; ++i) {
			printf(".");
			fflush(stdout);

			if (!uECC_make_key(public, private, curves[c])) {
				printf("uECC_make_key() failed\n");
				return 1;
			}
			memcpy(hash, public, sizeof(hash));
			
			if (!uECC_sign(private, hash, sizeof(hash), sig, curves[c])) {
				printf("uECC_sign() failed\n");
				return 1;
			}

			if (!uECC_verify(public, hash, sizeof(hash), sig, curves[c])) {
				printf("uECC_verify() failed\n");
				return 1;
			}
		}
		printf("\n");
	}

    
	return 1;
	
}
