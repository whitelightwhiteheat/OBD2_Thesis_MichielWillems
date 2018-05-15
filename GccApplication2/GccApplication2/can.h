/*
 * can.h
 *
 * Created: 5/15/2018 8:42:43 PM
 *  Author: michel
 */ 


#ifndef CAN_H_
#define CAN_H_

void can_init();

//For testing.
void CAN_INIT();

//For testing
void SendByMOb2();

void send_message( uint8_t mobnr , uint8_t *id, uint8_t *message );

void get_message( uint8_t mobnr , uint8_t *message );

void print_message( uint8_t mobnr);


#endif /* CAN_H_ */