/*
*	create.c
*	contains all function that relate to code creation
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
#define C_CMP 1
#define C_LEA 6
#define C_PRN 12

/*creates the initial code(actions)*/
int create_bin_code(char *label, char *sptr, int IC, int a_flag, int l_flag, symbol **symbol_Table, encoding **table)
{
	int reg_flag = 0, op, errno = 0;
	int strt_IC = IC;
	char* temp, *action;
	command* bin_code = {0};
	
	action = sptr;
	/*creating binary machine code*/
	bin_code = malloc(sizeof(command));
	memset(bin_code, 0, sizeof(command));
	bin_code->unused = 7; /*111*/
	switch(a_flag) {/*amount of operands*/
		case 0:
			bin_code->opCode = no_op(sptr); /*direct input because we know action is valid*/
			bin_code->source = 0;
			bin_code->dest = 0;
			bin_code->group = 0;
			if((sptr = strtok(NULL, " \t\n")) != NULL){
				fprintf(stderr, "Error: %s has too many operands (%s)\n", action, sptr);
				errno = 1;
			}
			else{
				add_code_com(bin_code, IC, table);
				
			}
			
			break;
		case 1:
			bin_code->group = 1; /*10*/
			bin_code->source = 0;
			op = one_op(sptr);
			if(op >= 0){
				bin_code->opCode = op;
			}
			else{
				fprintf(stderr, "Error: the command %s is not valid\n", sptr);
				errno = 1;
				break;
			}
			
			IC++;
			
			if((sptr = strtok(NULL, " \t\n")) != NULL){
				if(op == C_PRN){ /*the command 'prn' has specific demands*/
					if(valid_direct(sptr) || valid_label(sptr) || valid_ind_reg(sptr) || valid_reg(sptr)){
						if(setDest(sptr, symbol_Table) >= 0){
							bin_code->dest = setDest(sptr, symbol_Table);
							if((sptr = strtok(NULL,"\n")) == NULL){
								add_code_com(bin_code, IC - a_flag, table);
								
							}
							else{
								fprintf(stderr, "Error: The command \"%s\" has too many operands (%s)\n", action, sptr);
								errno = 1;
							}
						}
					}
					else{
						fprintf(stderr, "Error: %s is not a valid destination\n", sptr);
						errno = 1;
					}
					
				}
				else{ /*all other commands*/
					if(valid_label(sptr) || valid_ind_reg(sptr) || valid_reg(sptr)){ 
						if(setDest(sptr, symbol_Table) >= 0){
							bin_code->dest = setDest(sptr, symbol_Table);
							if((sptr = strtok(NULL,"\n")) == NULL){
								add_code_com(bin_code, IC - a_flag, table);
							}
							else{
								fprintf(stderr, "Error: The command \"%s\" has too many operands (%s)\n", action, sptr);
								errno = 1;
							}
						}
					}
					else{
						fprintf(stderr, "Error: %s is not a valid destination\n", sptr);
						errno = 1;
					}
				}
			}
			else{
				fprintf(stderr, "Error: missing operand\n");
				errno = 1;
			}
			break;
		case 2:
			bin_code->group = 2; /*11*/
			op = two_op(sptr);
			if(op >= 0){
				bin_code->opCode = op;
			}
			else{
				fprintf(stderr, "the command %s is not valid\n", sptr);
				break;
			}
			
			IC++;
			
			/*setting source*/
			if((sptr = strtok(NULL, " ,\t\n")) != NULL){
				if(op == C_LEA){ /*the command 'lea' has specific demands*/
					if(valid_label(sptr) || valid_ind_reg(sptr)){
						bin_code->source = setSource(sptr, symbol_Table);
						IC++;
					}
					else{
						fprintf(stderr, "Error: %s is not valid, must be a valid label or index register. (lea)",sptr);
						errno = 1;
						break;
					}
				}
				else{/*all other commands*/
					if(valid_direct(sptr) || valid_label(sptr)|| valid_ind_reg(sptr) || valid_reg(sptr)){
						if(setSource(sptr, symbol_Table) >= 0){
							bin_code->source = setSource(sptr, symbol_Table);
							IC++;
							if(valid_reg(sptr)){ /*if the first operand is a register, flag it*/
								reg_flag++;
							}
						}
					}
					else{
						fprintf(stderr, "Error: %s is not a valid source\n", sptr);
						errno = 1;
						break;
					}
				}
			}
			else{
				fprintf(stderr, "Error: missing first operand (source): %d\n", IC);
				errno = 1;
				break;
			}
			/*setting destination*/
			if((sptr = strtok(NULL, " \t\n")) != NULL){
				temp = sptr;
				if(op == C_CMP){ /*the command 'cmp' has specific demands*/
					if(setDest(sptr, symbol_Table) >= 0){
						bin_code->dest = setDest(sptr, symbol_Table);
						if((sptr = strtok(NULL, "\n")) == NULL){
							add_code_com(bin_code, IC - a_flag, table);
							if(valid_reg(temp)){/*if the second operand is also a register, flag it again*/
								reg_flag++;
							}
						}
						else{
							fprintf(stderr, "Error: The command \"%s\" has too many operands (%s)", action, sptr);
							errno = 1;
							break;
						}
						
					}
					else{
						fprintf(stderr, "Error: %s is not a valid destination\n", sptr);
						errno = 1;
						break;
					}
				}
				else{/*all other commands*/
					if(valid_label(sptr) || valid_ind_reg(sptr) || valid_reg(sptr)){
						if(setDest(sptr, symbol_Table) >= 0){
							bin_code->dest = setDest(sptr, symbol_Table);
							if((sptr = strtok(NULL, "\n")) == NULL){
								add_code_com(bin_code, IC - a_flag, table);
								if(valid_reg(temp)){/*flag if operand is a register*/
									reg_flag++;
								}
							}
							else{
								fprintf(stderr, "Error: The command has too many operands (%s)", sptr);
								errno = 1;
								break;
							}
						}
						
					}
					else{
						fprintf(stderr, "Error: %s is not a valid destination\n", sptr);
						errno = 1;
						break;
					}
				}
					
			}
			else{
				fprintf(stderr, "Error: missing second operand (destination)\n");
				errno = 1;
			}
		
			break;
	}
	
	/*creating new entry for the symbol table*/
	if(l_flag){ /*label_flag*/
		if(table[IC-a_flag] != NULL){ /*if the bin code was completed*/
			IC++;
		}
		
		if(reg_flag == 2){ /*both operands are registers, meaning they need only one 'word'*/
			--IC;
		}
	}
	else{
		if(reg_flag != 2){ /*all other options*/
			IC++;
		}
	}
	
	if(errno){ /*if program ran into an error*/
		IC = strt_IC;
	}
	
	free(bin_code);
	
	return IC;
}


/*creating the binary code for all other types other then action, this function follows the first and can be accessed only if the user's line is valid.*/
int create_bin_code2(char* fileName, char *sptr, int IC, int a_flag, int l_flag, symbol **symbol_Table, encoding **CPU, FILE *fp_ext)
{
	int i = 0; 
	char *temp;
	
	switch(a_flag) { 
		case 0:
			IC++;
			break;
		case 1:
			if((sptr = strtok(NULL, " \t\n")) != NULL){
				if(CPU[IC] != NULL && CPU[IC]->act){
					IC++;
					add_code_op(sptr, IC, symbol_Table, CPU);
					i = check_label(sptr, symbol_Table);
					if(i>=0){
						if(symbol_Table[i]->external){ /*if operand is declared as external*/
							if(fp_ext){
								fexternal(fp_ext, IC, symbol_Table[i]);
							}
							else{
								fprintf(stderr, "Error: fp_ext is NULL\n");
							}
						}
					}
					IC++;
					
					if((sptr = strtok(NULL, " \t\n")) != NULL){
						fprintf(stderr, "Error: too many operands: %s is not valid\n", sptr);
					}
				}
				
			}
			else{
				fprintf(stderr, "Error: missing operand\n");
			}
			break;
		case 2:
			/*setting source*/
			if((sptr = strtok(NULL, " ,\t")) != NULL){
					temp = sptr;
					
			}
			else{
				printf("spt: %s\n", sptr);
				fprintf(stderr, "Error: missing first operand (source)\n");
			}
			/*setting destination*/
			if((sptr = strtok(NULL, " \t\n")) != NULL){
				if(valid_reg(temp) && valid_reg(sptr)){ /*if both operands are registers*/
				
					if(CPU[IC]->act){/*the action binary code was created successfully */
						IC++;
						add_regs(temp, sptr, IC, CPU);
						IC++;
					}
				}
				else{
					if(CPU[IC]->act){
						IC++;
						add_code_op(temp, IC, symbol_Table, CPU); 
						i = check_label(temp, symbol_Table);
						if(i>=0){
							if(symbol_Table[i]->external){
								if(fp_ext){
									fexternal(fp_ext, IC, symbol_Table[i]);
								}
								else{
									fprintf(stderr, "Error: fp_ext is NULL\n");
								}
							}
							
						}
						
						IC++;
						add_code_op(sptr, IC, symbol_Table, CPU);
						i = check_label(sptr, symbol_Table);
						if(i>=0){
							if(symbol_Table[i]->external){
								if(fp_ext){
									fexternal(fp_ext, IC, symbol_Table[i]);
								}
								else{
									fprintf(stderr, "Error: fp_ext is NULL\n");
								}
							}
						}
						IC++;
					}
				}
				
			}
			else{
				fprintf(stderr, "Error: missing second operand (destination)\n");
			}

			break;
	}/*end switch*/
	
	
	return IC;
}

/*the function prints the desired symbol to the extern file*/
void fexternal(FILE *fp_ext, int IC, symbol *ssymbol)
{
	if(fp_ext){
		fprintf(fp_ext, "%s", ssymbol->label);
		
		if(ssymbol->address < OCTET){ 
			fprintf(fp_ext, "	%02X\n", IC);
		}
		else{
			fprintf(fp_ext, "	%03X\n", IC);
		}
		
	}
	else{
		fprintf(stderr, "Error: could not access file \n");
	}
	
}

/*adds the desired symbol to the entry file*/
void fentry(char *fileName, int i, int IC, symbol **symbol_Table)
{
	
	FILE *fp_ent; 
	char *file_ent = malloc(strlen(fileName)+5);
	
	sprintf(file_ent, "%s.ent", fileName);

	fp_ent = fopen(file_ent, "a");
	 
	if(fp_ent){
		fprintf(fp_ent, "%s", symbol_Table[i]->label);
		if(symbol_Table[i]->address < OCTET){ 
			fprintf(fp_ent, "	%02X\n", symbol_Table[i]->address);
		}
		else{
			fprintf(fp_ent, "	%03X\n", symbol_Table[i]->address);
		}
	
	}
	else{
		fprintf(stderr, "Error: could not create %s\n", file_ent);
	}

	free(file_ent);
}


/*creates the object file*/
void create_ob(char* fileName, int org_IC, int DC, encoding **CPU)
{
	FILE *fp_ob;
	int conc, i;
	char *file_ob = (char *)malloc((strlen(fileName))+4);
	sprintf(file_ob, "%s.ob", fileName);
	
	fp_ob = fopen(file_ob, "w");
	if(!fp_ob){
		fprintf(stderr, "Error: cound not create object\n");
	}
	else{
		fprintf(fp_ob, "Base 32		 Base 16\n");
		fprintf(fp_ob, "Address		Machine-Code\n");
		fprintf(fp_ob, "		%X	%X\n",org_IC - (START+DC), DC); /*size of the table*/
		i = START;
		while(i < org_IC){
			if(CPU[i]->act){
				conc = (CPU[i]->memory.action.unused << 2) | CPU[i]->memory.action.group;
				conc = (conc << 4) | CPU[i]->memory.action.opCode;
				conc = (conc << 2) | CPU[i]->memory.action.source;
				conc = (conc << 2) | CPU[i]->memory.action.dest;
				conc = (conc << 2) | CPU[i]->memory.action.are;
				
				fprintf(fp_ob, "	%X		 %04X\n", i, conc);
			}
			else if(CPU[i]->add){
				conc = (CPU[i]->memory.new_ad.bin_address << 2) | CPU[i]->memory.new_ad.are;
				fprintf(fp_ob, "	%X		 %04X\n", i, conc);
			}
			else if(CPU[i]->reg){
				conc = (CPU[i]->memory.new_reg.unused << 6) | CPU[i]->memory.new_reg.reg2;
				conc = (conc << 6) | CPU[i]->memory.new_reg.reg1;
				conc = (conc << 2) | CPU[i]->memory.new_reg.are;
				
				fprintf(fp_ob, "	%X		 %04X\n", i, conc);
			}
			else if(CPU[i]->dat){
				fprintf(fp_ob, "	%X		 %04X\n", i, CPU[i]->memory.new_data.value);
			}
			else if(CPU[i]->dir){
				conc = (CPU[i]->memory.new_direct.value << 2) | CPU[i]->memory.new_direct.are;
				fprintf(fp_ob, "	%X		 %04X\n", i, conc);
			}
			else{
				fprintf(stderr, "Error: code in CPU[%d] is not valid\n", i);
			
			}
			i++;
		}
	}
	
	/*free(file_ob);*/
	/*fclose(fp_ob);*/
}

