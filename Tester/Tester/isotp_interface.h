/*
 * isotp_interface.h
 *
 * Created: 1/16/2019 8:54:32 PM
 *  Author: michel
 */ 


#ifndef ISOTP_INTERFACE_H_
#define ISOTP_INTERFACE_H_

int isotpi_send(can_id_t id,uint8_t payload_size , can_msg_t payload[payload_size]);

int isotpi_receive(can_id_t id,uint8_t payload_size , can_msg_t payload[payload_size]);

int isotpi_send_multi(can_id_t id, uint8_t *payload_size, uint8_t *payload);
	
int isotpi_receive_multi(can_id_t id_send, can_id_t id_rec, uint8_t payload_size, uint8_t payload[payload_size]);


#endif /* ISOTP_INTERFACE_H_ */