#ifndef _RSA_ASM_H_
#define _RSA_ASM_H_

unsigned char 	rsa_add1   (unsigned char	*a,unsigned char al);
unsigned char 	rsa_cmpl   (unsigned char	*a,unsigned char *b,unsigned char al);
unsigned char 	rsa_rol    (unsigned char	*a,unsigned char al,unsigned char carry);
unsigned char 	rsa_ror    (unsigned char	*a,unsigned char al,unsigned char carry);
unsigned char	rsa_memtst (unsigned char	*a,unsigned char al);
void  			rsa_modulo (unsigned char   *a,unsigned char *b,unsigned char *c,unsigned char al,unsigned char bl,unsigned char cl);
void  			rsa_clrl   (unsigned char   *a,unsigned char al);
void  			rsa_mull   (unsigned char   *a,unsigned char *b,unsigned char *c,unsigned char al,unsigned char bl,unsigned char cl);
unsigned char 	rsa_addl   (unsigned char   *a,unsigned char *b,unsigned char al);
void  			rsa_subl   (unsigned char   *a,unsigned char *b,unsigned char al);
void 			rsa_movel  (unsigned char   *d,unsigned char *s,unsigned char ld,unsigned char ls);
unsigned int 	rsa_mov_ac (unsigned char   *a,unsigned char *b,unsigned char *c,unsigned char al,unsigned char bl,unsigned char cl);

#endif
