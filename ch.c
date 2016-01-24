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
 
struct Command{
	char * command;
	int input;
	int output;
	int status;
};

char * trim(char * s) 
{
    int l = strlen(s);

    while(isspace(s[l - 1])) --l;
    while(* s && isspace(* s)) ++s, --l;

    return strndup(s, l);
}

int execute_command(struct Command * cmd)
{
	char *argv[MAX_ARGS];
    int  argc;
    char *token;
	int	status;
	pid_t pid;
	
	/* tokenize and put to array argv[]
	 * argv[0] will contain the command name */
	token = strtok(cmd->command, " ");
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
			if (cmd->input != STDIN_FILENO){
				dup2(cmd->input, STDIN_FILENO);
				close(cmd->input);
			}
			
			if (cmd->output != STDOUT_FILENO){
				dup2(cmd->output, STDOUT_FILENO);
				close(cmd->output);
			}
			
			// execute the command and handle the errors  			
			if (execvp(argv[0],argv)<0){
				perror("Invalid command");
				return EXIT_FAILURE;
			}
	}
	// wait until the child process terminates	
	do {
		waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    
	return status;
}

struct Command * parse_command(char * command){
	
	char * token;
	int argc;
	char * argv[2];
	struct Command * cmd;
    
    if((cmd = malloc(sizeof(struct Command))) == NULL){
		perror("Memory allocation error");
		return cmd;
	}
    
    cmd->command = command;
    cmd->input = STDIN_FILENO;
    cmd->output = STDOUT_FILENO;
    cmd->output = EXIT_FAILURE;
    
	if (strchr(cmd->command, '>') != NULL){
  		// redirect output
		token = strtok(cmd->command, ">");
		argc = 0;
			
		while(token != NULL){
			if (argc > 1){
				perror("Invalid syntax");
				return cmd;
			}	
			argv[argc] = trim(token);
			token = strtok(NULL, ">");
			argc++;
		}
		
		cmd->command = argv[0];
		cmd->output = open(argv[1], O_WRONLY | O_TRUNC | O_CREAT,  S_IRUSR | S_IWUSR);
		if (cmd->output == -1){
			perror("Cannot open the file for writing");
			return cmd;
		}
	}
						
	if (strchr(cmd->command, '<') != NULL){
  		// redirect input 
		token = strtok(cmd->command, "<");
		argc = 0;
			
		while(token != NULL){
			if (argc > 1){
				perror("Invalid syntax");
				return cmd;
			}	
			argv[argc] = trim(token);
			token = strtok(NULL, "<");
			argc++;
		}
		
		cmd->command = argv[0];
		cmd->input = open(argv[1], O_RDONLY);
		if (cmd->input == -1){		
			perror("Cannot open the file for reading");
			return cmd;
		}			
  	}
  	cmd->status = EXIT_SUCCESS;
  	return cmd;
}

int main (void) 
{

	char	buf[MAXLINE];
	ssize_t ret;
	struct Command *cmd;
	
	
	fprintf(stdout, PROMPT);
	fflush(stdout);
  	/* Read commands from standard input */
	while ((ret = read(STDIN_FILENO, buf, MAXLINE)) > 0) {
		
		/* terminate the string by 0*/
		if (buf[ret - 1] == LINE_FEED)
			buf[ret - 1] = CHAR_NUL;

		// execute normal command
		cmd = parse_command(buf);
		if (cmd==NULL){
			perror("Unable to parse the command");
			continue;
		}
		execute_command(cmd);
		fprintf(stdout, PROMPT);
		fflush(stdout);
			
	}	
  	fprintf (stdout, "Bye!\n");
  	return 0;
}
