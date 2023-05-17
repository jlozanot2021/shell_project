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
  //int 
  int status = EXIT_SUCCESS;
  char std_msg[3000];

  if (argc != 1) {
    status = usage();
  }
  else {
    printf("\n--- SHELL by VERA T. PASCUAL GUIJARRO ---\n\n$ ");

    while(fgets(std_msg, 3000, stdin)) {
      if (check_bad_input(std_msg) == -1) {
        printf("$ ");
        continue;
      }
      pgm = read_input(std_msg);
      status = program(pgm);

      if (status == -1) {
        break;
        printf("\nAAAAAAAAAAAaaa ");
      }
      printf("$ ");

    }
    return EXIT_SUCCESS;
  }
  printf("\nBBBBBBBBBBBBBBBBbb");
  exit (EXIT_SUCCESS);
}