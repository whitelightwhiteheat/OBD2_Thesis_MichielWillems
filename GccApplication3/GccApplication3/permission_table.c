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
	permissions_t permissions = 0b00001111;
	add_entry_hex("0726", permissions);
	add_entry_hex("07E0", permissions);
	//add_entry_hex("0726", roles);
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

int find_entry(can_id_t msg_id, entry_t **dest){
	entry_t *curr = permission_table->head;
	uart_puts(curr->id);
	while(curr != NULL){
		if(entrycmp(curr, msg_id) == 0){
			*dest = curr;
			return 0;
		}else{
			curr = curr->successor;
		}
	}
	return 1;
}

int check_permission(can_id_t msg_id, permissions_t role){
	entry_t **adress;
	if(find_entry(msg_id, adress)) return 1;
	entry_t *entry = *adress;
	volatile uint8_t test = entry->permissions;
	if((entry->permissions && role) == role) return 0; 
	return 2;
}