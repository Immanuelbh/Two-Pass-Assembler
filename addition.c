/*
*	addition.c
*	contains all functions that adds binary code to the desired table(array)
*
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "assembler.h"


#define LENGTH 100 /*of input line*/
#define SIZE 30 /*of symbol table*/
/*prototyple declaration*/

int check_line(char *string);

/*adds a binary code from type 'command' */
void add_code_com(command *bin_code, int IC, encoding **table)
{
	
	table[IC] = malloc(sizeof(encoding));
	memset(table[IC], 0, sizeof(encoding));
	
	table[IC]->memory.action.are = bin_code->are;
	table[IC]->memory.action.dest = bin_code->dest;
	table[IC]->memory.action.source = bin_code->source;
	table[IC]->memory.action.opCode = bin_code->opCode;
	table[IC]->memory.action.group = bin_code->group;
	table[IC]->memory.action.unused = bin_code->unused;
	table[IC]->act = 1;
	
}

/*adds a binary code from type 'data' */
void add_code_data(data *to_add, int IC, encoding **table)
{
	table[IC] = malloc(sizeof(encoding));
	memset(table[IC], 0, sizeof(encoding));
	table[IC]->memory.new_data.value = to_add->value;
	table[IC]->dat = 1;
}

/*adds a binary code from type 'register' */
void add_regs(char* reg1, char* reg2, int IC, encoding **table)
{
	table[IC] = malloc(sizeof(encoding));
	memset(table[IC], 0, sizeof(encoding));
	table[IC]->memory.new_reg.are = 0;
	if(reg2 != NULL){
		table[IC]->memory.new_reg.reg1 = atoi(reg2+1);
	}
	else{
		table[IC]->memory.new_reg.reg1 = 0;
	}
	if(reg1 != NULL){
		table[IC]->memory.new_reg.reg2 = atoi(reg1+1);
	}
	else{
		table[IC]->memory.new_reg.reg2 = 0;
	}
	table[IC]->memory.new_reg.unused = 0;
	table[IC]->reg = 1;
	
}

/*adds numbers from a buffer to a data table */
int add_data(int buffer[], int tok_count, int DC, data **data_Table)
{
	int  i = 0;
	
	
	while(i < tok_count){ 
		data_Table[DC] = malloc(sizeof(data));
		memset(data_Table[DC], 0, sizeof(data));
		data_Table[DC]->value = buffer[i];
		DC++;
		i++;
	}
	
	
	return i;
	
}

/*reads a string and adds the letters to a data table*/
int readString(char *string, int DC, data **data_Table)
{
	int i = 1;	
	
	if(check_line(string)){
		if(strlen(string) > 2 && string[0] == '\"'){ /*must be atleast one letter*/
			while(i < strlen(string) && string[i] != '\"'){
				data_Table[DC] = malloc(sizeof(data));
				memset(data_Table[DC], 0, sizeof(data));
				data_Table[DC]->value = string[i];
				i++;
				DC++;
			}
			if(string[i] == '\"'){
				data_Table[DC] = malloc(sizeof(data));
				memset(data_Table[DC], 0, sizeof(data));
				data_Table[DC]->value = 0;
				return i;
			}
			else{
				fprintf(stderr, "Error: %s is missing a closing apostrophe\n", string);
				return 0;
			}
		}
		else{
			fprintf(stderr, "Error: %s is not a valid string\n", string);
		}
	}
	
	
	return 0;
}

/*checks to see if the input string is valid - starts and ends with " */
int check_line(char *string)
{
	int i, count = 0, last;
	
	for(i = 0; i < strlen(string); i++){
		if(string[i] == '\"'){
			count++;
			last = i;
		}
	}
	
	if(count == 2 && last == strlen(string) -1){
		return 1;
	}
	else if(count > 2){
		fprintf(stderr, "Error: too many \" in %s\n", string);
		
	}
	
	return 0;
}

/*adds a binary code from types 'label/address', 'index register', 'direct number', 'register' */
void add_code_op(char* operand, int IC, symbol **symbol_Table, encoding **table)
{
	int i;
	char *num;
	table[IC] = malloc(sizeof(encoding));
	memset(table[IC], 0, sizeof(encoding));
	
	
	i = check_label(operand, symbol_Table);
	if(i>=0){
		if(symbol_Table[i]->external){
			table[IC]->memory.new_ad.are = 1; /*01*/
			
		}
		else{
			table[IC]->memory.new_ad.are = 2; /*10*/
		}
		table[IC]->memory.new_ad.bin_address = symbol_Table[i]->address;
		table[IC]->add = 1;
	}
	else if(valid_ind_reg(operand)){
		table[IC]->reg = 1;
		table[IC]->memory.new_reg.are = 0;
		table[IC]->memory.new_reg.reg1 = operand[1] -'0';
		table[IC]->memory.new_reg.reg2 = operand[4] -'0';
		table[IC]->memory.new_reg.unused = 0;
		
		
	}
	else if(valid_direct(operand)){
		num = operand +1;
		table[IC]->memory.new_direct.are = 0;
		table[IC]->memory.new_direct.value = atoi(num);
		table[IC]->dir = 1;
	}
	else if(valid_reg(operand)){
		table[IC]->memory.new_reg.are = 0;
		table[IC]->memory.new_reg.unused = 0;
		if(table[IC-1]->act){
			table[IC]->memory.new_reg.reg2 = operand[1] -'0';
			table[IC]->memory.new_reg.reg1 = 0;
		}
		else if(table[IC-1]->add || table[IC-1]->reg || table[IC-1]->dat|| table[IC-1]->dir){
			add_regs(NULL, operand, IC, table);
		}
		else{
			table[IC]->memory.new_reg.reg1 = operand[4] -'0';
		}
		
		table[IC]->reg = 1;
	}
	else{
		fprintf(stderr, "Error: could not create code: operand (%s) , IC (%d)\n", operand, IC);
	}
	
}

/*adds a given symbol to the symbol table*/
int add_symbol(symbol* new_symbol, int count, symbol **symbol_Table)
{
	static size_t current_size = SIZE;

	if(count == current_size) {
	  current_size = current_size << 1;
	  symbol_Table = (symbol **)realloc(symbol_Table, sizeof(symbol *) * current_size);
	}
	
	if(count < SIZE){
		symbol_Table[count] = malloc(sizeof(symbol));
		memset(symbol_Table[count], 0, sizeof(symbol));
		symbol_Table[count]->action = new_symbol->action;
		symbol_Table[count]->external = new_symbol->external;
		symbol_Table[count]->address = new_symbol->address;
		strcpy(symbol_Table[count]->label, new_symbol->label);
		symbol_Table[count]->data_flag = new_symbol->data_flag;
		symbol_Table[count]->declared = new_symbol->declared;
		return ++count;
	}
	
	return count;
	
}
