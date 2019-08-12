
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "assembler.h"

#define LENGTH 100 /*of input line*/
#define SIZE 30 /*of symbol table*/
/*prototyple declaration*/


void print_data(data *to_print,	int DC)
{
	printf("value: %d | address: %d\n", to_print->value, DC);
}

void print_node(symbol *to_print)
{
	
	printf("action: %s | extern: %s | address: %d | label %s | data: %s | declared: %s\n", to_print->action? "yes" : "no", to_print->external? "yes" : "no", to_print->address, to_print->label, to_print->data_flag? "yes" : "no", to_print->declared? "yes" : "no");
	 
}

void print_sTable(int total, symbol **symbol_Table)
{
	int i = 0;
	
	while(i < total){
		printf("action: %s | extern: %s | address: %d | label %s | data: %s | declared: %s\n", symbol_Table[i]->action? "yes" : "no", symbol_Table[i]->external? "yes" : "no", symbol_Table[i]->address, symbol_Table[i]->label, symbol_Table[i]->data_flag? "yes" : "no", symbol_Table[i]->declared? "yes" : "no");
		i++;
	}
}

void print_code(int IC, encoding **table)
{
	int i;
	unsigned char byte;
	printf("|");
	printf("CPU[%d]: ",IC);
	if(table[IC]->act){
		for(i = 2; i >= 0; i--){
			byte = (table[IC]->memory.action.unused >> i) & 1;
			printf("%u", byte);
		}
		printf("-");
		for(i = 1; i >= 0; i--){
			byte = (table[IC]->memory.action.group >> i) & 1;
			printf("%u", byte);
		}
		printf("-");
		for(i = 3; i >= 0; i--){
			byte = (table[IC]->memory.action.opCode >> i) & 1;
			printf("%u", byte);
		}
		printf("-");
		for(i = 1; i >= 0; i--){
			byte = (table[IC]->memory.action.source >> i) & 1;
			printf("%u", byte);
		}
		printf("-");
		for(i = 1; i >= 0; i--){
			byte = (table[IC]->memory.action.dest >> i) & 1;
			printf("%u", byte);
		}
		printf("-");
		for(i = 1; i >= 0; i--){
			byte = (table[IC]->memory.action.are >> i) & 1;
			printf("%u", byte);
		}
		
	}
	else if(table[IC]->add){
		for(i = 12; i >= 0; i--){
				byte = (table[IC]->memory.new_ad.bin_address >> i) & 1;
				printf("%u", byte);
			}
		printf("-");
		for(i = 1; i >= 0; i--){
			byte = (table[IC]->memory.new_ad.are >> i) & 1;
			printf("%u", byte);
		}
	}
	else if(table[IC]->reg){
		for(i = 0; i >= 0; i--){
				byte = (table[IC]->memory.new_reg.unused >> i) & 1;
				printf("%u", byte);
			}
		printf("-");
		for(i = 5; i >= 0; i--){
			byte = (table[IC]->memory.new_reg.reg2 >> i) & 1;
			printf("%u", byte);
		}
		printf("-");
		for(i = 5; i >= 0; i--){
			byte = (table[IC]->memory.new_reg.reg1 >> i) & 1;
			printf("%u", byte);
		}
		printf("-");
		for(i = 1; i >= 0; i--){
			byte = (table[IC]->memory.new_reg.are >> i) & 1;
			printf("%u", byte);
		}
	}
	else if(table[IC]->dat){
		for(i = 14; i >= 0; i--){
			byte = (table[IC]->memory.new_data.value >> i) & 1;
			printf("%u", byte);
		}
	}
	else if(table[IC]->dir){
		for(i = 12; i >= 0; i--){
			byte = (table[IC]->memory.new_direct.value >> i) & 1;
			printf("%u", byte);
		}
		printf("-");
		for(i = 1; i >= 0; i--){
			byte = (table[IC]->memory.new_direct.are >> i) & 1;
			printf("%u", byte);
		}
		
	}
	else{
	}
	
	printf("|\n");
	
}

void print_all(int total, encoding **table)
{
	int i = 100;
	
	while(i < total){
		if(table[i] != NULL){
			print_code(i, table);
		}
		else{
		 printf("|CPU[%d]: \n", i);
		}
		i++;
	}
}
