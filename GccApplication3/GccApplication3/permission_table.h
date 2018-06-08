/*
 * permission_table.h
 *
 * Created: 6/8/2018 4:46:25 PM
 *  Author: michel
 */ 


#ifndef PERMISSION_TABLE_H_
#define PERMISSION_TABLE_H_

#include <stdint.h>
#include "types.h"

typedef struct permissions_struct{
	role_t *roles;
} permissions_t;

typedef struct entry {
	can_msg_t message;
	can_msg_t mask;
	permissions_t *permissions;
	struct entry *successor;
} entry_t;

typedef struct permission_table_struct{
	entry_t *head;
} permission_table_t;

void init_permissions_table();

int check_permission(can_msg_t message);


#endif /* PERMISSION_TABLE_H_ */