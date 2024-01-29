/************************************************
 * bshell.c - my implementation of a simple shell
 * Brendan Hoag
 * 8/27/2023
 * 
 * on GNU/Linux 5.15.0-67-generic x86_64
 ************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "tokenizer.h"

#define BUFF 256

char* getInput(char* prompt, int* charsRead);
void runCommand(char** args, int parentWait, int inputfd, int outputfd, int* pip);
void parse(Token* cmd, int numTokens, int numPipes);

int main(int argc, char* argv[])
{
	Token *list, *cmd;
	char cwd[BUFF], *prompt, *buffer;
	int i, pid, numTokens, numPipes, numChars;
	
	// wrong usage
	if(argc != 1){
		fprintf(stderr, "Usage: ./bshell\n");
		exit(1);
	}

	// forever loop of shell
	while(1){

		// building prompt
		prompt = (char*) malloc(sizeof(char) * BUFF);
		strncpy(prompt, "Bshell", 6);
		if(getcwd(cwd, sizeof(cwd)) == NULL){
			perror("getcwd() failed");
			exit(3);
		}
		strcat(prompt, ":");
		strcat(prompt, cwd);
		strcat(prompt, "#");

		// get input & check for EOF
		buffer = getInput(prompt, &numChars);
		if(numChars == -1){
			printf("\b\b\n");
			break ;
		}
		
		// tokenize input
		list = tokenize(buffer, &numTokens, &numPipes);
		cmd = list;

		// if the only token is ; (only newline input), skip
		if(numTokens != 1)
			// otherwise parse input & run commands
			parse(cmd, numTokens, numPipes);

		// free used memory & reset prompt
		freeTokens(list);
		free(buffer);
		free(prompt);
		prompt = NULL;
	}
	return 0;
}

/*
* Function: parse
* ------------------
* acts as a manager for runCommand, handling
* redirects, pipes, and firing requests to runCommand
* 
* cmd: a list of tokens
* numTokens: the number of tokens in cmd
* numPipes: the number of pipes in cmd
*
* returns: none
*/
void parse(Token* cmd, int numTokens, int numPipes)
{
	char **args;
	int i, j, pipesSeen, **pipeArr, infd, outfd, reset;

	// initalize array of pipes & free + reset on errors
	pipesSeen = 0;
	pipeArr = (int**) malloc(numPipes * sizeof(int*));
	for(i = 0; i < numPipes; i++)
		pipeArr[i] = (int*) malloc(2 * sizeof(int));

	// initalize I/O fds
	infd = STDIN_FILENO;
	outfd = STDOUT_FILENO;

	// build args, waiting for &, ;, |, <, >, or >> tokens to proceed
	args = (char**) malloc(numTokens * (sizeof(char*) + 1));
	for(i = 0; i < numTokens; i++){
		if(strncmp(cmd->data, ">", 1) == 0){ // redirect output (overwrite) flag = 1
			outfd = creat(cmd->next->data, 00700);
			if(outfd < 0){
				fprintf(stderr, "error redirecting to %s\n", cmd->next->data);
				break ;
			}
			cmd = cmd->next->next;
			i++;
		}
		else if(strncmp(cmd->data, ">>", 2) == 0){ // redirect output (append) flag = 0
			outfd = open(cmd->next->data, O_WRONLY | O_CREAT | O_APPEND, 00700);
			if(outfd < 0){
				fprintf(stderr, "error redirecting to %s\n", cmd->next->data);
				break ;
			}
			cmd = cmd->next->next;
			i++;
		}
		else if(strncmp(cmd->data, "<", 1) == 0){ // redirect input
			infd = open(cmd->next->data, O_RDONLY);
			if(infd < 0){
				fprintf(stderr, "error redirecting from %s\n", cmd->next->data);
				break ;
			}
			cmd = cmd->next->next;
			i++;
		}
		else if(strncmp(cmd->data, "|", 1) == 0){ // pipe & run command
			args[i] = NULL;
			if(pipe(pipeArr[pipesSeen]) < 0){
				fprintf(stderr, "error creating pipe\n");
				break ;
			}
			runCommand(args, 1, infd, outfd, pipeArr[pipesSeen]);
			numTokens = numTokens - i - 1;
			i = -1;
			cmd = cmd->next;
		}
		else if(strncmp(cmd->data, "&", 1) == 0){ // fire off command and continue processing tokens
			args[i] = NULL;
			runCommand(args, 0, infd, outfd, NULL);
			numTokens = numTokens - i - 1;
			i = -1;
			cmd = cmd->next;
		}
		else if(strncmp(cmd->data, ";", 1) == 0){ // fire off command and continue processing tokens after waiting
			args[i] = NULL;
			runCommand(args, 1, infd, outfd, NULL);
			numTokens = numTokens - i - 1;
			i = -1;
			cmd = cmd->next;
		}
		else { // add to args vector
			args[i] = cmd->data;
			cmd = cmd->next;
		}
	}
	// process will always end w/ ; so wait to not offset prompt in cases w/ multiple &'s,
	wait(NULL);
	free(args);
	for(i = 0; i < numPipes; i++)
		free(pipeArr[i]);
	free(pipeArr);
	reset = open("/dev/tty", O_RDONLY); // linux file for the terminal, ensures stdin was not incorrectly closed
	dup2(reset, STDIN_FILENO);
	close(reset);
}

/*
* Function: runCommand
* ------------------
* given an argument vector and a wait signal,
* runs a command with command line arguments
* 
* args: a null-terminated string vector containing
*		the command line arguments 
* parentWait: an int representing if the command is
* inputfd: an int representing the input file descriptor to redirect stdin to
* outputfd: an int representing the output file descriptor to redirect stdout to
* pip: an int array of size 2 representing a pipe's read & write fd's
*
* returns: none
*/
void runCommand(char** args, int parentWait, int inputfd, int outputfd, int* pip)
{
	int pid;
	
	// cd builtin
	if(strncmp(args[0], "cd", 2) == 0){
		if(chdir(args[1]) != 0){
			fprintf(stderr, "cd: bad syntax\n");
		}
		return ; 
	}
	
	// exit builtin
	if(strncmp(args[0], "exit", 4) == 0){
		exit(0);
	}

	// if not builtin, fork & exec
	pid = fork();

	// error
	if(pid == -1){ 
		perror("fork");
		exit(5);
	}

	// child
	if(pid == 0) {
		// close active fds from pipe so it doesn't hang
		if(pip != NULL){
			dup2(pip[1], STDOUT_FILENO);
			close(pip[1]);
			close(pip[0]);
		}
		
		// don't overwrite just stdin or just stdout
		if(inputfd != STDIN_FILENO){
			dup2(inputfd, STDIN_FILENO);
			close(inputfd);
		}
		if(outputfd != STDOUT_FILENO){
			dup2(outputfd, STDOUT_FILENO);
			close(outputfd);
		}

		// execute arguments and check for errors
		if(execvp(args[0], args) == -1){
			printf("no such program %s\n", args[0]);
		}
		exit(0);
	}

	// parent
	else{
		// close pipe ends and dup its read 
		// side to stdin for continued pipeline if it exists
		if(pip != NULL){
			close(pip[1]);
			dup2(pip[0], STDIN_FILENO);
			close(pip[0]);
		}
		if(parentWait) { // waiting (;)
			wait(NULL);
		}
		// otherwise not waiting (&)
	}
}

/*
* Function: getInput
* ------------------
* given a prompt to display, takes user input
* 
* prompt: a string representing the terminal prompt 
*		  to be displayed
* charsRead: an int pointer to store the number of characters read
*
* returns: a string containing the user's input
*
* side-effects: stores the number of characters read in charsRead
*/
char* getInput(char* prompt, int* charsRead)
{
	char *buffer = NULL;
	size_t buffSize;

	printf("%s ", prompt);
	*charsRead = getline(&buffer, &buffSize, stdin);
	return buffer;
}