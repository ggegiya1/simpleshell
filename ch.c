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
#include <sys/types.h>
#include <sys/wait.h>

#define LINE_FEED		10		// Line feed
#define CHAR_NUL		0		// Null character

#define TRUE			1
#define FALSE   		0	

<<<<<<< HEAD
#define MAXLINE			2048	// Length of command	
#define MAX_OPT			16		// Options of command
=======
#define MAXLINE			1024		// The maximum length of one command
#define MAX_ARGS		1024	

#define PROMPT 			"%%"
>>>>>>> f0e769d957f934a42fc97f2e2176ceea4f88be7e


/* ############################################################################
 * Prototypes
 * ############################################################################
 */


int main (void) 
{

<<<<<<< HEAD
	char	command[MAXLINE];
	char	*program;
	char	options[MAX_OPT];
	
	int		argc		= 0;
	int		status;

	pid_t	pid;

	/* Print promt */

	fprintf (stdout, "%% ");

  	/* Read commands from standard input */

	while (fgets (command, MAXLINE, stdin) != NULL) {

		if (command[strlen(command) - 1] == LINE_FEED) 
			command[strlen(command) - 1] = CHAR_NUL;

		if ((program = strtok(command, " ")) != NULL) {
			
			if (strcmp(program, COMMAND_CAT) == 0) {
				
				options[0] = strtok(command, " ");

			}

			if ((pid = fork()) < 0) {
			
				printf("error: fork error...");

			} else if (pid == 0) {

				execlp(program, options, (char *)0);
				printf("error: couldn't execute: %s", program);
				exit(127);

			} else {
			
				if ((pid = waitpid(pid, &status, 0)) < 0)
					printf("error: waitpid error...");
			}
		}
		
=======
	char	buf[MAXLINE];
	ssize_t ret;
	pid_t	pid;
	char *argv[MAX_ARGS];              //user command
    int   argc;
    char *token;
	int   status;
	//int 	i;
	
	fprintf(stderr, PROMPT);
	
  	/* Read commands from standard input */
	while ((ret = read(STDIN_FILENO, buf, MAXLINE)) > 0) {
		
		/* terminate the string by 0*/
		if (buf[ret - 1] == LINE_FEED)
			buf[ret - 1] = CHAR_NUL;
			
		/* fork to execute the command */
		switch (pid=fork()){
			
			case -1:
				perror("fork");
				exit(EXIT_FAILURE);
			
			case 0:		
				/*  this is a child code 
				 *	will execute command here
				 */ 
				
				/* get the first token */
				token = strtok(buf, " ");
				argc = 0;
				while(token != NULL)
				{	
					argv[argc] = token;
					token = strtok(NULL, " ");
					argc++;
				}
				argv[argc] = NULL;
				
				/* execute command */  
				execvp(argv[0],argv);
			    exit(EXIT_SUCCESS);
			    
			default:
				waitpid(pid, &status, WEXITED);
				fprintf(stderr, PROMPT);
			}
>>>>>>> f0e769d957f934a42fc97f2e2176ceea4f88be7e
	}	
  	fprintf (stdout, "Bye!\n");
  	return 0;
}
