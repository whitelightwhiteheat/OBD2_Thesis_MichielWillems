/*
 * uart_f.h
 *
 * Created: 5/9/2018 1:59:52 AM
 *  Author: michel
 */ 


#ifndef UART_F_H_
#define UART_F_H_

#ifndef F_CPU
# define F_CPU 8000000L
#endif

#define USART_BAUDRATE 9600
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

void uart_init();

void uart_puts(char* s);


#endif /* UART_F_H_ */