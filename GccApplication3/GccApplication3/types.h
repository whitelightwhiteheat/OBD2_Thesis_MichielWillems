/*
 * types.h
 *
 * Created: 6/8/2018 5:26:44 PM
 *  Author: michel
 */ 


#ifndef TYPES_H_
#define TYPES_H_


typedef enum {
	IDLE_S,
	WAIT_FOR_SIGNATURE_S,
	AUTHENTICATED_S
} state_t;

typedef enum {
	NULL_E,
	INIT_RECEIVED_E,
	SIGNATURE_RECEIVED_E,
	MESSAGE_RECEIVED_E
} event_t;

typedef enum {
	OWNER_ROLE,
	REPAISHOP_ROLE,
	POLICEMAN_ROLE,
	TESTER_ROLE
} role_t;

typedef uint8_t can_msg_t[8];
typedef uint8_t can_buff_512_t[64];

#endif /* TYPES_H_ */