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
#include <dirent.h>

#define LINE_FEED		10		// Line feed
#define CHAR_NUL		0		// Null character

#define MAXLINE			4096	// The maximum length of a command
#define MAXARGS			4096	// Maximal number of arguments
#define MAXCMDS			128		// Maximal number of commands in the line

#define CURRENT_DIR		"."
#define PROMPT 			"%%"

/* Structure to handle one command */ 

struct Command {

	char * command;
	int input;
	int output;
	int status;
};

/* Trim extra spaces at the beginning and at the end of command */

char * trim(char * s) {

    int l = strlen(s);

    while(isspace(s[l - 1])) --l;
    while(* s && isspace(* s)) ++s, --l;

    return strndup(s, l);
}

void read_directory(char * path, char ** files) {	

	DIR *dpath = NULL;
	struct dirent *direntry = NULL;
	
	if ((dpath = opendir(path)) == NULL) {
		perror("Failed to open the directory");
	}

	int i;
	for(i=0; (direntry = readdir(dpath)) != NULL; i++) {
		files[i] = direntry->d_name;
	}

	files[i] = NULL;
}

/* Spawn one command */

int execute_command(struct Command * cmd) {

	char *argv[MAXARGS];
	char *files[MAXARGS];
    char *token;

    int	argc;
	int	status;

	pid_t	pid;

	/* Tokenize and put to array argv[]
	 * argv[0] will contain the command name 
	 */

	token = strtok(cmd->command, " ");
	argc  = 0;

	while(token != NULL) {

		char *argument = trim(token);
		
		/* Replace the "*' by the list of the files in the current directory */

		if (strcmp("*", argument) == 0) {

			read_directory(CURRENT_DIR, files);
			int i;

			for(i = 0; files[i] != NULL; i++) {

				argv[argc] = files[i];
				argc++;

				if (argc == MAXARGS){
					perror("Too many files");
					return EXIT_FAILURE;
				}
			}

		} else {

			argv[argc] = argument;
			argc++;

			if (argc == MAXARGS){
				perror("Too many arguments");
				return EXIT_FAILURE;
			}
		}

		token = strtok(NULL, " ");
	}

	/* NULL is the special element, end of the list */
	argv[argc] = NULL;

	/* Fork the porcess to execute the command */

	switch (pid = fork()) {

		case -1:
			perror("Cannot fork");
			return EXIT_FAILURE;
			 
		case 0:	
			/*  This is a child code */
	
			/* Redirect input to read from file */

			if (cmd->input != STDIN_FILENO){
				dup2(cmd->input, STDIN_FILENO);
				close(cmd->input);
			}
			
			/* Rredirect output to file */

			if (cmd->output != STDOUT_FILENO){
				dup2(cmd->output, STDOUT_FILENO);
				close(cmd->output);
			}

			/* Execute the command and handle the errors */

			if (execvp(argv[0],argv)<0){
				perror("Invalid command");
				return EXIT_FAILURE;
			}
	}
	
	do {

		/* Wait until the child process terminates */	
		waitpid(pid, &status, WUNTRACED);

	} while (!WIFEXITED(status) && !WIFSIGNALED(status));

	return EXIT_SUCCESS;
}

struct Command * parse_command(char * command) {
	
	char * token;
	char * argv[2];
	int argc;

	struct Command * cmd;

    if ((cmd = malloc(sizeof(struct Command))) == NULL) {
		perror("Memory allocation error");
		return cmd;
	}
    
	cmd->command = command;
	cmd->input = STDIN_FILENO;
	cmd->output = STDOUT_FILENO;
	cmd->output = EXIT_FAILURE;
    
	if (strchr(cmd->command, '>') != NULL) {
  		
		/* Redirect output to file */
		token = strtok(cmd->command, ">");

		argc  = 0;	
	
		while(token != NULL) {

			if (argc > 1) {
				perror("Invalid syntax");
				return cmd;
			}	

			argv[argc] = trim(token);
			token = strtok(NULL, ">");
			argc++;
		}
		
		cmd->command = argv[0];
		cmd->output  = 
			open(argv[1], O_WRONLY | O_TRUNC | O_CREAT,  S_IRUSR | S_IWUSR);

		if (cmd->output == -1) {
			perror("Cannot open the file for writing");
			return cmd;
		}
	}
						
	if (strchr(cmd->command, '<') != NULL) {
  		
		/* Read the input from file */ 
		token = strtok(cmd->command, "<");

		argc = 0;
			
		while(token != NULL) {

			if (argc > 1) {
				perror("Invalid syntax");
				return cmd;
			}	

			argv[argc] = trim(token);
			token = strtok(NULL, "<");
			argc++;
		}
		
		cmd->command = argv[0];
		cmd->input   = open(argv[1], O_RDONLY);

		if (cmd->input == -1) {		
			perror("Cannot open the file for reading");
			return cmd;
		}			
  	}

  	cmd->status = EXIT_SUCCESS;
  	return cmd;
}

int main (void) {

	char	buf[MAXLINE];
	ssize_t ret;
	struct  Command * cmd[MAXCMDS];
	int     count;
	
	int _pipe[2];
	int input;
	int status;
	
	fprintf(stdout, PROMPT);
	fflush(stdout);

  	/* Read commands from standard input */

	while ((ret = read(STDIN_FILENO, buf, MAXLINE)) > 0) {
		
		status  = EXIT_SUCCESS;
		
		/* Terminate the string by 0*/

		if (buf[ret - 1] == LINE_FEED)
			buf[ret - 1] = CHAR_NUL;

		/* Allow using pipes */

		if (strchr(buf, '|') != NULL){

			/* Parse commamnds and store in cmd[] array */

			char * token = strtok(buf, "|");
			input = STDIN_FILENO;
			count = 0;

			while(token != NULL) {

				struct Command * _command = parse_command (token);
				
				if (_command == NULL || _command->status == EXIT_FAILURE) {
					
					perror("Unable to parse the command");
					status = EXIT_FAILURE;
					break;
				}

				cmd[count++] = _command;
				token = strtok(NULL, "|");

				if (count == MAXCMDS) {
					perror("Too many commands");
					status = EXIT_FAILURE;
					break;
				}
			}
			
			/* Execute commands in chain if the command line was successfully parsed */

			if (status == EXIT_SUCCESS) {
				
				int i;

				/* Execute commands secuentially */

				for (i=0; i < count - 1; i++){
					
					/* Create pipe */
					pipe(_pipe);
					
					/* Connect input and output to the pipe */ 

					cmd[i]->input  = input;
					cmd[i]->output = _pipe[1];
					
					execute_command (cmd[i]);

					/* Close output , but keep input for the next command */

					close(_pipe[1]);
					input = _pipe[0];
					free(cmd[i]);
				}

				/* Last command reads from pipe but writes to STDOUT */

				cmd[i]->input = input;
				execute_command (cmd[i]);
				close(_pipe[0]);
				free(cmd[i]);
			}

		}else{

			/* Execute one command */
			struct Command * _command = parse_command(buf);

			if (_command == NULL || _command->status == EXIT_FAILURE){
				perror("Unable to parse the command");
				continue;
			}

			execute_command(_command);
			free(_command);
		}
		
		fprintf(stdout, PROMPT);
		fflush(stdout);
			
	}	

	fflush(stdout);
  	fprintf (stdout, "Bye!\n");

  	return 0;
}
