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
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>


#define LINE_FEED		10		// Line feed
#define CHAR_NUL		0		// Null character

#define TRUE			1
#define FALSE   		0	

#define MAXLINE			1024		// The maximum length of one command
#define MAX_ARGS		1024	

#define PROMPT 			"%%"


/* ############################################################################
 * Prototypes
 * ############################################################################
 */

char * trim(char * s) 
{
    int l = strlen(s);

    while(isspace(s[l - 1])) --l;
    while(* s && isspace(* s)) ++s, --l;

    return strndup(s, l);
}

int execute_command(char *command)
{
	char *argv[MAX_ARGS];              //user command
    int   argc;
    char *token;
	int   status;
	pid_t	pid;

	/* get the first token */
	token = strtok(command, " ");
	argc = 0;
	while(token != NULL)
	{	
		argv[argc] = trim(token);
		token = strtok(NULL, " ");
		argc++;
	}
	argv[argc] = NULL;

	/* fork to execute the command */
	switch (pid=fork()){
		case -1:
			perror("Cannot fork");
			return EXIT_FAILURE;
		case 0:		
			/*  this is a child code 
			 *	will execute command here
			 */ 			
			execvp(argv[0],argv);
		}
	waitpid(pid, &status, 0);		
	return status;
}

int main (void) 
{

	char	buf[MAXLINE];
	ssize_t ret;
	int out_fd;
	char *argv[MAX_ARGS];
    int   argc;
    char *token;
    int saved_stdout;

	fprintf(stdout, PROMPT);
	
  	/* Read commands from standard input */
	while ((ret = read(STDIN_FILENO, buf, MAXLINE)) > 0) {
		
		/* terminate the string by 0*/
		if (buf[ret - 1] == LINE_FEED)
			buf[ret - 1] = CHAR_NUL;

  		if (strchr(buf, '>') != NULL){
  			// redirect output 
			token = strtok(buf, ">");
			argc = 0;
			while(token != NULL){	
				argv[argc] = trim(token);
				token = strtok(NULL, ">");
				argc++;
			}
			if (argc != 2){
				perror("Invalid command");
				continue;
			}

			out_fd = open(argv[1], O_WRONLY | O_TRUNC | O_CREAT,  S_IRUSR | S_IWUSR);
			if (out_fd == -1){		
				perror("Cannot open the file for writing");
				continue;
			}
			saved_stdout = dup(STDOUT_FILENO);
			dup2(out_fd, STDOUT_FILENO);
			close(out_fd);
			execute_command(argv[0]);
			dup2(saved_stdout, STDOUT_FILENO);
			close(saved_stdout);

  		}else{
  			// execute normal command
  			execute_command(buf);
  		}
  
  			
		fprintf(stdout, PROMPT);
		fflush(stdout);
	}	
  	fprintf (stdout, "Bye!\n");
  	return 0;
}
