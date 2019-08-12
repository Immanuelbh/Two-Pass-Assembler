/*
*	validation.c
*	contains all functions that relate to validation.
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


/*checks validity of the inputed label, must contain only letters and numbers, cannot have a similar name as an instruction or a regestry*/
int valid_label(char *label)
{
	if(isalpha(label[0])){
		if(valid_string(label)){
			if(valid_action(label) >= 0 || valid_reg(label)){
				return 0;
			}
			else{
				return 1; 
			}
		}
	}
	
	return 0;
}

/*checks validity of a string, must conatin only letters and numbers*/
int valid_string(char *string)
{
	char temp;
	int len = strlen(string);
	int i ;
	
	for(i = 0; i < len ; i++){
		temp = string[i];
		if((temp >= 'A' && temp <= 'Z') || (temp >= 'a' && temp <= 'z') || (temp >= '0' && temp <= '9')){
			continue; 
			
		}
		else {
			return 0;
		}
	} 
	
	return 1; 
}

/*checks if input is a known register*/
int valid_reg(char *reg){
	
	if(strcmp(reg, "r1") == 0 || strcmp(reg, "r2") == 0 || strcmp(reg, "r3") == 0 ||
		strcmp(reg, "r4") == 0 || strcmp(reg, "r5") == 0 || strcmp(reg, "r6") == 0 || strcmp(reg, "r7") == 0 ){
		return 1;
	}
	
	return 0;
}

/*checks if input is a known action*/
int valid_action(char *action)
{
	if(strcmp(action, "mov") == 0 || strcmp(action, "cmp") == 0 || strcmp(action, "add") == 0 || strcmp(action, "sub") == 0 ||  strcmp(action, "lea") == 0){
			return 2;
	}
	else if(strcmp(action, "not") == 0 || strcmp(action, "clr") == 0 || strcmp(action, "inc") == 0 || strcmp(action, "dec") == 0 || strcmp(action, "jmp") == 0 ||
		strcmp(action, "bne") == 0 || strcmp(action, "red") == 0 || strcmp(action, "prn") == 0 || strcmp(action, "jsr") == 0){
		return 1;
	}
	else if(strcmp(action, "rts") == 0 || strcmp(action, "stop") == 0){
		return 0;
	}
	
	return -1;
}

/*checks if input is a valid number, can contain +/-.*/
int valid_num(char *num)
{
	int i = 0, check = 0;
	int len = strlen(num);
	char c;
	
	if(num[0] == '+' || num[0] == '-'){
		while((c = num[++i])){
			if(c >= '0' && c <= '9'){
				check++;
			}
		}

		if(check == len-1){
			return 1;
		}
	}
	else if(isdigit(num[0])){
		while((c = num[i++])){
			if(c >= '0' && c <= '9'){
				check++;
			}
			
		}

		if(check == len){
			return 1;
		}
	}
	return 0;
}

#define MAX_DIRECT 4095
#define MIN_DIRECT -4096

/*checks if input is a valid direct number*/
int valid_direct(char* direct)
{
	int i = 2;
	int len = strlen(direct);
	if(len > 1){
		if(direct[0] == '#' && (direct[1] == '+' || direct[1] == '-' || isdigit(direct[1]))){
			while(i < len && isdigit(direct[i])){ 
				i++;
			}
			if(i == len){
				if(atoi(direct+1) <= MAX_DIRECT && atoi(direct+1) >= MIN_DIRECT){
					return 1;
				}
			}
		}
	}
	
	return 0; /*not a direct addressing*/
	
	
}

/*checks if input is a valid index register*/
int valid_ind_reg(char* input)
{
	
	if(strlen(input) == 6){
		if(input[0] != 'r'){
			return 0;
		}
		else if(input[1] != '1' && input[1] != '3' && input[1] != '5' && input[1] != '7'){
			return 0;
		}
		else if(input[2] != '['){
			return 0;
		}
		else if(input[3] != 'r'){
			return 0;
		}
		else if(input[4] != '0' && input[4] != '2' && input[4] != '4' && input[4] != '6'){
			return 0;
		}
		else if(input[5] != ']'){
			return 0;
		}
		
		return 1;
		
	}
	return 0;
}

/*checks if operands are valid, used to decide if labels exist and can be used*/
int valid_ops(int num_act, char *operand, symbol **symbol_Table)
{
	
	char *sptr;
	sptr = strtok(operand, ", \t\n");
	switch(num_act){
		case 0:
				
			break;
		case 1: 
			if(valid_direct(sptr) || check_label(sptr, symbol_Table) >= 0 || valid_ind_reg(sptr) || valid_reg(sptr)){
				return 1;
			}
			else{
				fprintf(stderr, "Error: \"%s\" is not a valid operand\n", sptr);
			}
			break;
		case 2:
			if(valid_direct(sptr) || check_label(sptr, symbol_Table) >= 0 || valid_ind_reg(sptr) || valid_reg(sptr)){
				sptr = strtok(NULL, " \t\n");
				if(valid_direct(sptr) || check_label(sptr, symbol_Table) >= 0 || valid_ind_reg(sptr) || valid_reg(sptr)){
					return 1;
				}
				else{
					fprintf(stderr, "Error: \"%s\" is not a valid operand\n", sptr);
				}
			}
			else{
				fprintf(stderr, "Error: \"%s\" is not a valid operand\n", sptr);
			}
			break;
	}
	
	return 0;
}


/*checks if input is a known action with no operands*/
int no_op(char* action)
{
	
	if(action != NULL){
		if(strcmp(action, "rts") == 0){
			return 14; /*1110*/
		}
		else if(strcmp(action, "stop") == 0){
			return 15; /*1111*/
		}
	}
		
	return 0;
	
}

/*checks if input is a known action with one operand*/
int one_op(char* action)
{
	
	if(action != NULL){
		if(strcmp(action, "not") == 0){ 
			return 4; /*0100*/
		}
		else if(strcmp(action, "clr") == 0){
			return 5; /*0101*/
		}
		else if(strcmp(action, "inc") == 0){
			return 7; /*0111*/
		}
		else if(strcmp(action, "dec") == 0){
			return 8; /*1000*/
		}
		else if(strcmp(action, "jmp") == 0){
			return 9; /*1001*/
		}
		else if(strcmp(action, "bne") == 0){
			return 10; /*1010*/
		}
		else if(strcmp(action, "red") == 0){
			return 11; /*1011*/
		}
		else if(strcmp(action, "prn") == 0){
			return 12; /*1100*/
		}
		else if(strcmp(action, "jsr") == 0){
			return 13; /*1101*/
		}
	}
		
	
	return (-1);
}

/*checks if input is a known action with two operands*/
int two_op(char* action)
{
	if(action != NULL){
		if(strcmp(action, "mov") == 0){
			return 0; /*0000*/
		}
		else if(strcmp(action, "cmp") == 0){ 
			return 1; /*0001*/
		}
		else if(strcmp(action, "add") == 0){
			return 2; /*0010*/
		}
		else if(strcmp(action, "sub") == 0){ 
			return 3; /*0011*/
		}
		else if(strcmp(action, "lea") == 0){ 
			return 6; /*0110*/
		}
	}
	
	
	return (-1);
}

/*checks if input is a valid destination*/
int setDest(char *dest, symbol **symbol_Table)
{
	if(valid_direct(dest)){
		return 0; /*00*/
	}
	else if(valid_label(dest) || check_label(dest, symbol_Table)>=0){ 
		return 1;/*01*/
	}
	else if(valid_ind_reg(dest)){
		return 2; /*10*/
	}
	else if(valid_reg(dest)){
		return 3; /*11*/
	}
	return (-1);
}

/*checks if input is a valid source*/
int setSource(char *source, symbol **symbol_Table)
{
	if(valid_direct(source)){
		return 0; /*00*/
	}
	else if(valid_label(source) || check_label(source, symbol_Table)>=0){ 
		return 1;/*01*/
	}
	else if(valid_ind_reg(source)){
		return 2; /*10*/
	}
	else if(valid_reg(source)){
		return 3; /*11*/
	}
	return (-1);
	
	
}


/*checks if label exists in the symbol table*/
int check_label(char* label, symbol **symbol_Table)
{
	int i = 0;
	
	
	while(symbol_Table[i] != NULL){
		if(strcmp(label, symbol_Table[i]->label) == 0){
			return i;
		}
		i++;
	}
	
	
	return (-1);
}

/*checks if the label is declared*/
int label_dec(char* label, symbol **symbol_Table)
{
	int i = 0;
	
	
	while(symbol_Table[i] != NULL){
		if(strcmp(label, symbol_Table[i]->label) == 0){
			if(symbol_Table[i]->declared == 1){
				return 1;
			}
		}
		i++;
	}
	
	
	return (0);
}

/*checks if the label is external*/
int label_ext(char* label, symbol **symbol_Table)
{
	int i = 0;
	
	
	while(symbol_Table[i] != NULL){
		if(strcmp(label, symbol_Table[i]->label) == 0){
			if(symbol_Table[i]->external == 1){
				return 1;
			}
		}
		i++;
	}
	
	
	return (0);
}

/*checks if the label is an action*/
int label_act(char* label, symbol **symbol_Table)
{
	int i = 0;
	
	
	while(symbol_Table[i] != NULL){
		if(strcmp(label, symbol_Table[i]->label) == 0){
			if(symbol_Table[i]->action == 1){
				return 1;
			}
		}
		i++;
	}
	
	
	return (0);
}

/*sets the line value according to the given line*/
void line_is_valid(valid_lines* new_line, int count, valid_lines **file_line)
{
	static size_t current_size = LINE_SIZE;

	if(count == current_size) { /*in case the table reaches it's maximum size*/
	  current_size = current_size << 1;
	  file_line = (valid_lines **)realloc(file_line, sizeof(valid_lines *) * current_size);
	}
	
	if(count < LINE_SIZE){
		file_line[count] = malloc(sizeof(valid_lines));
		memset(file_line[count], 0, sizeof(valid_lines));
		file_line[count]->not_valid = new_line->not_valid;
		
	}
	
}

