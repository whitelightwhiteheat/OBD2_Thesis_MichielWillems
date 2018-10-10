
// This is a RSA implementation for the Mega or AVR4 core that is a at90s8515 
// By Emile van der Laan.
// This is not free ware, but if you use this commercially you will need to give a at least 100 Euro and 0.10 euro per device to a GNU project.
// It uses AVR Studio 4.12.452,and WINAVR 
// And WinAVR - 20050214

// 1520 Bytes of Flash used for both the decrypt and encrypt 
// 202 msec on a at90s8515 @ 8 Mc and 512 Bits and exponent of 3 
// 174 msec on a ATMEGA32  @ 8 Mc and 512 Bits and exponent of 3
// 76.6 sec on a at90s8515 @ 8 Mc and 512 Bits and exponent of 512 bits
// 65.4 sec on a ATMEGA32  @ 8 Mc and 512 Bits exponent of 512 bits

#include "rsa.h"
#include "rsa_asm.h"
#include <avr\pgmspace.h>
#include <string.h>



unsigned char s[RSALEN];
unsigned char rsa_tmp[2*RSALEN];

// 202 msec on a at90s8515 @ 8 Mc and 512 Bits
// 174 msec on a ATMEGA32  @ 8 Mc and 512 Bits
// 1520 Bytes of Flash used

void rsa(unsigned char   *a ,unsigned char e,unsigned char   *d)
{
unsigned char flag;
flag=2;
/* s = 1 */
/* a = a^e%d */
while(e!=0)
  {
	if(0!=(e&1))
	  {/*   s=(s*a)%d; */
	   if(flag)
		  { 
	  		flag=0;
			rsa_movel(s,a,RSALEN,RSALEN);                  
             /*  modulo(tmp,d,s,A_LEN,D_LEN,S_LEN);*/ /* S = A % D */
		  }
		else
		  {
		  	rsa_mull(a,s,rsa_tmp,RSALEN,RSALEN,2*RSALEN); /* TMP = A*S   */                  
		  	rsa_modulo(rsa_tmp,d,s,2*RSALEN,RSALEN,RSALEN); /* S = TMP % D */
		  }
	  }
	e>>=1;
	/* a=(a*a)%d   */
	if(e) /* A is not used is e == 0 */
	{
	  rsa_mull(a,a,rsa_tmp,RSALEN,RSALEN,2*RSALEN); /* TMP = A * A    */
	  rsa_modulo(rsa_tmp,d,a,2*RSALEN,RSALEN,RSALEN); /* A   = TMP % D  */
    }
  }
rsa_movel(a,s,RSALEN,RSALEN);
}


//  
// 76.6 sec on a at90s8515 @ 8 Mc and 512 Bits
// 65.4 sec on a ATMEGA32  @ 8 Mc and 512 Bits
//

void rsa_inc(unsigned char  *a ,unsigned char *e,unsigned char  *d)
{
unsigned char flag;
flag=2;
/* s = 1 */
/* a = a^e%d */
while(rsa_memtst(e,RSALEN)!=0)
  {
	if(0!=(e[RSALEN-1]&1))
	  {/*   s=(s*a)%d; */
	   if(flag)
		  { flag=0;
			rsa_movel(s,a,RSALEN,RSALEN);
		  }
		else
		  {
		  rsa_mull(a,s,rsa_tmp,RSALEN,RSALEN,2*RSALEN); /* TMP = A*S   */
		  rsa_modulo(rsa_tmp,d,s,2*RSALEN,RSALEN,RSALEN); /* S = TMP % D */
		  }
	  }
	rsa_ror(e,RSALEN,0);
	/* a=(a*a)%d   */
	if(e) /* A is not used is e == 0 */
	 {
	  rsa_mull(a,a,rsa_tmp,RSALEN,RSALEN,2*RSALEN); /* TMP = A * A    */
	  rsa_modulo(rsa_tmp,d,a,2*RSALEN,RSALEN,RSALEN); /* A   = TMP % D  */
	 }
  }
rsa_movel(a,s,RSALEN,RSALEN);
}


