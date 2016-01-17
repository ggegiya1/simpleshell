/******************************************************************************
 * ch.c --- Un shell pour les hélvètes.                                       *
 *                                                                            *
 ******************************************************************************
 * C'est une ligne de commande, similaire a /bin/sh qui sait:                 *
 *         1. Exécuter la commande "cat Makefile".                            *
 *         2. Exécuter la commande "echo *".                                  *
 *         3. Exécuter la commande "cat <Makefile > foo".                     *
 *         4. Exécuter la commande "find -name Makefile | xargs grep ch".     *
 *                                                                            *
 ******************************************************************************
 * L'application a été réalisée par:                                          *
 *         Cordeleanu Corneliu < cordeleanu@gmail.com >                       *
 *         Georgiy Gegiya      < gegiya@gmail.com >                           *
 *                                                                            *
 ******************************************************************************
 * Compilateur gcc version 4.8.4 (Ubuntu 4.8.4-2ubuntu1~14.04)                *
 * IFT2245 05 - 28 Janvier 2016                                               *
 *                                                                            *
 *****************************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#define LINE_FEED		10		// Line feed
#define CARR_RETURN		13	
#define SPACE   		32		// Space
#define CHAR_NUL		0		// Null character

#define TRUE			1
#define FALSE   		0	

#define MAXLINE			256		// The maximum length of commands	

#define COMMAND_CAT		"cat"	
#define COMMAND_ECHO	"echo"  
#define COMMAND_FIND	"find"  

/* ############################################################################
 * Prototypes
 * ############################################################################
 */

int validate_command(char *input);

/* 
 * Test whether the input command are valid.
 * if so, return the number of command.
 * if not, return 0 after printing an error message. 
 */

int validate_command(char *input)
{
//	char *command = strtok(input, SPACE);

//	printf("Command : %s\n", command);

	return FALSE;
}

int main (void) 
{

	char	command[MAXLINE];
	pid_t	pid;
	int		status;
	int 	i;

	/* Print promt */

	fprintf (stdout, "%% ");

  	/* Read commands from standard input */

	while (fgets (command, MAXLINE, stdin) != NULL) {

		if (command[strlen(command) - 1] == LINE_FEED)
			command[strlen(command) - 1] = CHAR_NUL;

		if ((pid = fork()) < 0) {
			
			printf("error: fork error...");

		} else if (pid == 0) {

//			printf("number of command: %d\n", validate_command(command));

			execlp(command, command, (char *)0);
			printf("error: couldn't execute: %s", command);
			exit(127);

		} else {
			
			if ((pid = waitpid(pid, &status, 0)) < 0)
				printf("error: waitpid error...");
		}

		
	}	

  	fprintf (stdout, "Bye!\n");
  	exit (0);
}
