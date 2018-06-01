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

void can_get_frame_buffer( uint8_t *message , uint8_t buff_len );

int can_send_message( uint8_t mobnr , uint8_t id, uint8_t *message );

void can_get_message( uint8_t mobnr , uint8_t *message );

void can_print_message( uint8_t mobnr);

int can_send_frame_buffer( uint8_t *message, uint8_t buff_len);

int can_receive_message( uint8_t mobnr, uint8_t id, uint8_t mask, uint8_t *message);

int can_receive_frame_buffer( uint8_t *message, uint8_t buff_len);


#endif /* CAN_H_ */