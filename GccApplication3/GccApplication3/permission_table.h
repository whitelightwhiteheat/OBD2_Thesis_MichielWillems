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

typedef uint8_t permissions_t;

typedef struct entry {
	can_id_t id;
	permissions_t permissions;
	struct entry *successor;
} entry_t;

typedef struct permission_table_struct{
	entry_t *head;
} permission_table_t;

void init_permissions_table();

int add_entry(can_id_t id, permissions_t permissions);

int check_permission(can_id_t msg_id, permissions_t role);

int add_entry_hex(char *hex, permissions_t permissions);

#endif /* PERMISSION_TABLE_H_ */