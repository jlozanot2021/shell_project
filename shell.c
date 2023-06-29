/************************************************
 * Author: Vera T. Pascual
 * Goal: definition of functions in 'Shell' project
************************************************/

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <err.h>
#include <pwd.h>

#include "shell.h"

enum {
	MAX_SIZE = 255,
  MaxPath = 1024,
  BUFF_SIZE = 30000,
};

void
print_prompt()
{
	char cwd[MAX_SIZE + 1];

	// Obtener el nombre de usuario, el nombre del host y el directorio actual
	getcwd(cwd, MAX_SIZE);

  printf("\033[1;31m");
	printf("%s$ ",cwd);

	// Restablecer el color y el estilo
	printf("\033[0m");
	fflush(stdout);
}

int
usage()
{
	fprintf(stderr, "usage: ./shell\n");
	return (EXIT_FAILURE);
}

int
check_bad_input(char *std_msg)
{
  int i, bad_in;

  if (std_msg[0] == '\n') { // if only 'new line' is inserted
    bad_in = -1;
  }
  
  for (i = 0; std_msg[i] ; i++) { // if only 'space + new line' is inserted
    if (std_msg[i] > ' ') {
      bad_in = 1;
      break;
    }
    if (std_msg[i+1] == '\n') {
      bad_in = -1;
      break;
    }
  }
  return bad_in;
}

void
free_mem(Program_Input *p){
  int i, size;

  if (p->in_file) {
    size = sizeof(p->in_file);
    memset(p->in_file, 0, size);
  }
  if (p->out_file) {
    size = sizeof(p->in_file);
    memset(p->out_file, 0, size);
  }

  for (i = 0; i < p->num_pipes+1; i++) {
    if (p->cmd[i]) {
      free(p->cmd[i]);
    }
  }

  free(p);
}

void
execute(Command *command)
{
  char *dir[] = { "/bin", "/usr/bin", NULL };
	char path[MaxPath];
	int i;

  // cmd execution 
  // execution of no built-in commands

  for (i = 0; dir[i] != NULL; i++) {
    sprintf(path, "%s/%s", dir[i],command->cmd_arg[0]); // the cmd is searched in dir[i]
    execv(path, command->cmd_arg);  // cmd execution
    perror("program execution failed");
  }
}

char*
del_tabs(char *std_msg)
{
  int i;
  char *last;

  while (*std_msg == ' ' || *std_msg == '\t'){  // Beginning of input
    std_msg++;
  }
  
  if(*std_msg == 0){  // If (once tabs at beginning are del) input is null
    return std_msg;
  }

  last = std_msg + strlen(std_msg) - 1;

  while (last > std_msg && (*last == ' ' || *last == '\t')) { // End of input
    last--;
  }

  *(last+1) = 0;  // Character null at the end of line

  for (i = 0; std_msg[i]; i++){ // If tab is in the middle of input
    if(std_msg[i] == '\t'){
      std_msg[i] = ' ';
    }
  }

  return std_msg;
}

Command*
tokenize(char *arg)
{
  char *path, *ptr_dollar,  *env;
	int j = 0;

  Command *cmd = malloc(sizeof(Command));
  memset(cmd,0,sizeof(Command ));

  arg = del_tabs(arg);

  path = strtok_r(arg, " ", &arg);
	while (path != NULL) {	// while there are words in 'arg'
    

    if ((ptr_dollar = strrchr(path, '$')) != NULL) {
      ptr_dollar++;
      if ((env = getenv(ptr_dollar)) == NULL) {
        fprintf(stderr, "error: var %s does not exist", ptr_dollar); // SI VA ????????????????????????
        //exit(EXIT_FAILURE);
      }
      cmd->cmd_arg[j] = env;
    } else {
      //fprintf(stderr, "error: var %s does not exist\n", path);
      cmd->cmd_arg[j] = path;

    }
		j++;
    path = strtok_r(NULL, " ", &arg);
	}
  cmd->argc = j;

  return cmd;
}

void
def_var(char *input)
{
  char *name, *value, *symbol;
  int i;

  if ((symbol = strrchr(input, '=')) != NULL) { // divides input by '=' symbol
    name = strtok(input, "=");  // left-side part is variable's name
    value = strtok(NULL, "=");  // right-side part is variable's value

    if ((name != NULL) && (value != NULL)) {
      if(setenv(name, value, 1) != 0){  // set new variable
        warn("program failed\n");
        exit (EXIT_FAILURE);
      }
      memset(input, 0, sizeof(input[i])); // set as null (delete) 'input' value
    }
  }
}

Program_Input *
read_input(char * std_msg)
{
  Program_Input *p = malloc(sizeof(Program_Input));
  char *input, *output, *pp, *aux ;
  int i;

  memset(p, 0, sizeof(Program_Input));

  // fix Program_Input 'p' values
  strrchr(std_msg, '\n')[0] = '\0';
  def_var(std_msg);

  if ((aux = strrchr(std_msg, '&')) != NULL){
    p->background = 1;
    p->in_file = "/dev/null";
    *aux = '\0';  // delete char '&'
  } else {
    p->background = 0;
  }

  aux = std_msg;
  if ((input = strrchr(std_msg, '<')) != NULL) {
    input++;
    p->in_file = strtok_r(input," ",&input);
    printf("file: %s\n", p->in_file);
    strrchr(std_msg, '<')[0] = '\0';
  } else {
    p->in_file = NULL;
  }

  if ((output = strrchr(aux, '>')) != NULL) {
    output++;
    p->out_file = strtok_r(output," ",&output);
    strrchr(aux, '>')[0] = '\0';
  } else {
    p->out_file = NULL;
  }

  pp = strtok_r(std_msg, "|", &std_msg);

  for (i = 0; pp != NULL; i++) {
    p->num_pipes = i;    
    p->cmd[i] = tokenize(pp);
    pp = strtok_r(NULL, "|", &std_msg);
  }
  
  return p;
}

int
child(Program_Input *p, int bg, char *in_fd, char *out_fd, int i, int commands)
{
  // ARG 'int i' refers to number of cmd inside pipeline !!
  // ARG 'int commands' refers to total number of cmds !!
  int fd_in = -1, fd_out = -1, j;
  pid_t pid;
  //printf("num i : %d\n", i);
  //printf("num cmds : %d\n", commands);
  pid = fork();

  if (pid < 0) {		// error in fork
    perror("fork 1 failed");
    exit(EXIT_FAILURE);
  }

  if (pid == 0) {
    for(j = 0; j < commands - 1; j++){
      if( j == i || j + 1 == i ){
        continue;
      }
      close(p->cmd[j]->cmd_fd[0]);
      close(p->cmd[j]->cmd_fd[1]);
    }
    if (i == 0){ // first cmd in pipeline
      if (in_fd != NULL) { // if there is input from redirection file 
        fd_in = open(in_fd, O_RDONLY);
        if (fd_in < 0) {
          fprintf(stderr, "error openning file origin/destination");
          exit(EXIT_FAILURE);
        }
        dup2(fd_in, 0);
      }
      close(p->cmd[i]->cmd_fd[0]);
      if (p->cmd[i]->cmd_fd[1] != STDOUT_FILENO) {
			  dup2(p->cmd[i]->cmd_fd[1], STDOUT_FILENO);
			  close(p->cmd[i]->cmd_fd[1]);
      }
    }
    else if ( (i + 1) ==  commands){
      if (out_fd != NULL) { // if output to redirection file is needed
        fd_out = open(out_fd, O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU);
        if (fd_out < 0) {
          fprintf(stderr, "error openning file origin/destination");
        }
        dup2(fd_out, 1);
      }

      close(p->cmd[i - 1]->cmd_fd[1]);

      if (p->cmd[i - 1]->cmd_fd[0] != STDIN_FILENO){
        dup2(p->cmd[i - 1]->cmd_fd[0], STDIN_FILENO);
			  close(p->cmd[i - 1]->cmd_fd[0]);
      }
    }
    else if( (i != 0) && (i + 1 != commands)){
      close(p->cmd[i]->cmd_fd[0]);
      close(p->cmd[i - 1]->cmd_fd[1]);
      if (p->cmd[i - 1]->cmd_fd[0] != STDIN_FILENO){
        dup2(p->cmd[i -1]->cmd_fd[0], STDIN_FILENO);
			  close(p->cmd[i -1]->cmd_fd[0]);
      }
      if (p->cmd[i]->cmd_fd[1] != STDOUT_FILENO) {
			  dup2(p->cmd[i]->cmd_fd[1], STDOUT_FILENO);
			  close(p->cmd[i]->cmd_fd[1]);
      }
    }
    //if( i + 1 != commands){
  	//  if (c->cmd_fd[1] != STDOUT_FILENO) {
		//	  dup2(c->cmd_fd[1], STDOUT_FILENO);
		//	  close(c->cmd_fd[1]);
    //  }
		//}
    execute(p->cmd[i]);
    close(p->cmd[i]->cmd_fd[0]);
    close(p->cmd[i]->cmd_fd[1]);

    //-----Nota : aqui debes poner exitsuccess cuando llegues a lo del ifnot / ifok
    exit(EXIT_FAILURE);

	}
    if ( fd_out != -1 ){
      close(fd_out);
    }
    if (fd_in != -1){
      close(fd_in);
    } 
  return pid;
}
int
program(Program_Input *p)
{
  int status;
	pid_t pid;
  int j, fd_in = -1, fd_out = -1, stat, i = 0;

  stat = 0;

//-------------------------------------------------------------------------------------------------------
//Nota: puedes poner este bloque dentro de si no hay pipes ya que es un poco raro poner cd | ....

  if (!strcmp(p->cmd[0]->cmd_arg[0], "exit")){  // NO VA !!!!!!!!!!!!!!!!!!!!!
    printf("\n--exiting shell--\n");
    free_mem(p);
    exit (EXIT_SUCCESS);

  } else if (!strcmp(p->cmd[0]->cmd_arg[0], "cd")) {  // execution of 'cd' built-in
    if (p->cmd[0]->argc == 1) { // if 'cd' has no arguments, the arg is user's $HOME
      chdir(getenv("HOME"));

    } else if (p->cmd[0]->argc == 2) {  // if 'cd' has 1 arg, change directory to arg
      chdir(p->cmd[0]->cmd_arg[1]);

    } else {  // if 'cd' has more than 1 arg, error
      fprintf(stderr, "error: too much arguments in cd command\n");
      exit (EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
  }

//------------------------------------------------------------------------------------------------------
  
  if (p->num_pipes == 0){ // there are no pipes
    
    pid = fork();
    if (pid < 0) {		// error in fork
      perror("fork 1 failed");
      return EXIT_FAILURE;
    }

    if (pid == 0) {
      if (p->in_file != NULL) { // if there is input from redirection file 
        fd_in = open(p->in_file, O_RDONLY);
        if (fd_in < 0) {
          fprintf(stderr, "error openning file origin/destination");
          exit(EXIT_FAILURE);
        }
        dup2(fd_in, 0); // input file
      } 
      if (p->out_file != NULL) {  // if output is to redirection file 
        fd_out = open(p->out_file, O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU);
        if (fd_out < 0) {
          fprintf(stderr, "error openning file origin/destination");
          exit(EXIT_FAILURE);
        }
        dup2(fd_out, 1);  // output file
      }
      execute(p->cmd[i]); // execute cmd

      if (fd_out != -1){
        close(fd_out);
      }
      if (fd_in != -1){
        close(fd_in);
      }
    }
    
    if (p->background == 0) { // if cmd is in background, do not wait for child
      waitpid(pid, &status, 0);
    }

  } else { // if there is 1 or more pipes
    for (j = 0; j < p->num_pipes; j++) {
      if (pipe(p->cmd[j]->cmd_fd) == -1) {	// pipe initialization
        perror("pipe failed");
        exit(EXIT_FAILURE);
      }
    }

    for (j = 0; j < (p->num_pipes + 1); j++){
      pid = child(p, p->background, p->in_file, p->out_file, j,(p->num_pipes + 1));
      // if (p->cmd[j]->cmd_fd[1] != 1) {
			  //close(p->cmd[j]->cmd_fd[1]);
		  // }
      
      //if (j != p->num_pipes){
      //  memcpy(p->cmd[j + 1]->cmd_fd, p->cmd[j]->cmd_fd, sizeof(p->cmd[j]->cmd_fd));
      //} 
     
    }

    for (j = 0; j < p->num_pipes; j++) {
      close(p->cmd[j]->cmd_fd[0]);
      close(p->cmd[j]->cmd_fd[1]);
    }
  }
  if (p->background == 0) {  // si no hay background
    waitpid(pid, &status, 0); 
    if (WIFEXITED(status)){
      free_mem(p);
      stat = WEXITSTATUS(status);
      return stat;
    }
  }
  return stat;
}
