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
#include <avr\io.h>
 
	.global asm_rsa
	.global asm_rsa_inc
	.extern tWETS  
#define __zero_reg__ R1
	.data			; switches to the .data section (initialized RAM variables)

	.text			; switches to the .text section (code and ROM constants)

	.global rsa_add1	; (unsigned char   *a,unsigned char   al);
	.global rsa_cmpl 	; (unsigned char   *a,unsigned char   *b,unsigned char al);
	.global rsa_rol  	; (unsigned char   *a,unsigned char   al,unsigned char carry);
	.global rsa_ror		; (unsigned char   *a,unsigned char   al,unsigned char carry);
	.global rsa_memtst	; (unsigned char   *a,unsigned char   al)
	.global rsa_modulo	; (unsigned char   *a,unsigned char   *b,unsigned char   *c,unsigned char al,unsigned char bl,unsigned char cl);
	.global rsa_clrl	; (unsigned char   *a,unsigned char   al);
	.global rsa_mull 	; (unsigned char   *a,unsigned char   *b,unsigned char   *c,unsigned char al,unsigned char bl,unsigned char cl);
	.global rsa_addl 	; (unsigned char   *a,unsigned char   *b,unsigned char al);
	.global rsa_subl 	; (unsigned char   *a,unsigned char   *b,unsigned char al);
	.global rsa_movel	; (unsigned char   *d,unsigned char   *s,unsigned char ld,unsigned char ls);
	.global rsa_mov_ac	; (unsigned char   *a,unsigned char   *b,unsigned char   *c,unsigned char al,unsigned char bl,unsigned char cl)

; C calling  unsigned int rsa_mov_ac(unsigned char   *a,unsigned char   *b,unsigned char   *c,unsigned char al,unsigned char bl,unsigned char cl)
;	*a is in R25:R24 
;	*b is in R23:R22 
;	*c is in R21:R20 
;	al is the size in R18	;
;	bl is the size in R16	;
;	cl is the size in R14	;
; RETURN r25:r24
rsa_mov_ac:
 		push r2
 		push r3
 		push r4
 		push r5
 		push r6
 		push r7
 		push r8
 		push r9
 		push r10
 		push r11
 		push r12
 		push r13
 		push r14
 		push r15
 		push r16
 		push r17
 		push r28
 		push r29
 		mov r6,r24	 ;  a, a
 		mov r7,r25	 ;  a, a
 		mov r27,r23	 ;  b, b
 		mov r26,r22	 ;  b, b
 		mov r2,r20	 ;  c, c
 		mov r3,r21	 ;  c, c
 		mov r17,r18	 ;  al, al
 		mov r4,r14	 ;  cl, cl
 		mov r8,r16	 ;  bl, bl
 		clr r9	 	 ;  bl
 		mov r19,r9	 ;  ts, bl
 		mov r18,r8	 ;  ts, bl
 		dec r18
 		sbc r19,__zero_reg__
 		lsl r18	 ;  ts
 		rol r19	 ;  ts
 		lsl r18	 ;  ts
 		rol r19	 ;  ts
 		lsl r18	 ;  ts
 		rol r19	 ;  ts
 		mov r5,__zero_reg__	 ;  s
 		mov r28,__zero_reg__ ;  tc,
 		mov r29,__zero_reg__ ;  tc,
.L6:
		cp r28,r18	 ;  tc, ts
		cpc r29,r19	 ;  tc, ts
 		brsh .L3	 ; ,
  		ld r24,X+	 ;  tmp94,* b	
 		tst r24	 	 ;  tmp94
 		brne .L3	 ; ,
 		inc r5	 	 ;  s
		adiw r28,8	 ;  tc,
 		rjmp .L6	 ; 
.L3:
 		inc r5	 ;  s
 		adiw r28,8	 ;  tc,
 		mov r12,r5	 ;  s, s
 		clr r13	 ;  s
 		mov r10,r4	 ;  cl, cl
 		clr r11	 ;  cl
 		mov r15,r11	 ;  tmp59, cl
 		mov r14,r10	 ;  tmp59, cl
 		sub r14,r12	 ;  tmp59, s
 		sbc r15,r13	 ;  tmp59, s
 		mov r21,r15	 ; , tmp59
 		mov r20,r14	 ; , tmp59
 		mov r23,r7	 ;  a, a
 		mov r22,r6	 ;  a, a
 		mov r25,r13	 ; , s
 		mov r24,r12	 ; , s
 		add r24,r2	 ; , c
 		adc r25,r3	 ; , c
 		rcall memcpy	 ; 
 		mov r16,r17	 ;  tmp69, al
 		clr r17	 ;  tmp69
 		sub r16,r10	 ;  tmp69, cl
 		sbc r17,r11	 ;  tmp69, cl
 		add r16,r12	 ;  tmp69, s
 		adc r17,r13	 ;  tmp69, s
 		add r14,r6	 ;  tmp59, a
 		adc r15,r7	 ;  tmp59, a
 		mov r21,r17	 ; , tmp69
 		mov r20,r16	 ; , tmp69
 		mov r23,r15	 ; , tmp59
 		mov r22,r14	 ; , tmp59
 		mov r25,r7	 ;  a, a
 		mov r24,r6	 ;  a, a
 		rcall memcpy	 ; 
 		sub r4,r5	 ;  cl, s
 		mov r22,r4	 ; , cl
 		mov r25,r7	 ; , a
 		mov r24,r6	 ; , a
 		add r24,r16	 ; , tmp69
 		adc r25,r17	 ; , tmp69
 		rcall rsa_clrl	 ; 
 	 	lsl r8	 
 		rol r9	 
 	 	lsl r8	 
 		rol r9	 
 	 	lsl r8	 
 		rol r9	 
 		mov r25,r9	 
 		mov r24,r8	 
 		sub r24,r28	 
 		sbc r25,r29	 
 		pop r29
 		pop r28
 		pop r17
 		pop r16
 		pop r15
 		pop r14
 		pop r13
 		pop r12
 		pop r11
 		pop r10
 		pop r9
 		pop r8
 		pop r7
 		pop r6
 		pop r5
 		pop r4
 		pop r3
 		pop r2
 		ret


; C calling  rsa_add1(unsigned char   *a,unsigned char al);
;	*a is in R25:R24 
;	al is the size in R22	;
; 	This function will add 1 to a
rsa_add1:
	mov r31,r25
	mov r30,r24
	add r30,r22
	adc r31,__zero_reg__ 
RSAADD1:
	ld r24,-Z
	inc R24
	st Z,R24
	BRNE RSAADD0
	DEC R22
	BRNE RSAADD1
	ldi R24,1
	rjmp RSAADD2
RSAADD0:
	CLR R24
RSAADD2:
	CLR R25
	ret


; C calling  rsa_cmpl(unsigned char   *a,unsigned char   *b,unsigned char al);
;	*a is in R25:R24 
;	*b is in R23:R22 
;	al is the size in R20	;
; 	This function will compare 2 longs
rsa_cmpl:
	mov R31,R23	; X
	mov R30,R22
	mov R27,R25	; Z
	mov R26,R24
RSACMPL2:
	ld R25,x+
	ld R24,z+
	CP R24,R25
	brlo RSACMPL0
	CP R25,R24
	brlo RSACMPL1
	dec R20
	BRNE RSACMPL2
	rjmp RSACMPL0
RSACMPL1:
	ldi R24,1
	rjmp RSACMPL3
RSACMPL0:
	CLR R24
RSACMPL3:
	CLR R25
// R25/R24	return;
	ret 	 	

; C calling  rsa_rol  	; (unsigned char   *a,unsigned char al,unsigned char carry);
;	*a is in R25:R24 
;	al is the size in R22	
;	Carry is R20
; 	This function will rol a long

rsa_rol:
// R25/R24 	= *a
// R22 		= al
// R20 		= carry
	mov R31,R25
	mov R30,R24
	add R30,R22
	adc R31,__zero_reg__
	mov R25,R20
	ROL R20
RSAROL1:
	ld R24,-Z
	ROL R24
	st Z,R24
	dec R22
	BRNE RSAROL1
	mov R24,R25
	rol R24
	mov R25,__zero_reg__
//  R25/R24	= return
	ret   	

; C calling  rsa_ror(unsigned char   *a,unsigned char al,unsigned char carry);
;	*a is in R25:R24 
;	al is the size in R22	
;	Carry is R20
; 	This function will ror a long
rsa_ror:
// R25/R24 	= *a
// R22 		= al
// R20 		= carry
	mov R31,R25
	mov R30,R24
	ROR R20
RSAROR1:
	ld R24,Z
	ROR R24
	st Z+,R24
	dec R22
	BRNE RSAROR1
// R25/R24	= return
	ret 		


; C calling  void modulo(unsigned char   *a,unsigned char   *b,unsigned char   *c,unsigned char al,unsigned char bl,unsigned char cl)
;	*a is in R25:R24
; 	*b is in R23:R22
;	*c is in R21:R20
;	al is the size in R18
;	bl is the size in R16
;	cl is the size in R14

rsa_modulo:	

	push	r7
	push	r8
	push	r9
	push	r10
	push	r11
	push	r12
	push	r13
	push	r14
	push	r15
	push	r16
	push	r17
	push	r28
	push	r29
	mov	r8, r24
	mov	r9, r25
	mov	r10, r22
	mov	r11, r23
	mov	r12, r20
	mov	r13, r21
	mov	r17, r18
	mov	r7, r14

;unsigned int tl,tc,ts;
;unsigned char carry_idvl;
;unsigned char stap;
;rsa_clrl(c,cl);
	mov	r22, r14
 	mov	r25, r21
	mov	r24, r20
	rcall	rsa_clrl   
;tc=al;
	mov	r28, r17
	eor	r29, r29
;tc<<=3;
	lsl r28
	rol r29
	lsl r28
	rol r29
	lsl r28
	rol r29
;ts=rsa_mov_ac(a,b,c,al,bl,cl); /* it just for speed up */
	mov	r18, r17
	mov	r21, r13
	mov	r20, r12
	mov	r23, r11
	mov	r22, r10
	mov	r25, r9
	mov	r24, r8
	rcall	rsa_mov_ac
;stap=0;
	eor	r14, r14
;for(tl=ts;tl<tc;tl++)
	cp	r24, r28
	cpc	r25, r29
	brcc	rsa_modulo_1 
	sub	r28, r24
	sbc	r29, r25
	mov	r16, r28
	mov	r17, r29
;   {
rsa_modulo_2:
;   if(0 == stap){
	and	r14, r14
	brne	rsa_modulo_5
;   carry_idvl=*a;
;				a++;
	mov	r31, r9
	mov	r30, r8
	ld	r15, Z+
	mov	r8, r30
	mov	r9, r31
;				stap=8;
	ldi	r24, 0x08	; 8
	mov	r14, r24
;				}
;   stap--;
rsa_modulo_5:
 	dec	r14
;   carry_idvl= rsa_rol(c,cl,carry_idvl);
	mov	r20, r15
	mov	r22, r7
	mov	r25, r13
	mov	r24, r12
	rcall	rsa_rol 
	mov	r15, r24
;   if(carry_idvl&0x01)
	sbrs	r24, 0
	rjmp	rsa_modulo_4
;	{
;	rsa_subl(c,b,cl);      /*  c(al) = c(al) - b(bl) */
	mov	r20, r7
	mov	r23, r11
	mov	r22, r10
	mov	r25, r13
	mov	r24, r12
	rcall	rsa_subl
;	carry_idvl|=0x01;
	ldi	r31, 0x01	; 1
	or	r15, r31
	rjmp	rsa_modulo_3
;	}
;  else
rsa_modulo_4:
;	 if(!rsa_cmpl(c,b,cl))
	mov	r20, r7
	mov	r23, r11
	mov	r22, r10
	mov	r25, r13
	mov	r24, r12
	rcall	rsa_cmpl
	and	r24, r24
	brne	rsa_modulo_3
;		{
;		rsa_subl(c,b,cl);  /*  c(al) = c(al) - b(bl) */
	mov	r20, r7
	mov	r23, r11
	mov	r22, r10
	mov	r25, r13
	mov	r24, r12
	rcall	rsa_subl
;		carry_idvl|=0x01;
	ldi	r24, 0x01	; 1
	or	r15, r24
rsa_modulo_3:
	subi	r16, 0x01	; 1
	sbci	r17, 0x00	; 0
	brne	rsa_modulo_2 
rsa_modulo_1:
	pop	r29
	pop	r28
	pop	r17
	pop	r16
	pop	r15
	pop	r14
	pop	r13
	pop	r12
	pop	r11
	pop	r10
	pop	r9
	pop	r8
	pop	r7
	ret

; C calling  rsa_clrl(unsigned char   *a,unsigned char al);
;	*a is in R25:R24 
;	al is the size in R22	;
; 	This function will clear
rsa_clrl:
	mov r31,r25
	mov r30,r24
Mem_Clr_1:
	st Z+,__zero_reg__
	dec r22
	brne Mem_Clr_1
	ret 	

; C calling  void mull(unsigned char *a,unsigned char *b,unsigned char *c,unsigned char alp,unsigned char blp,unsigned char clp)
;	*a is in R25:R24
; 	*b is in R23:R22
;	*c is in R21:R20
;	alp is the size in R18
;	blp is the size in R16
;	clp is the size in R14

; 	This function will Mull
rsa_mull:
	push r3
	push r4
	push r5
	push r6
	push r7
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	push r16
	push r17
	push r28
	push r29
	mov r4,r24
	mov r5,r25
	mov r8,r22
	mov r9,r23
	mov r12,r20
	mov r13,r21

	mov r3,r18
	mov r17,r16
	mov r6,r14
// unsigned char cl,bl,al;
// unsigned int mullab;
// unsigned char *pa,*pc;
// b=&b[blp];
	add r8,r17
	adc r9,__zero_reg__
	mov r27,r9// X
	mov r26,r8
// pa=&a[alp];
	add r4,r3
	adc r5,__zero_reg__
// pc=c;
	mov r10,r12
	mov r11,r13
// clrl(pc,clp);
	mov r22,r6
	mov r25,r11
	mov r24,r10
	rcall rsa_clrl
// bl=blp;
	mov r7,r17
// clp-=blp;
	sub r6,r7
	.MULL37:
// do
//   {  
//  cl=clp+bl;  /* */
	mov r16,r6
	add r16,r7
//   c=&pc[cl];  
	mov r13,r11
	mov r12,r10
	add r12,r16
	adc r13,__zero_reg__
//   b--;
   LD R24,-X // R24 is dummy 
//   a=pa;
	mov r29,r5
	mov r28,r4
//   cl--;  
	subi r16,lo8(-(-1))
//   c--;
	sec
	sbc r12,__zero_reg__
	sbc r13,__zero_reg__

//   al=alp;         /* end of a[] +1 */
	mov r17,r3
	.MULL36:
// 	do
//	 {	 
//	 a--;         
//      mullab=(*a)*(*b);
	ld r24,-y
	ld r22,x

#if __AVR_ARCH__ < 5
// R24:R14 = R24*R22
//
	clr R14
	lsr R24
	brcc .MULLD0//  D0
	ADD R14,R22
.MULLD0:
	ROR R14
	ROR R24
	brcc .MULLD1//  D1
	ADD R14,R22
.MULLD1:
	ROR R14
	ROR R24
	brcc .MULLD2//  D2
	ADD R14,R22
.MULLD2:
	ROR R14
	ROR R24
	brcc .MULLD3//  D3
	ADD R14,R22
.MULLD3:
	ROR R14
	ROR R24
	brcc .MULLD4//  D4
	ADD R14,R22
.MULLD4:
	ROR R14
	ROR R24
	brcc .MULLD5//  D5
	ADD R14,R22
.MULLD5:
	ROR R14
	ROR R24
	brcc .MULLD6//  D6
	ADD R14,R22
.MULLD6:
	ROR R14
	ROR R24
	brcc .MULLD7//  D7
	ADD R14,R22
.MULLD7:
	ROR R14
	ROR R24
//	 *c+= mullab ;
#else
// R14:R24 = R24*R22
	mul R24,R22
	mov r24,r0
	mov r14,r1
	eor r1,r1
#endif	

	mov r31,r13
	mov r30,r12

	ld r22,Z
	add r22,r24
	st Z,r22
//	 if(*c<(mullab & 0xff))add1(pc,cl);
   brcc .MULL33
//   push R25
	mov r22,r16
	mov r25,r11
	mov r24,r10
	rcall rsa_add1
.MULL33:
//		 cl--;
	dec r16
//		 c--;
	sec
	sbc r12,__zero_reg__
	sbc r13,__zero_reg__
//          mullab>>=8;
//          if(mullab!=0)
	tst R14
	breq .MULL34
//		   {
//           *c+=(unsigned char) mullab;
	mov r31,r13
	mov r30,r12
	ld r24,Z
	add r24,R14
	st Z,r24
//             if(*c<mullab)add1(pc,cl);
	brcc .MULL34
	mov r22,r16
	mov r25,r11
	mov r24,r10
	rcall rsa_add1
.MULL34:
// 		   }
// 	  al--;
	dec r17
// 	 } while(al>0);
	breq .MULL36NOT
	rjmp .MULL36
.MULL36NOT:
//	 bl--;
	dec r7
//    }while(bl>0);   
	breq .MULL37NOT
	rjmp .MULL37
.MULL37NOT:
  	pop r29
	pop r28
	pop r17
	pop r16
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop r7
	pop r6
	pop r5
	pop r4
	pop r3
	ret  	

; C calling  rsa_addl(unsigned char   *a,unsigned char   *b,unsigned char al);
;	*a is in R25:R24 
;	*a is in R23:R22 
;	al is the size in R20	
; 	This function will add two longs
rsa_addl:
// R25/R24 	= *a
// R23/R22	= *b
// R20 		= al
	mov R31,R25 	; Z
	mov R30,R24
	add R30,R20
	adc R31,__zero_reg__
	mov R27,R23		;X
	mov R26,R22
	add R26,R20
	adc R27,__zero_reg__
 	CLC
RSAADDL1:
	ld R24,-Z
	ld R25,-X
	adc R24,R25
	st Z,R24
	dec R20
	BRNE RSAADDL1
	clr R24
	rol R24
	clr R25
// R25/R24	= return
	ret  	

; C calling  rsa_subl(unsigned char   *a,unsigned char   *b,unsigned char al);
;	*a is in R25:R24 
;	*b is in R23:R22 
;	al is the size in R20	
; 	This function will add two longs
rsa_subl:
	mov R31,R25 	;Z
	mov R30,R24
	add R30,R20
	adc R31,__zero_reg__
	mov R27,R23		;X
	mov R26,R22
	add R26,R20
	adc R27,__zero_reg__
 	CLC
RSASUBL1:
	ld R24,-Z
	ld R25,-X
	SBC R24,R25
	st Z,R24
	dec R20
	BRNE RSASUBL1
	ret  	

; C calling  rsa_movel(unsigned char   *d,unsigned char   *s,unsigned char ld,unsigned char ls)
;	*a is in R25:R24 
;	*s is in R23:R22 
;	ld is the size in R20	
;	ld is the size in R18	
; 	This function one long to the ohther
rsa_movel:
 	push r14
 	push r15
 	push r16
 	push r17
 	push r28
 	push r29
 
 	mov r29,r25
 	mov r28,r24
 	mov r14,r22
 	mov r15,r23
 	mov r16,r20
 	mov r17,r18
 // if(ld>ls)clrl(d,ld-ls);
 
 	cp r17,r16
 	brsh .MOVEL28
 	mov r24,r16
 	sub r24,r17
 	mov r22,r24
 	mov r24,r28
 	mov r25,r29
 	rcall rsa_clrl
.MOVEL28:
 // d=&d[ld];
 	add r28,r16
 	adc r29,__zero_reg__
 // s=&s[ls];
 	add r14,r17;
 	adc r15,__zero_reg__
 	mov r31,r15
 	mov r30,r14
 // while(ls>0 && ld>0)
 	rjmp .MOVEL34
.MOVEL31:
 //   {
 //   ld--;
 	dec r16
 //   ls--;
 	dec r17
 //   d--;
 //   s--;
 //   *d=*s;  
 	ld r24,-Z
 	st -Y,r24
.MOVEL34:
 	tst r17
 	breq .MOVEL30
 	tst r16
 	brne .MOVEL31
.MOVEL30:
 	pop r29
 	pop r28
 	pop r17
 	pop r16
 	pop r15
 	pop r14
	ret	

; unsigned char rsa_memtst(unsigned char  *a,unsigned char al)
;	*a is in R25:R24 
;	al is the size in R22	
;	Return R25:R24

rsa_memtst:
	mov R30,R24
	mov R31,R25
	add r30,r22
	adc r31,__zero_reg__
	mov R24,__zero_reg__
	mov R25,__zero_reg__
memtst_L1:
	dec R22
	breq memtst_end
	ld r24,-Z
	tst R24
	breq memtst_L1
	ldi R24,1 
memtst_end:
	ret


#if  defined(__NOT_IN_USE__) 

#define RSALEN	64
#define TMP_LEN (2*RSALEN)

 	
;	void asm_rsa(unsigned char   *a ,unsigned char e,unsigned char   *d);
;	*a is in R25:R24 
;	e  is in R22	
; 	*d is in R21:R20
; This is the C ocde of the asm function 
;void rsa(unsigned char   *a ,unsigned char e,unsigned char   *d)
;{
;unsigned char flag;
;flag=2;
;/* s = 1 */
;/* a = a^e%d */
;while(e!=0)
;  {
;	if(0!=(e&1))
;	  {/*   s=(s*a)%d; */
;	   if(flag)
;		  { flag=0;
;                 movel(s,a,RSALEN,RSALEN);                  
;                /*  modulo(tmp,d,s,A_LEN,D_LEN,S_LEN);*/ /* S = A % D */
;		  }
;		else
;		  {
;		  mull(a,s,rsa_tmp,RSALEN,RSALEN,TMP_LEN); /* TMP = A*S   */                  
;		  modulo(rsa_tmp,d,s,TMP_LEN,RSALEN,RSALEN); /* S = TMP % D */
;		  }
;	  }
;	e>>=1;
;	/* a=(a*a)%d   */
;	if(e) /* A is not used is e == 0 */
;	 {
;	  mull(a,a,rsa_tmp,RSALEN,RSALEN,TMP_LEN); /* TMP = A * A    */
;	  modulo(rsa_tmp,d,a,TMP_LEN,RSALEN,RSALEN); /* A   = TMP % D  */
;	 }
; }
;movel(a,s,RSALEN,RSALEN);
;}

asm_rsa:
; flag=2;
		LDI     R24,0x02       ;  Load immediate
; while(e!=0)
asm_rsa_While_e:
		TST		R22
		breq 	asm_rsa_end
		ROR		R22
		brcs	asm_rsa_NOexp
;	   if(flag)
;		  { flag=0;
;                 movel(s,a,RSALEN,RSALEN);                  
;                /*  modulo(tmp,d,s,A_LEN,D_LEN,S_LEN);*/ /* S = A % D */
;		  }
;		else
;		  {
;		  mull(a,s,rsa_tmp,RSALEN,RSALEN,TMP_LEN); /* TMP = A*S   */                  
;		  modulo(rsa_tmp,d,s,TMP_LEN,RSALEN,RSALEN); /* S = TMP % D */
;		  }
asm_rsa_NOexp:
		TST		R22
		brne	asm_rsa_NOexp2	
		rjmp 	asm_rsa_While_e
asm_rsa_NOexp2:
;	  mull(a,a,rsa_tmp,RSALEN,RSALEN,TMP_LEN); /* TMP = A * A    */
;	 a is in R25:R24 
;	 a is in R23:R22 
; 	rsa_tmp is in R21:R20
;	RSALEN = R18
;	RSALEN = R16
;  TMP_LEN = R14
		PUSH 	R20
		PUSH	R21
		PUSH 	R22
		LDI 	R18,TMP_LEN
		MOV 	R18,R14
		LDI     R18,RSALEN
		LDI     R16,RSALEN
		LDI 	R20,lo8(rsa_tmp)	
		LDI 	R21,hi8(rsa_tmp)	
		MOV 	R24,R22
		MOV		R25,R23
		RCALL 	mull

;	  modulo(rsa_tmp,d,a,TMP_LEN,RSALEN,RSALEN); /* A   = TMP % D  */		
;	 rsa_tmp is in R25:R24 
;	       d is in R23:R22 
; 	       a is in R21:R20
;	TMP_LEN= R18
;	RSALEN = R16
;   RSALEN = R14

		POP		R22
		POP 	R21
		POP		R20

		rjmp 	asm_rsa_While_e
asm_rsa_end:
; movel(a,s,RSALEN,RSALEN);
;	 a is in R25:R24 
;	 s is in R23:R22	
		LDI     R18,RSALEN
		LDI     R20,RSALEN
		LDI 	R22,lo8(s)	
		LDI 	R23,hi8(s)	
		RCALL 	movel
		RET

asm_rsa_inc:
		ret

#endif


