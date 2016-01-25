/******************************************************************************
 * SIMPLESHELL							                                      *
 *                                                                            *
 ******************************************************************************
 * Executes the commands:                                                     *
 *         cat Makefile                                                       *
 *         echo *                                                             *
 *         cat <Makefile > foo                                                *
 *         find -name Makefile | xargs grep ch                                *
 *                                                                            *
 ******************************************************************************
 * Authors:                                          						  *
 *         Cordeleanu Corneliu < cordeleanu@gmail.com >                       *
 *         Georgiy Gegiya      < gegiya@gmail.com >                           *
 *                                                                            *
 ******************************************************************************
 * Compiler: gcc version 4.8.4 (Ubuntu 4.8.4-2ubuntu1~14.04)                  *
 * IFT2245 05 - 28 January 2016                                               *
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

#define MAXLINE			1024	// The maximum length of one command
#define MAX_ARGS		1024	

#define PROMPT 			"%% "

#define PIPE_STDIN		0
#define PIPE_STDOUT		1

/* ############################################################################
 * Prototypes
 * ############################################################################
 */

int main (void) {

	char	buf[MAXLINE];
	char 	*argv[MAX_ARGS], *argv_two[MAX_ARGS];
    char 	*token;

	ssize_t ret;
	pid_t	pid;

    int   	argc, status, in, out;
	int		the_pipe[2];

	int		is_input = 0, is_output = 0, is_pipe = 0, is_ast = 0;

	/* Print promt */	
	fprintf(stderr, PROMPT);

  	/* Read commands from standard input */
	while ((ret = read(STDIN_FILENO, buf, MAXLINE)) > 0) {
		
		/* Terminate the string by 0 */
		if (buf[ret - 1] == LINE_FEED)
			buf[ret - 1] = CHAR_NUL;

		/* Get the arguments */
		token = strtok(buf, " ");
		argc  = 0;

		while(token != NULL) {	

			/* If is a redirect from file */
			if (token[0] == '<' || is_input) {

				if (0 == is_input && 1 == strlen(token)) 
					is_input = 1;

				else if (0 == is_input && strlen(token) > 1) {
					in  = open(strndup(token, -1), "r");					
					is_input = 0;
				}

				else {
					in = open(token, "r");
					is_input = 0;
				}

				continue;
			}

			/* If is a redirect to file */
			else if (token[0] == '>' || is_output) {

				if (0 == is_output && 1 == strlen(token)) 
					is_output = 1;

				else if (0 == is_output && strlen(token) > 1) {
					out  = open(strndup(token, -1), "w");					
					is_output = 0;
				}

				else {
					out = open(token, "w");
					is_output = 0;
				}

				continue;
			}

			/* Asterisk character */
			else if (token[0] == '*') {
				argv[argc] = (token[1] != CHAR_NUL) ? strndup(token, -1) : " ";
				is_ast = 1;
			}
			
			else 
				argv[argc] = token;
			
			token = strtok(NULL, " ");
			argc++;
		}

		argv[argc] = NULL;		

		/* Fork to execute the commands */
  		if ((pid = fork()) < 0) {
  			
  			perror("fork");
			exit(EXIT_FAILURE);

		/* Tthis is a child code 
		 * will execute command here
		 */ 
  
  		} else if (pid == 0) {

			/* Command echo */ 
			if (strcmp(argv[0], "echo") == 0 && is_ast) {
				argv[0] = "ls";
				argv[1] = NULL;
			}

			/* Redirect */ 
			if (is_input && is_output) {
  				
				/* Replace standards inputs with input files */
				dup2(in,  0);
  				dup2(out, 1);

  				/* Close unused file descriptors */
  				close(in);
  				close(out);
			}

			execvp(argv[0], argv);
			exit(EXIT_SUCCESS);	

  		} else {

			waitpid(pid, &status, WEXITED);
			fprintf(stderr, PROMPT);
  		}  		
  	}

  	fprintf (stdout, "Bye!\n");
  	return 0;
}	
