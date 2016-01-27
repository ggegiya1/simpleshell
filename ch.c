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

#define MAXLINE			4096		// The maximum length of a command
#define MAXDIRSIZE		4096		// Maximum of the files in the directory
#define CURRENT_DIR		"."
#define PROMPT 			"%%"

/* structure to handle one command */ 
struct Command{
	char * command;
	int input;
	int output;
	int status;
};

/* trim extra spaces at the beginning and at the end of command */
char * trim(char * s) 
{
    int l = strlen(s);

    while(isspace(s[l - 1])) --l;
    while(* s && isspace(* s)) ++s, --l;

    return strndup(s, l);
}

void read_directory(char * path, char ** files){
	
	DIR *dpath = NULL;
	struct dirent *direntry = NULL;
	
	if ((dpath = opendir(path))==NULL){
		perror("Failed to open the directory");
	}
	int i;
	for(i=0;(direntry = readdir(dpath)) != NULL;i++){
		files[i] = direntry->d_name;
	}
	files[i] = NULL;
}

/* spawn one command */
int execute_command(struct Command * cmd)
{
	char	**argv;
    	int	argc;
    	char	*token;
	int	status;
	pid_t	pid;
	char    *files[MAXDIRSIZE];

	/* tokenize and put to array argv[]
	 * argv[0] will contain the command name */
	argv = (char**) malloc(sizeof(char*));
	token = strtok(cmd->command, " ");
	argc = 0;
	while(token != NULL){
		argv[argc] = trim(token);
		/* replace the "*' by the list of the files in the current directory */
		if (strcmp("*", argv[argc]) == 0){
			read_directory(CURRENT_DIR, files);
			int i;
			for(i=0;files[i]!=NULL;i++){
				argv[argc] = files[i];
				argc++;
				argv = realloc(argv, sizeof(char*)*(argc+1));
			}
		}
		
		token = strtok(NULL, " ");
		argc++;
		argv = (char**)realloc(argv, sizeof(char*)*(argc+1));
	}
	argv[argc] = NULL;
	/* fork the porcess to execute the command */
	switch (pid = fork()){
		case -1:
			perror("Cannot fork");
			return EXIT_FAILURE;
		case 0:
			/*  this is a child code */ 
	
			/* redirect stdin and stdout */
			if (cmd->input != STDIN_FILENO){
				dup2(cmd->input, STDIN_FILENO);
				close(cmd->input);
			}
			
			if (cmd->output != STDOUT_FILENO){
				dup2(cmd->output, STDOUT_FILENO);
				close(cmd->output);
			}
			
			/* execute the command and handle the errors */  			
			if (execvp(argv[0],argv)<0){
				perror("Invalid command");
				return EXIT_FAILURE;
			}
	}
	
	/* wait until the child process terminates */	
	do {
		waitpid(pid, &status, WUNTRACED);
	} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	free(argv);
	return EXIT_SUCCESS;
}

struct Command * parse_command(char * command){
	
	char * token;
	int argc;
	char * argv[2];
	struct Command * cmd;

	/* don't forget to free the memory after the execution */
    if((cmd = malloc(sizeof(struct Command))) == NULL){
		perror("Memory allocation error");
		return cmd;
	}
    
    cmd->command = command;
    cmd->input = STDIN_FILENO;
    cmd->output = STDOUT_FILENO;
    cmd->output = EXIT_FAILURE;
    
	if (strchr(cmd->command, '>') != NULL){
  		/* redirect output to file */
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
  		/* read the input from file */ 
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
	struct  Command * cmd[128];
	int     count;
	
	int _pipe[2];
	int input;
	int status;
	
	fprintf(stdout, PROMPT);
	fflush(stdout);
  	/* Read commands from standard input */
	while ((ret = read(STDIN_FILENO, buf, MAXLINE)) > 0) {
		status = EXIT_SUCCESS;
		
		/* terminate the string by 0*/
		if (buf[ret - 1] == LINE_FEED)
			buf[ret - 1] = CHAR_NUL;

		/* allow using pipes */
		if (strchr(buf, '|') != NULL){

			/* parse commamnds and store in cmd[] array */
			char * token = strtok(buf, "|");
			input = STDIN_FILENO;
			count = 0;
			while(token != NULL){
				struct Command * one_command = parse_command (token);
				if (one_command==NULL || one_command->status==EXIT_FAILURE){
					perror("Unable to parse the command");
					status = EXIT_FAILURE;
					break;
				}
				cmd[count++] = one_command;
				token = strtok(NULL, "|");
			}
			
			if (status == EXIT_SUCCESS){
				int i;
				/* execute commands secuentially */
				for (i=0; i<count-1; i++){
					
					/* create pipe */
					pipe(_pipe);
					
					/* connect input and output to the pipe */  
					cmd[i]->input = input;
					cmd[i]->output = _pipe[1];

					execute_command (cmd[i]);
					/* close output , but keep input for the next command */
					close(_pipe[1]);
					input = _pipe[0];
					free(cmd[i]);
				}
				/* last command reads from pipe but writes to STDOUT */
				cmd[i]->input = input;
				execute_command (cmd[i]);
				close(_pipe[0]);
				free(cmd[i]);
			}
		}else{
			/* execute one command */
			struct Command * one_command = parse_command(buf);
			if (one_command == NULL || one_command->status == EXIT_FAILURE){
				perror("Unable to parse the command");
				continue;
			}
			execute_command(one_command);
			free(one_command);
		}
		
		fprintf(stdout, PROMPT);
		fflush(stdout);
			
	}	
	fflush(stdout);
  	fprintf (stdout, "Bye!\n");
  	return 0;
}
