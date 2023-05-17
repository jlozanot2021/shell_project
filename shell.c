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

#include "shell.h"

enum {
	MAX_SIZE = 255,
  MaxPath = 1024,
  BUFF_SIZE = 30000,
};

int
usage()
{
	fprintf(stderr, "usage: ./shell\n");
	return (EXIT_FAILURE);
}

int
check_bad_input (char *std_msg)
{
  int i, bad_in;

  if (std_msg[0] == '\n') { 
    bad_in = -1;
  }

  for (i = 0; std_msg[i] ; i++) {
//    if (std_msg[i] == '\t') { // NO VA !!!!!!!!!!!! ESTA EN TOKENIZZE
//      printf("tab\n");
//      std_msg[i] = ' ';
//      bad_in = 1;
//    }

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
  int i;

  for (i = 0; i < p->num_pipes+1; i++) {
    if (p->cmd[i] != NULL) {
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

  if (!strcmp(command->cmd_arg[0], "exit")){  // NO VA !!!!!!!!!!!!!!!!!!!!!
    printf("\n--exiting shell--\n");
    //return -1;
    exit(EXIT_SUCCESS);

  } else if (!strcmp(command->cmd_arg[0], "cd")) {  // execution of 'cd' built-in
    if (command->argc == 1) { // if 'cd' has no arguments, the arg is user's $HOME
      chdir(getenv("HOME"));

    } else if (command->argc == 2) {  // if 'cd' has 1 arg, change directory to arg
      chdir(command->cmd_arg[1]);

    } else {  // if 'cd' has more than 1 arg, error
      fprintf(stderr, "error: too much arguments in cd command\n");
      exit (EXIT_FAILURE);
    }

  } else {  // execution of no built-in commands

    for (i = 0; dir[i] != NULL; i++) {
      sprintf(path, "%s/%s", dir[i],command->cmd_arg[0]); // the cmd is searched in dir[i]
      execv(path, command->cmd_arg);  // cmd execution
      perror("program execution failed");
    }
  }
	
  //return 1;
}

Command*
tokenize(char *arg)
{
  char *path, *ptr_dollar,  *env;
	int i, j = 0;

  Command *cmd = malloc(sizeof(Command));
  memset(cmd,0,sizeof(Command ));


  path = strtok_r(arg, " ", &arg);
	while (path != NULL) {	// while there are words in 'arg'

    //if ((ptr_tab = strrchr(path, '\t')) != NULL) {
    //  printf("tab\n");
    //  fprintf(stderr, "AAAAAAA %s does not exist\n", path);
    //  ptr_tab[j] = '\0';
    //  fprintf(stderr, "VVVVVVVVV %s does not exist\n", path);
    //}

    for (i = 0; path[i]; i++) {
      if (path[i] == '\t') {  // NO VA !!!!!!!!!!!!
        printf("tab in %d\n", i);
        path[i] = ' ';
      }
    }
    if ((ptr_dollar = strrchr(path, '$')) != NULL) {
      ptr_dollar++;
      if ((env = getenv(ptr_dollar)) == NULL) {
        fprintf(stderr, "error: var %s does not exist\n", ptr_dollar); // NO VA !!!!!!!!!!!!!!!!!!!!
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

  if ((symbol = strrchr(input, '=')) != NULL) {

    name = strtok(input, "=");
    value = strtok(NULL, "=");

    if ((name != NULL) && (value != NULL)) {
      if(setenv(name, value, 1) != 0){
        warn("program failed\n");
        exit (EXIT_FAILURE);
      }
      memset(input, 0, sizeof(input[i]));
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

void
child(Command *c, int bg, char *in_fd, char *out_fd, int i, int commands)
{
  int status, fd_in = -1, fd_out = -1;
  pid_t pid;

  pid = fork();

  if (pid < 0) {		// error in fork
    perror("fork 1 failed");
    exit(EXIT_FAILURE);
  }

  if (pid == 0) {
    if (i == 0){
            if (in_fd != NULL) { // if there is input from redirection file 
        fd_in = open(in_fd, O_RDONLY);
        if (fd_in < 0) {
          fprintf(stderr, "error openning file origin/destination");
        }
        dup2(fd_in, 0);
      }
    }
    if ( (i + 1) ==  commands){
            if (out_fd != NULL) {
        fd_out = open(out_fd, O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU);
        if (fd_out < 0) {
          fprintf(stderr, "error openning file origin/destination");
        }
        dup2(fd_out, 1);
      }
    }
    if( i != 0 ){
      if (c->cmd_fd[0] != STDIN_FILENO){
      dup2(c->cmd_fd[0], STDIN_FILENO);
			close(c->cmd_fd[0]);
    }
    }
    if( i + 1 != commands){
  	if (c->cmd_fd[1] != STDOUT_FILENO) {
			dup2(c->cmd_fd[1], STDOUT_FILENO);
			close(c->cmd_fd[1]);
    }
		}
    execute(c);
    close(c->cmd_fd[0]);
    close(c->cmd_fd[1]);

	} else {		// parent process
    if (bg == 0) {
      waitpid(pid, &status, 0);
            if ( fd_out != -1 ){
        close(fd_out);
      }
      if (fd_in != -1){
        close(fd_in);
      }
    }
  }
}

int
program(Program_Input *p)
{
  int status;
	pid_t pid;
  int j, fd_in = -1, fd_out = -1, stat, i = 0;

  stat = 0;

  if (p->num_pipes == 0){
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
        }
        dup2(fd_in, 0);
        //execute(p->cmd[i]);
        //
      } 
      if (p->out_file != NULL) {
        fd_out = open(p->out_file, O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU);
        if (fd_out < 0) {
          fprintf(stderr, "error openning file origin/destination");
        }
        dup2(fd_out, 1);
      }
      execute(p->cmd[i]);
      if ( fd_out != -1 ){
        close(fd_out);
      }
      if (fd_in != -1){
        close(fd_in);
      }
      
    }
    
    if (p->background == 0) { // if cmd is in background, do not wait for child
      waitpid(pid, &status, 0);
    }

  } else {
    for (j = 0; j < p->num_pipes; j++) {
      if (pipe(p->cmd[j]->cmd_fd) == -1) {	// pipe initialization
        perror("pipe failed");
        exit(EXIT_FAILURE);
      }
    }
    for (j = 0; j < (p->num_pipes + 1); j++){

      child(p->cmd[j], p->background, p->in_file, p->out_file, j,(p->num_pipes + 1));
      if (p->cmd[j]->cmd_fd[1] != 1) {
			  close(p->cmd[j]->cmd_fd[1]);
		  }
      
      if (j != p->num_pipes) {
        p->cmd[j + 1]->cmd_fd[0] = p->cmd[j]->cmd_fd[0];
        p->cmd[j + 1]->cmd_fd[1] = p->cmd[j]->cmd_fd[1];
      } 
      
    }

    for (j = 0; j < p->num_pipes; j++) {
      close(p->cmd[j]->cmd_fd[0]);
      close(p->cmd[j]->cmd_fd[1]);
    }
  }

  free_mem(p); // NO VA !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  return stat;
}