/************************************************
 * Author: Vera T. Pascual
 * Goal: main program for 'Shell' project
************************************************/

#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "shell.h"

int
main(int argc, char *argv[])
{
  Program_Input *pgm;
  int status = EXIT_SUCCESS;
  char std_msg[3000];

  if (argc != 1) {  // program's usage
    status = usage();
  }
  else {
    printf("\n--- SHELL by VERA T. PASCUAL GUIJARRO ---\n\n$ ");

    while(fgets(std_msg, 3000, stdin)) {  // read standar input from user
      if (check_bad_input(std_msg) == -1) { // true if '\n' or '\0\n' is  inserted
        printf("$ ");
        continue;
      }
      pgm = read_input(std_msg);
      status = program(pgm);

      if (status == -1) { // NO VA !!!!!!!!!!!!!!1
        break;
      }
      printf("$ ");

    }
    return EXIT_SUCCESS;
  }
  exit (EXIT_SUCCESS);  // JAMAS SALE, NO VA !!!!!!!!!!!1
}