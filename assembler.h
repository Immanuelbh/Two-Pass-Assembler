#define SIZE 30
#define LINE_SIZE 300


typedef struct command{
	unsigned int are : 2;
	unsigned int dest :2;
	unsigned int source :2;
	unsigned int opCode :4;
	unsigned int group :2;
	unsigned int unused :3;
} command;

typedef struct label_address{
	unsigned int are :2;
	unsigned int bin_address :13;
} label_address;

typedef struct registers{
	unsigned int are :2;
	unsigned int reg1 :6;
	unsigned int reg2 :6;
	unsigned int unused :1;
} registers;

typedef struct direct{
	unsigned int are :2;
	unsigned int value :13;
} direct;

typedef struct data{
	unsigned int value :15;
} data;


typedef struct encoding{
	
	unsigned int act :1;
	unsigned int add :1;
	unsigned int reg :1;
	unsigned int dat :1;
	unsigned int dir :1;
	
	union memory{
		label_address new_ad;
		command action;
		registers new_reg;
		data new_data;
		direct new_direct;
	}memory;
}encoding;

typedef struct symbol{
	unsigned action :1;
	unsigned external :1;
	unsigned data_flag :1;
	unsigned declared :1;
	int address;	
	char label[SIZE];

} symbol ; 

typedef struct counters{
	int IC;
	int DC;
	int sTable;
	int lines;
	int actions;
} counters;

typedef struct valid_line{
	unsigned not_valid :2;
} valid_lines;

/*functions*/

/*addition*/
void add_code_com(command *bin_code, int IC, encoding **table);
void add_code_op(char* operand, int IC, symbol **symbol_Table, encoding **table);
void add_code_data(data *to_add, int IC, encoding **table);
void add_regs(char* reg1, char* reg2, int IC, encoding **table);
int add_data(int buffer[], int tok_count, int DC, data **data_Table);
int add_symbol(symbol* new_symbol, int count, symbol **symbol_Table);

/*validation*/
int valid_label(char *label);
int valid_string(char *string);
int valid_action(char *action);
int valid_reg(char *reg);
int valid_direct(char* direct);
int valid_ind_reg(char* input);
int valid_num(char *num);
int valid_dest(int opCode, char* operand);
int valid_ops(int num_act, char *operand, symbol **symbol_Table);
int check_label(char* label, symbol **symbol_Table);
int label_dec(char* label, symbol **symbol_Table);
int label_ext(char* label, symbol **symbol_Table);
int label_act(char* label, symbol **symbol_Table);
int readString(char *string, int DC, data **data_Table);
void line_is_valid(valid_lines* new_line, int count, valid_lines **file_line);
int no_op(char* action);
int one_op(char* action);
int two_op(char* action);
int setDest(char *dest, symbol **symbol_Table);
int setSource(char *source, symbol **symbol_Table);

/*create*/
int create_bin_code(char *label, char *sptr, int IC, int a_flag, int l_flag, symbol **symbol_Table, encoding **table);
int create_bin_code2(char* fileName, char *sptr, int IC, int a_flag, int l_flag, symbol **symbol_Table, encoding **table, FILE *fp_ext);
void fexternal(FILE *fp_ext, int IC, symbol *ssymbol);
void fentry(char *fileName, int i, int IC, symbol **symbol_Table);
void create_ob(char* fileName, int org_IC, int DC, encoding **table);

/*fileLoops*/
counters readFile(FILE *fp, symbol **symbol_Table, data **data_Table, valid_lines **file_line, encoding **action_Table);
void finishFile(FILE *fp, int IC, int DC, int sTable_count, char *fileName, data **data_Table, valid_lines **file_line, symbol **symbol_Table , encoding **action_Table);





/*
void print_node(symbol *to_print);
void print_data(data  *to_print, int DC);
void print_sTable(int total, symbol **symbol_Table);
void print_code(int IC, encoding **table);
void print_all(int total, encoding **table);
*/

