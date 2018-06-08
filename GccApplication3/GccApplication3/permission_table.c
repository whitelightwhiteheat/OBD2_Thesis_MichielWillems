/*
 * permission_table.c
 *
 * Created: 6/8/2018 4:19:07 PM
 *  Author: michel
 */ 

#include "permission_table.h"
#include <stdlib.h>
#include <string.h>

permission_table_t *permission_table; 

void init_permissions_table(){
	permission_table = malloc(sizeof(permission_table_t));
}

int add_entry(can_msg_t msg, can_msg_t mask, role_t *roles){
	entry_t *curr = permission_table->head;
	entry_t *new_entry; 
	if(curr != NULL){
		while(curr->successor != NULL){
			curr = curr->successor;
		}
	}
	new_entry = curr;
	new_entry = malloc(sizeof(entry_t));
	memcpy(new_entry->message, msg,  sizeof(can_msg_t));
	memcpy(new_entry->mask, mask,  sizeof(can_msg_t));
	new_entry->permissions = malloc(sizeof(permissions_t));
	memcpy(new_entry->permissions->roles, roles, sizeof(roles));
	return 0;
}

int entrycmp(entry_t *entry, can_msg_t msg){
	int i;
	for(i=0; i<8; i++){
		uint8_t cmp = entry->message[i] & entry->mask[i];
		if(cmp == msg[i]) return 1;
	}
	return 0;
}

int find_entry(can_msg_t message, entry_t *dest){
	entry_t *curr = permission_table->head;
	while(curr != NULL){
		if(entrycmp(curr, message) == 0) dest = curr;
		return 0;
	}
	return 1;
}

int check_permission(can_msg_t message){
	entry_t *entry;
	if(find_entry(message, entry)) return 1;
	if(entrycmp(entry, message)) return 2; 
	return 0;
}