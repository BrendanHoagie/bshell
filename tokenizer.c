/********************************************************
 * tokenizer.c - a program to tokenize a string of inputs
 * Brendan Hoag
 * 8/27/2023
 * 
 * on GNU/Linux 5.15.0-67-generic x86_64
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tokenizer.h"

/*
* Function: newToken
* -------------------
* generates an empty Token
*
* returns: a pointer to a Token
*/
Token* newToken()
{
	Token *tok;

	tok = (Token*) malloc(sizeof(Token));
	tok->data = NULL;
	tok->next = NULL;
	return tok;
}

/*
* Function: addToken
* -------------------
* given a list of Tokens and a string, creates
* a new token and adds it to the end of the list
* 
* list: a Token pointer representing the list of Tokens
* str: a string representing the data field of the new Token
*
* returns: none
*/
void addToken(Token* list, char* str)
{
	Token *tok, *cur;

	tok = newToken();
	tok->data = (char*) malloc(sizeof(char) * strlen(str) + 1);
	strcpy(tok->data, str);
	cur = list;
	while(cur->next != NULL){
		cur = cur->next;
	}
	cur->next = tok;
}

/*
* Function: tokenize
* -------------------
* seperates a string into individual tokens
* 
* str: a string representing the user input to be tokenized
* numTokens: an int pointer representing the number of tokens in str
* numPipes: an int pointer representing the number of pipes found in str
*
* returns: a list of Token pointers representing the tokenized string,
*		   always ending with a ; character
*
* side-effects: updates numTokens with number of tokens in str
*/
Token* tokenize(char* str, int* numTokens, int* numPipes)
{
	Token *list, *ret;
	char *word, delim[3] = " \n\t";
	int i, lastWasSpecial, pipes;

	list = newToken();
	word = strtok(str, delim);
	i = 0;
	lastWasSpecial = 0;
	pipes = 0;
	while(word != NULL){
		// handle error checking for ; and & cases 
		// ; and & as their own tokens
		// ; and & as the last character of a token
		// not allow multiple ; or & in a row
		if(strncmp(word, "&", 1) == 0 && !lastWasSpecial){
			addToken(list, word);
			lastWasSpecial = 1;
		}
		else if(word[strlen(word) - 1] == '&' && !lastWasSpecial){
			word[strlen(word) - 1] = '\0';
			addToken(list, word);
			addToken(list, "&");
			i++;
			lastWasSpecial = 1;
		}
		else if(strncmp(word, ";", 1) == 0 && !lastWasSpecial){
			addToken(list, word);
			lastWasSpecial = 1;
		}
		else if(word[strlen(word) - 1] == ';' && !lastWasSpecial){
			word[strlen(word) - 1] = '\0';
			addToken(list, word);
			addToken(list, ";");
			i++;
			lastWasSpecial = 1;
		}
		// determine > vs >>
		else if(strncmp(word, ">", 1) == 0 && str[0] == '>'){
			addToken(list, ">>");
			str++;
			lastWasSpecial = 0;
		}
		// increment pipe counter
		else if(strncmp(word, "|", 1) == 0){
			addToken(list, "|");
			pipes++;
		}
		// normal token
		else if(strstr(word, "&") == NULL && strstr(word, ";") == NULL){
			addToken(list, word);
			lastWasSpecial = 0;
		}
		// otherwise invalid combination (such as ; ;), skip this token by decrementing i
		else
			i--;
		word = strtok(NULL, delim);
		i++;
	}
	if(!lastWasSpecial){
		addToken(list, ";");
		i++;
	}
	ret = list->next;
	free(list);
	*numTokens = i;
	*numPipes = pipes;
	return ret;
}

/*
* Function: freeTokens
* ---------------------
* frees a list of Tokens to avoid memory leaks
* 
* list: a Token pointer representing the list of Tokens
*
* returns: none
*/
void freeTokens(Token* list)
{
	Token *tmp;

	while(list != NULL){
		tmp = list->next;
		free(list);
		list = tmp;
	}
}

/*
* Function: printTokens
* ---------------------
* prints a list of Tokens for debugging purposes
* 
* list: a Token pointer representing the list of Tokens
*
* returns: none
*/
void printTokens(Token* list)
{
	int i;

	i = 0;
	printf("List is:\n");
	while(list != NULL){
		printf("%d: %s\n", i, list->data);
		list = list->next;
		i++;
	}
}