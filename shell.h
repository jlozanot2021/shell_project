/************************************************
 * Author: Vera T. Pascual
 * Goal: header for functions in 'Shell' project
************************************************/

#include <stdlib.h>
#include <stdio.h>

struct Program_Input {
  int background;
  int num_pipes;
  char * in_file;
  char * out_file;
  struct Command *cmd[100];
};

typedef struct Program_Input Program_Input;

struct Command {
  int argc;
  char *cmd_arg[255];
  int cmd_fd[2];
};

typedef struct Command Command;

//void print_prompt();
// prints program's usage and returns with failure
int usage();

// checks the standar input for not supported formats
int check_bad_input(char *std_msg);

// free memory
void free_mem (Program_Input *p);

// counts the number of spaces in 'arr'
int arr_length(char *arr);

// executes a program 'arr'
void execute(Command *command);

// tokenize words 
Command* tokenize(char *arg);

// save external variable defined by the user
void def_var(char *input);

// read from standar input
Program_Input* read_input(char * std_msg);

// creates pipes to execute the cmd line
void child(Command *c, int bg, char *in_fd, char *out_fd, int i, int commands);

// main structure of execution and others
int program(Program_Input *p);

// deletes tabulators in input
char* del_tabs(char *std_msg);

//void in_pipe(int fd[]);
//
//void out_pipe(int fd[]);