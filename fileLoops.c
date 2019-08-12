/*
*	fileLoops.c
*	contains the two loops that go through the inputed file(s).
*
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "assembler.h"

#define LENGTH 100 /*of input line*/
#define SIZE 30 /*of symbol table*/
#define LINE_SIZE 300
#define CPU_SIZE 1000
#define OCTET 255
#define START 100
#define MAX_VAL 16383 /*max number of 15 bits*/
#define MIN_VAL -16384 /*min number of 15 bits*/
#define LABEL_SIZE 30
#define LABEL 1
#define TO_CHECK 11

int count_comma(char *data_line);

/*first pass on the file, checks validity of the lines, creates symbol table, data table and a temporary action table.*/
counters readFile(FILE *fp, symbol **symbol_Table, data **data_Table, valid_lines **file_line, encoding **action_Table)
{
	int IC = 100, DC = 0, i, tok_count, sTable_count = 0, line_count = 0, temp_num;
	int buffer[SIZE], temp_DC, temp_IC, a_flag;
	char line[LENGTH]; /* assuming max input is 80*/
	char *sptr, *temp, *data_line; /*string pointer*/ 
	int commas;
	int l_flag = 0, errno = 0;
	symbol* new_symbol = NULL;
	valid_lines* new_line = NULL;
	counters* new_count = NULL;
	
	while(fgets(line, sizeof(line), fp) != NULL){
		line_count++;
		errno = 0;
		if(isalpha(line[0]) || isdigit(line[0])){ /*if it's a label*/
			if((sptr = strtok(line, ":")) != NULL){
				if(valid_label(sptr) && strlen(sptr) <= LABEL_SIZE){ 
					if(label_dec(sptr, symbol_Table) == 0){
						temp = sptr;
						l_flag = 1;
						sptr = strtok(NULL, " \t\n"); 
						a_flag = valid_action(sptr);/*amount of operands*/
						if(a_flag >= 0){
							/*check if the whole line with the valid action is correct*/
						
							/*creating binary machine code*/
							temp_IC = IC;
							IC = create_bin_code(temp, sptr, IC, a_flag, l_flag, symbol_Table, action_Table);
							if(temp_IC < IC){/*creation of the action code was successful, create symbol for the label*/
								new_symbol =  malloc(sizeof(symbol));
								memset(new_symbol, 0, sizeof(symbol));
								
								strcpy(new_symbol->label, temp);
								new_symbol->action = 1; 
								new_symbol->external = 0;
								new_symbol->declared = 1;
								new_symbol->address = temp_IC; /*IC is the address that the bin code of the command will be put in to*/
								new_symbol->data_flag = 0;
								sTable_count = add_symbol(new_symbol, sTable_count, symbol_Table);
								free(new_symbol);
								
								/*if one of the operands is a label, flag it for a future check*/
								if(action_Table[temp_IC]->memory.action.dest == LABEL || action_Table[temp_IC]->memory.action.source == LABEL){
									errno = TO_CHECK;
								}
								
							}
							else{
								errno = 1;
							}
						} /*end if*/
						else if(strcmp(sptr, ".data") == 0){ /*no line for action*/
							memset(&buffer, 0, sizeof buffer);
							i = 0;
							tok_count = 0;
							errno = 0;
							if((data_line = strtok(NULL, "\n")) != NULL){
								commas = count_comma(data_line);
								for(sptr = strtok(data_line, ", \t\n"); sptr != NULL ; sptr = strtok(NULL, ", \t\n")){
							  		if(i < SIZE && valid_num(sptr)){
							  			temp_num = atoi(sptr);
							  			if(temp_num >= MIN_VAL && temp_num <= MAX_VAL){
							  				buffer[i] = temp_num; /*fill buffer*/
											i++;
										}
										else{
											errno = 1;
											break;
										}
									}
									else{
										errno = 2;
										break;
									}
									tok_count++;
									if(tok_count > commas +1){
										errno = 3;
										break;
									}
								}
								/*if all the numbers that were entered are valid*/
								if(errno == 0 && i == tok_count && commas +1 == tok_count){
									/*add to data table*/
									DC += add_data(buffer, tok_count, DC, data_Table);
									/*create symbol*/
									new_symbol =  malloc(sizeof(symbol));
									memset(new_symbol, 0, sizeof(symbol));
									
									strcpy(new_symbol->label, temp);
									new_symbol->external = 0;
									new_symbol->address = DC - tok_count; 
									new_symbol->action = 0;
									new_symbol->data_flag = 1;
									new_symbol->declared = 1;
									sTable_count = add_symbol(new_symbol, sTable_count, symbol_Table);
									free(new_symbol);
									
								}
								else{ /*not all numbers were valid*/
									if(sptr != NULL){
										switch(errno){
											case 1:
												fprintf(stderr, "(%d) Error: %d is out of bounds. Valid numbers are between %d and %d\n", line_count, temp_num, MIN_VAL, MAX_VAL);
												break;
											case 2:
												fprintf(stderr, "(%d) Error: %s is not a valid number\n",line_count, sptr);
												break;
											case 3:
												fprintf(stderr, "(%d) Error: too many numbers\n", line_count);
												break;
											default:
												fprintf(stderr, "(%d) Error: %s is not valid, .data can only contain integers with commas between them\n", line_count, sptr);
												break;
												
										}
									}
									else{
										fprintf(stderr, "(%d) Error: not a valid line.\n", line_count);
										errno = 1;
									}
								}
							}
						}
						else if(strcmp(sptr, ".string") == 0){ 
							temp_DC = DC;
							if((sptr = strtok(NULL, "\n")) != NULL){
								DC += readString(sptr, DC, data_Table);
							}
							else{
								fprintf(stderr, "(%d) Error: missing string\n", line_count);
							}
							/*creating symbol*/
							if(temp_DC < DC){ /*if the string read was successful*/
								new_symbol =  malloc(sizeof(symbol));
								memset(new_symbol, 0, sizeof(symbol));
								strcpy(new_symbol->label, temp);
								new_symbol->external = 0;
								new_symbol->address = temp_DC;
								new_symbol->action = 0;
								new_symbol->data_flag = 1;
								new_symbol->declared = 1;
								sTable_count = add_symbol(new_symbol, sTable_count, symbol_Table);
								free(new_symbol);
								
							}
							else{
								errno = 1;
							}
						}
						else if(strcmp(sptr, ".entry") == 0){
							if((sptr = strtok(NULL, "\n")) != NULL){
								fprintf(stderr, "(%d) Warning: the label will be ignored\n", line_count);
								if(valid_label(sptr)){
									if(label_ext(sptr, symbol_Table)){
										fprintf(stderr, "(%d) Error: \"%s\" is already declared as external, cannot be entry\n", line_count, sptr);
										errno = 1;
									} 
								}
								else{
									printf("(%d) Error: %s is not a valid label\n",line_count, sptr);
									errno = 1;
								}
								
							}
							else{
								fprintf(stderr, "(%d) Error: missing entry\n", line_count);
								
								errno = 1;
							
							}
						}
						else if(strcmp(sptr, ".extern") == 0){
							if((sptr = strtok(NULL, "\t\n")) != NULL){
								fprintf(stderr, "(%d) Warning: the label will be ignored\n", line_count);
								if(valid_label(sptr)){ 
									if(check_label(sptr, symbol_Table) >= 0){ /*label exists*/
										if(label_ext(sptr, symbol_Table)){
											fprintf(stderr, "(%d) Warning: \"%s\" is already registered as external\n", line_count, sptr);
											/*not an error, just avoid creating an extra symbol*/
										}
										else if(label_act(sptr, symbol_Table)){
											fprintf(stderr, "(%d) Error: \"%s\" is a local label and cannot be external\n", line_count, sptr);
											errno = 1;
										}
										else{
											/*an existing label that is not an action or an external*/
										}
									}
									else{ /*label is valid and does not exist -> create*/
										new_symbol =  malloc(sizeof(symbol));
										memset(new_symbol, 0, sizeof(symbol));
										strcpy(new_symbol->label, sptr);
										new_symbol->external = 1;
										new_symbol->address = 0; 
										new_symbol->action = 0;
										new_symbol->data_flag = 0;
										new_symbol->declared = 0;
										sTable_count = add_symbol(new_symbol, sTable_count, symbol_Table);
										free(new_symbol);
							
									}
								}
								else{
									fprintf(stderr, "(%d) Error: \"%s\" is not a valid label\n", line_count, sptr);
								}
							}
							else{
								fprintf(stderr, "(%d) Error: missing entry\n", line_count);
								errno = 1;
							}
							
						}
						else{
							fprintf(stderr, "(%d) Warning: %s is not valid after a label declaration\n", line_count, sptr);
							errno = 1;
						}
					}
					else{
						fprintf(stderr, "(%d) Error: the label \"%s\" already exists\n", line_count, sptr);
						errno = 1;
						
					}
					
				}
				else{
					fprintf(stderr, "(%d) %s is not a valid label\n", line_count, sptr);
					errno = 1;
				} 
			}
		}
		else if(line[0] == ';'){ /*if it's a comment*/
		
		}
		else if(line[0] == '.') { 
			sptr = strtok(line, " ");
			if(strcmp(sptr, ".extern") == 0){
				sptr = strtok(NULL, "\t\n");
				if(valid_label(sptr)){
					if(check_label(sptr, symbol_Table) >= 0){ /*label exists*/
						if(label_ext(sptr, symbol_Table)){
							fprintf(stderr, "(%d) Warning: \"%s\" is already registered as external\n", line_count, sptr);
							/*not an error, just avoid creating an extra symbol*/
						}
						else if(label_act(sptr, symbol_Table)){
							fprintf(stderr, "(%d) Error: \"%s\" is a local label and cannot be external\n", line_count, sptr);
							errno = 1;
						}
						else{
							/*an existing label that is not an action or an external*/
						}
					}
					else{ /*label is valid and does not exist -> create*/
						new_symbol =  malloc(sizeof(symbol));
						memset(new_symbol, 0, sizeof(symbol));
						strcpy(new_symbol->label, sptr);
						new_symbol->external = 1;
						new_symbol->address = 0; 
						new_symbol->action = 0;
						new_symbol->data_flag = 0;
						new_symbol->declared = 0;
						sTable_count = add_symbol(new_symbol, sTable_count, symbol_Table);
						free(new_symbol);
					}
				}		
			}
			else if(strcmp(sptr, ".entry") == 0){
				if((sptr = strtok(NULL, " \n")) != NULL){
					if(valid_label(sptr)){
						if(label_ext(sptr, symbol_Table)){
							fprintf(stderr, "(%d) Error: \"%s\" is already declared as external, cannot be entry\n", line_count, sptr);
							errno = 1;
						} 
					}
					else{
						printf("(%d) Error: %s is not a valid label\n",line_count, sptr);
						errno = 1;
					}
				}
				else{
					fprintf(stderr, "(%d) Error: missing entry\n", line_count);
					errno = 1;
				}
			}
			else{
				fprintf(stderr, "(%d) Error: %s is not a valid instruction\n", line_count, sptr);
				errno = 1;
			}
		}
		else if(line[0] == ' ' || line[0] == '\t') { /* line is action only*/
			if((sptr = strtok(line, " \t\n")) != NULL){
				a_flag = valid_action(sptr);
				l_flag = 0;
				if(a_flag >= 0){
					/*creating binary machine code*/
					temp_IC = IC;
					IC = create_bin_code(NULL, sptr, IC, a_flag, l_flag, symbol_Table, action_Table);
					if(temp_IC < IC){
						if(action_Table[temp_IC]->memory.action.dest == LABEL || action_Table[temp_IC]->memory.action.source == LABEL){
							/*one of the operands is a label, flag it for a future check*/
							errno = TO_CHECK;
						}
					}
					else{
						errno = 1;
					}
				} /*end if*/
				else if(strcmp(sptr, ".data") == 0){ 
					memset(&buffer, 0, sizeof buffer);
					i = 0;
					tok_count = 0;
					errno = 0;
					if((data_line = strtok(NULL, "\n")) != NULL){
						commas = count_comma(data_line); /*to help with validating the string*/
						for(sptr = strtok(data_line, ", \t\n"); sptr != NULL ; sptr = strtok(NULL, ", \t\n")){
					  		if(i < SIZE && valid_num(sptr)){
					  			temp_num = atoi(sptr);
					  			if(temp_num >= MIN_VAL && temp_num <= MAX_VAL){
					  				buffer[i] = temp_num; /*fill buffer*/
									i++;
								}
								else{
									errno = 1;
									break;
								}
							}
							else{
								errno = 2;
								break;
							}
							tok_count++;
							if(tok_count > commas +1){
								errno = 3;
								break;
							}
						}
						/*if no errors were found*/
						if(errno == 0 && i == tok_count && commas +1 == tok_count){
							temp_DC = DC;
							DC += add_data(buffer, tok_count, DC, data_Table);
							if(temp_DC == DC){
								fprintf(stderr, "(%d) Error: not a valid string\n", line_count);
								errno = 1;
							}
						}
						else{
							if(sptr != NULL){
								switch(errno){
									case 1:
										fprintf(stderr, "(%d) Error: %d is out of bounds. Valid numbers are between %d and %d\n", line_count, temp_num, MIN_VAL, MAX_VAL);
										break;
									case 2:
										fprintf(stderr, "(%d) Error: %s is not a valid number\n",line_count, sptr);
										break;
									case 3:
										fprintf(stderr, "(%d) Error: too many numbers\n", line_count);
										break;
									default:
										fprintf(stderr, "(%d) Error: %s is not valid, .data can only contain integers with commas between them\n", line_count, sptr);
										break;
								}
							}
							else{
								fprintf(stderr, "(%d) Error: not a valid line.\n", line_count);
								errno = 1;
							}
						}
					}
					else{
						fprintf(stderr, "(%d) Error: missing data\n", line_count);
						errno = 1;
					}
				}
				else if(strcmp(sptr, ".string") == 0){ 
					temp_DC = DC;
					if((sptr = strtok(NULL, "\n")) != NULL){
						temp_DC = DC;
						DC += readString(sptr, DC, data_Table);
						if(temp_DC == DC){
							fprintf(stderr, "(%d) Error: %s is not a valid string\n", line_count, sptr);
							errno = 1;
						}
					}
					else{
						fprintf(stderr, "(%d) Error: missing string\n", line_count);
						errno = 1;
					}
				}
				else if(strcmp(sptr, ".extern") == 0){ 
					if((sptr = strtok(NULL, " \n")) != NULL){
						if(valid_label(sptr)){ 
							if(check_label(sptr, symbol_Table) >= 0){ /*label exists*/
								if(label_ext(sptr, symbol_Table)){
									fprintf(stderr, "(%d) Warning: \"%s\" is already registered as external\n", line_count, sptr);
									/*not an error, just avoid creating an extra symbol*/
								}
								else if(label_act(sptr, symbol_Table)){
									fprintf(stderr, "(%d) Error: \"%s\" is a local label and cannot be external\n", line_count, sptr);
									errno = 1;
								}
								else{
									/*an existing label that is not an action or an external*/
								}
							}
							else{ /*label is valid and does not exist -> create*/
								new_symbol =  malloc(sizeof(symbol));
								memset(new_symbol, 0, sizeof(symbol));
								strcpy(new_symbol->label, sptr);
								new_symbol->external = 1;
								new_symbol->address = 0; 
								new_symbol->action = 0;
								new_symbol->data_flag = 0;
								new_symbol->declared = 0;
								sTable_count = add_symbol(new_symbol, sTable_count, symbol_Table);
								free(new_symbol);
							}
						}
						else{
							fprintf(stderr, "(%d) Error: %s is not a valid label\n",line_count, sptr);
							errno = 1;
						}
					}
					else{
						fprintf(stderr, "(%d) Error: missing entry\n", line_count);
						errno = 1;
					}
				}
				else if(strcmp(sptr, ".entry") == 0){
					if((sptr = strtok(NULL, " \n")) != NULL){
						if(valid_label(sptr)){
							if(label_ext(sptr, symbol_Table)){
								fprintf(stderr, "(%d) Error: \"%s\" is already declared as external, cannot be entry\n", line_count, sptr);
								errno = 1;
							} 
						}
						else{
							printf("(%d) Error: %s is not a valid label\n",line_count, sptr);
							errno = 1;
						}
					}
					else{
						fprintf(stderr, "(%d) Error: missing entry\n", line_count);
						errno = 1;
					}
				}
				else{
					fprintf(stderr, "(%d) Error: %s is not a valid command\n",line_count, sptr);
					errno = 1;
				}
			} /*end if*/ 
		} /*end else if*/ 
		
		if(errno){/*line is not valid, flag it*/
			if(errno == TO_CHECK){/*one or more operands are labels*/
				new_line = malloc(sizeof(valid_lines));
				memset(new_line, 0, sizeof(valid_lines));
				new_line->not_valid = 2; /*line to be checked*/
				line_is_valid(new_line, line_count, file_line);
				free(new_line);
			}
			else{/*invalid line*/
				new_line = malloc(sizeof(valid_lines));
				memset(new_line, 0, sizeof(valid_lines));
				new_line->not_valid = 1; /*line is not valid*/
				line_is_valid(new_line, line_count, file_line);
				free(new_line);
			}	
		}
		else{/*valid line*/
			new_line = malloc(sizeof(valid_lines));
			memset(new_line, 0, sizeof(valid_lines));
			new_line->not_valid = 0; /*valid line*/
			line_is_valid(new_line, line_count, file_line);
			free(new_line);
		}
		/*next line*/
		continue;
	}
	
	/*save the IC, DC, amount of symbols and the validity of the lines of the file*/
	new_count = malloc(sizeof(counters));
	memset(new_count, 0, sizeof(counters));
	new_count->IC = IC;
	new_count->DC = DC;
	new_count->sTable = sTable_count;
	new_count->lines = line_count;
	return *new_count;
}

/*counts the amont of commas in a given string*/
int count_comma(char *data_line)
{
	int i, count = 0;
	
	for(i = 0; i < strlen(data_line); i++){
		if(data_line[i] == ','){
			count++;
		}
	}
	
	return count;
}

/*the second loop on the file, creates the extenal, entry and object files.*/
void finishFile(FILE *fp, int IC, int DC, int sTable_count, char *fileName, data **data_Table, valid_lines **file_line, symbol **symbol_Table , encoding **CPU)
{
	int i = 0, s_flag, a_flag, errno = 0; /*symbol_flag*/
	int  l_flag = 0, line_count = 0;
	int temp_IC;
	char line[LENGTH]; /* assuming max input is 80*/
	char *sptr, *temp, *comm, *comm2; 
	FILE *fp_ext;
	char *file_ext = malloc(strlen(fileName)+5);
	
	/*naming the extern file in case it's needed later*/
	sprintf(file_ext, "%s.ext", fileName);
	
	/*updating addresses*/
	while(i < sTable_count){
		if(symbol_Table[i]->data_flag){
			symbol_Table[i]->address += IC;
		}
		i++;
	}
	/*searching for an external symbol to decide if ext should be created*/
	i = 0;
	while(i < sTable_count){
		if(symbol_Table[i]->external){
			fp_ext = fopen(file_ext, "a");
			break;
		}
		i++;
	}
	
	IC = START; /*reseting IC*/
	
	while(fgets(line, sizeof(line), fp) != NULL){
		line_count++;
		/*creating copys of input line for later use*/
		comm = malloc(sizeof(line)+1);
		strcpy(comm, line);
		comm2 = malloc(sizeof(line)+1);
		strcpy(comm2, line);
		
		/*reading only lines that are marked as valid or those to be checked again (one of the operands is a label*/
		if(file_line[line_count]->not_valid == 0 || file_line[line_count]->not_valid == 2){ 
			if(isalpha(line[0])){ /*line starts with a label*/
				sptr = strtok(line, ":");
				temp = sptr;
				i = 0;
				a_flag = 0;
				/*check if the label exists in the symbol table -> which means it's a valid label*/
				while(i < sTable_count){
					if(strcmp(symbol_Table[i]->label, sptr) == 0){
						s_flag = 1;
						break;
					}
					i++;
				}
				if(s_flag){
					sptr = strtok(NULL, " \t\n"); /*get command*/
					a_flag = valid_action(sptr);
					if(a_flag >= 0){
						sptr = strtok(NULL, "\n"); /*get operands*/
						temp = sptr;
						if(file_line[line_count]->not_valid == 2){
							/*marked to be checked, with label*/
							if(valid_ops(a_flag, temp, symbol_Table)){ /*check validity of operands*/
								while (*comm != 0 && *(comm++) != ' ') {}; /*trim label*/
								comm = strtok(comm, " \t\n");
								temp_IC = IC;
								IC = create_bin_code(NULL, comm, IC, a_flag, l_flag, symbol_Table, CPU);
								if(temp_IC == IC){
									fprintf(stderr, "(%d) Error: could not create line\n", line_count);
									errno = 1;
								}
								else if(temp_IC +1 < IC){ /*line is valid and has 1 or more operands*/
									/*creating binary machine code*/
									while (*comm2 != 0 && *(comm2++) != ' ') {}; /*trim label*/
									comm = strtok(comm2, " \t\n");
									IC = create_bin_code2(fileName, comm2, temp_IC, a_flag, l_flag, symbol_Table, CPU, fp_ext);
									if(temp_IC == IC){
										fprintf(stderr, "(%d) Error: could not create line\n", line_count);
										errno = 1;
									}
								}
							}
						}
						else{ /*valid line with label*/
							while (*comm != 0 && *(comm++) != ' ') {};
							comm = strtok(comm, " \t\n");
							temp_IC = IC;
							IC = create_bin_code(NULL, comm, IC, a_flag, l_flag, symbol_Table, CPU);
							if(temp_IC == IC){
								fprintf(stderr, "(%d) Error: could not create line\n", line_count);
								errno = 1;
							}
							else if(temp_IC +1 < IC){
								/*creating binary machine code*/
								while (*comm2 != 0 && *(comm2++) != ' ') {};
								comm = strtok(comm2, " \t\n");
								IC = create_bin_code2(fileName, comm2, temp_IC , a_flag, l_flag, symbol_Table, CPU, fp_ext);
								if(temp_IC == IC){
									fprintf(stderr, "(%d) Error: could not create line\n", line_count);
									errno = 1;
								}
							}
						}
					}/*end a_flag*/
					else if(strcmp(sptr, ".extern") == 0){
					
					}
					else if(strcmp(sptr, ".entry") == 0){
						/*will send the label name to the entry file*/
						sptr = strtok(NULL, " \t\n");
						i = check_label(sptr, symbol_Table);
						if(i>=0 && errno == 0){
							fentry(fileName, i, IC, symbol_Table);
						}
						else{
							fprintf(stderr, "(%d) Error: %s is not a valid operand\n", line_count, sptr);
							errno = 1;
						}
					}
				}/*end s_flag*/
				else{
					temp = sptr;
					sptr = strtok(NULL, " \t"); /*get command*/
					if(strcmp(sptr, ".extern") == 0){
					
					}
					else if(strcmp(sptr, ".entry") == 0){
						/*will send the label name to the entry file*/
						sptr = strtok(NULL, " \t\n");
						i = check_label(sptr, symbol_Table);
						if(i>=0 && errno == 0){
							fentry(fileName, i, IC, symbol_Table);
						}
						else{
							fprintf(stderr, "(%d) Error: %s is not a valid operand\n", line_count, sptr);
							errno = 1;
						}
					}
					else{
						fprintf(stderr, "(%d) Error: could not create code, either label %s does not exist or there was a problem creating the binary code for the command\n", line_count, sptr);
						errno = 1;
					}
				}
			}/*end of label reading*/
			else if(line[0] == ';'){ /*if it's a comment*/
			
			}
			else if(line[0] == '.') { 
				sptr = strtok(line, " ");
				if(strcmp(sptr, ".extern") == 0){
				
				}
				else if(strcmp(sptr, ".entry") == 0){
					/*will send the label name to the entry file*/
					sptr = strtok(NULL, " \t\n");
					i = check_label(sptr, symbol_Table);
					if(i>=0 && errno == 0){
						fentry(fileName, i, IC, symbol_Table);
					}
					else{
						fprintf(stderr, "(%d) Error: %s is not a valid operand\n", line_count, sptr);
						errno = 1;
					}
				}
				else{
					fprintf(stderr, "(%d) Error: %s is not a valid instruction\n", line_count, sptr);
					errno = 1;
				}
			}
			else if(line[0] == ' ' || line[0] == '\t') { 
				if((sptr = strtok(line, " \t\n")) != NULL){/*get command*/
					a_flag = valid_action(sptr);
					if(a_flag >= 0){
						sptr = strtok(NULL, "\n"); /*get operands*/
						temp = sptr;
						if(file_line[line_count]->not_valid == 2){
							if(valid_ops(a_flag, temp, symbol_Table)){
								comm = strtok(comm, " \t\n");
								temp_IC = IC;
								IC = create_bin_code(NULL, comm, IC, a_flag, l_flag, symbol_Table, CPU);
								if(temp_IC == IC){
									fprintf(stderr, "(%d) Error: could not create line\n", line_count);
									errno = 1;
								}
								else if(temp_IC +1 < IC){
									/*creating binary machine code*/
									while (*comm2 != 0 && (*(comm2) >= 'a' && *(comm2++) <= 'z')) {};
									comm2 = strtok(comm2, " \t\n");
									IC = create_bin_code2(fileName, comm2, temp_IC , a_flag, l_flag, symbol_Table, CPU, fp_ext);
									if(temp_IC == IC){
										fprintf(stderr, "(%d) Error: could not create line\n", line_count);
										errno = 1;
									}
								}
							}
						}
						else{
							comm = strtok(comm, " \t\n");
							temp_IC = IC;
							IC = create_bin_code(NULL, comm, IC, a_flag, l_flag, symbol_Table, CPU);
							if(temp_IC == IC){
								fprintf(stderr, "(%d) Error: could not create line\n", line_count);
								errno = 1;
							}
							else if(temp_IC +1 < IC){
								/*creating binary machine code*/
								comm = strtok(comm2, " \t\n");
								IC = create_bin_code2(fileName, comm2, temp_IC, a_flag, l_flag, symbol_Table, CPU, fp_ext);
								if(temp_IC == IC){
									fprintf(stderr, "(%d) Error: could not create line\n", line_count);
									errno = 1;
								}
								
							}
						}
					}/*end a_flag*/
					else if(strcmp(sptr, ".data") == 0){ 
						
					}
					else if(strcmp(sptr, ".string") == 0){ 
						
					}
					else if(strcmp(sptr, ".extern") == 0){
					
					}
					else if(strcmp(sptr, ".entry") == 0){
						/*will send the label name to the entry file*/
						sptr = strtok(NULL, " \t\n");
						i = check_label(sptr, symbol_Table);
						if(i>=0 && errno == 0){
							fentry(fileName, i, IC, symbol_Table);
						}
						else{
							fprintf(stderr, "(%d) Error: %s is not a valid opernad\n", line_count, sptr);
							errno = 1;
						}
					}
					else{
						fprintf(stderr, "(%d) Error: not a valid command\n", line_count);
						errno = 1;
					}
				}
			}
			else{
				/*printf("skipping line\n");*/
			}
		}/*end if valid line*/
			
		/*next line*/
		continue;
	}	
	/*move IC to the end of the table incase of mismatch*/
	while(CPU[IC] != NULL){
		IC++;
	}
	
	/*adding the data table*/
	i = 0;
	while(data_Table[i] != NULL){
		add_code_data(data_Table[i], IC++, CPU);
		i++;
	}
	
	/*creating the object file*/
	if(errno == 0){
		create_ob(fileName, IC, DC, CPU);
	}
}

