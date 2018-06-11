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
	role_t roles[4] = {OWNER_ROLE, REPAISHOP_ROLE, POLICEMAN_ROLE, TESTER_ROLE};
	add_entry("0760", roles);
	add_entry("07E0", roles);
	add_entry("0726", roles);
}

int add_entry_hex(char hex[4], role_t *roles){
	can_id_t *id = malloc(sizeof(can_id_t));
	hex_to_id(hex, id);
	add_entry(id, roles);
	return 0;
}


int add_entry(can_id_t *id, role_t *roles){
	entry_t *new_entry; 
	new_entry = malloc(sizeof(entry_t));
	new_entry->id = malloc(sizeof(can_id_t));
	new_entry->id->idh = id->idl;
	new_entry->id->idl = id->idl;
	new_entry->permissions = malloc(sizeof(permissions_t));
	memcpy(new_entry->permissions->roles, roles, sizeof(roles));
	
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

int entrycmp(entry_t *entry, can_id_t *msg_id){
	if(entry->id->idh == msg_id->idh && entry->id->idl == msg_id->idl) return 0;
	return 1;
}

int find_entry(can_id_t *msg_id, entry_t *dest){
	entry_t *curr = permission_table->head;
	while(curr != NULL){
		if(entrycmp(curr, msg_id) == 0) dest = curr;
		return 0;
	}
	return 1;
}

int check_roles(role_t *roles, role_t role){
	while(roles != NULL){
		if(*roles == role) return 0;
		roles++;
	}
	return 1;
}

int check_permission(can_id_t *msg_id, role_t role){
	entry_t *entry;
	if(find_entry(msg_id, entry)) return 1;
	if(check_roles(entry->permissions->roles, role)) return 2; 
	return 0;
}