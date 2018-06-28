/*
 * permission_table.c
 *
 * Created: 6/8/2018 4:19:07 PM
 *  Author: michel
 */ 

#include "permission_table.h"
#include "hexconv.h"
#include <stdlib.h>
#include <string.h>

permission_table_t *permission_table; 

void init_permissions_table(){
	permission_table = malloc(sizeof(permission_table_t));
	permission_table->head = NULL;
	permissions_t permissions = 0b00001111;
	add_entry_hex("b107", permissions);
	add_entry_hex("e007", permissions);
	add_entry_hex("2607", permissions);
	add_entry_hex("3007", permissions);
}

int add_entry_hex(char *hex, permissions_t permissions){
	can_id_t id;
	hex_to_bytes(hex, 4, id);
	add_entry(id, permissions);
	return 0;
}

int add_entry(can_id_t id, permissions_t permissions){
	entry_t *new_entry; 
	new_entry = malloc(sizeof(entry_t));
	new_entry->permissions = permissions;
	new_entry->successor = NULL;
	memcpy(new_entry->id, id, sizeof(can_id_t));
	entry_t *curr = permission_table->head;
	if(curr != NULL){
		while(curr->successor != NULL){
			curr = curr->successor;
		}
		curr->successor = new_entry;
	}else{
		permission_table->head = new_entry;
	}
	return 0;
}

int entrycmp(entry_t *entry, can_id_t msg_id){
	return memcmp(entry->id, msg_id, 2);
}

int find_entry_permissions(can_id_t msg_id, permissions_t *dest){
	entry_t *curr = permission_table->head;
	while(curr != NULL){
		if(entrycmp(curr, msg_id) == 0){
			*dest = curr->permissions;
			return 0;
		}else{
			curr = curr->successor;
		}
	}
	return 1;
}

int check_permission(can_id_t msg_id, permissions_t role){
	volatile permissions_t permissions;
	if(find_entry_permissions(msg_id, &permissions)) return 1;
	if((permissions & role) == role) return 0; 
	return 2;
}