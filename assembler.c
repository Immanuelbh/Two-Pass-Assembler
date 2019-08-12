/*
*	assembler.c
*	author: Immanuel Ben Hefer, I.D. 309991644
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
/*prototyple declaration*/

void free_CPU(int total);
void free_symbol_Table(int count);
void free_data_Table(int count);
void free_file_line(int count);
void free_action_Table(int count);
void clear_counter(counters* count);

symbol **symbol_Table;
data **data_Table;
valid_lines **file_line;
encoding **action_Table;
encoding **CPU;

int main(int argc, char *argv[])
{
	FILE *fp;
	counters* count;
	char* txt_buffer;
	int i = 1;
	
	while(i < argc){
		txt_buffer = malloc(strlen(argv[i])+4);
		sprintf(txt_buffer, "%s.as", argv[i]);
		symbol_Table = (symbol **) malloc(sizeof(symbol *) * SIZE);
		data_Table = (data **) malloc(sizeof(data *) * (SIZE*2));
		file_line = (valid_lines **) malloc(sizeof(valid_lines *) * LINE_SIZE);
		action_Table = (encoding **) malloc(sizeof(encoding *) * LINE_SIZE);
		CPU = (encoding **) malloc(sizeof(encoding *) * CPU_SIZE);
		
		memset(symbol_Table, 0, SIZE*sizeof(symbol *));
		memset(data_Table, 0, SIZE*sizeof(data *));
		memset(file_line, 0, LINE_SIZE*sizeof(valid_lines *));
		memset(action_Table, 0, LINE_SIZE*sizeof(encoding *));
		memset(CPU, 0, CPU_SIZE*sizeof(encoding *));
		
		fp = fopen(txt_buffer, "r");
		if(!fp){
			fprintf(stderr, "Error reading file %d: \"%s\"\n", i, argv[i]);
			exit(1);
		}
		count = malloc(sizeof(counters));
		*count = readFile(fp, symbol_Table, data_Table, file_line, action_Table); /*the first pass*/
		rewind(fp);
		finishFile(fp, count->IC, count->DC, count->sTable, argv[i], data_Table, file_line, symbol_Table, CPU); /*the second pass*/
		
		free_symbol_Table(SIZE);
		free(symbol_Table);
		
		free_data_Table(SIZE*2);
		free(data_Table);
		
		free_action_Table(count->actions);
		free(action_Table);
		
		free(count);
		clear_counter(count);
		free(txt_buffer);
		txt_buffer = NULL;
		i++;
	}
	
	return (0);
}

void free_symbol_Table(int count)
{
	int i;
	for(i = 0; i < count ; i++){
	  if(symbol_Table[i] != NULL) 
	  {
		free(symbol_Table[i]);
		symbol_Table[i] = NULL;
	  }
	}
	
}

void free_data_Table(int count)
{
	int i;
	for(i = 0; i < count ; i++){
	  if(data_Table[i] != NULL) 
	  {
		free(data_Table[i]);
		data_Table[i] = NULL;
	  }
	}
	
}

void free_action_Table(int count)
{
	int i;
	for(i = 0; i < count ; i++){
	  if(action_Table[i] != NULL) 
	  {
		free(action_Table);
		action_Table[i] = NULL;
	  }
	}
	
}

void clear_counter(counters* count)
{	
	count = NULL;
}


