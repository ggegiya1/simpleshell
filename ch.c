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

int execute_command(char *command, int input, int output)
{
	char *argv[MAX_ARGS];              //user command
    int  argc;
    char *token;
	int	status;
	pid_t	pid, wpid;
	
	/* tokenize and put to array argv[]
	 * argv[0] will contain the command name */
	token = strtok(command, " ");
	argc = 0;
	while(token != NULL){	
		argv[argc] = trim(token);
		token = strtok(NULL, " ");
		argc++;
	}
	argv[argc] = NULL;

	/* fork to execute the command */
	switch (pid = fork()){
		case -1:
			perror("Cannot fork");
			return EXIT_FAILURE;
		case 0:
			/*  this is a child code 
			 *	will execute command here
			*/
			
			// redirect stdin and stdout
			if (input != STDIN_FILENO){
				dup2(input, STDIN_FILENO);
				close(input);
			}
			
			if (output != STDOUT_FILENO){
				dup2(output, STDOUT_FILENO);
				close(output);
			}
			
			// execute the command and handle the errors  			
			if (execvp(argv[0],argv)<0){
				perror("Invalid command");
				return EXIT_FAILURE;
			}
	}
	// wait until the child process terminates	
	do {
		wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    
	return status;
}

int main (void) 
{

	char	buf[MAXLINE];
	ssize_t ret;
	char *argv[MAX_ARGS];
    int  argc;
    char *token;

	fprintf(stdout, PROMPT);
	fflush(stdout);
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

			int output = open(argv[1], O_WRONLY | O_TRUNC | O_CREAT,  S_IRUSR | S_IWUSR);
			if (output == -1){		
				perror("Cannot open the file for writing");
				continue;
			}
			execute_command(argv[0], STDIN_FILENO, output);
			
		}else if (strchr(buf, '<') != NULL){
  			// redirect input 
			token = strtok(buf, "<");
			argc = 0;
			
			while(token != NULL){	
				argv[argc] = trim(token);
				token = strtok(NULL, "<");
				argc++;
			}
			if (argc != 2){
				perror("Invalid command");
				continue;
			}

			int input = open(argv[1], O_RDONLY);
			if (input == -1){		
				perror("Cannot open the file for reading");
				continue;
			}
			execute_command(argv[0], input, STDOUT_FILENO);
			
  		}else{
  			// execute normal command
  			execute_command(buf, STDIN_FILENO, STDOUT_FILENO);
  		}
  
  			
		fprintf(stdout, PROMPT);
		fflush(stdout);
	}	
  	fprintf (stdout, "Bye!\n");
  	return 0;
}
