/*************************************************************************
 * tokenizer.h - header file for working with Tokens and tokenizing input
 * Brendan Hoag
 * 8/27/2023
 * 
 * on GNU/Linux 5.15.0-67-generic x86_64
 *************************************************************************/

/*
* Struct: Token
* ------------------
* a node representing a token in an input
* 
* data: a string representing the tokenized input
* next: a pointer representing the next Token in the list
*/
typedef struct Token Token;
typedef struct Token{
	char* data;
	Token* next;
}Token;

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
Token* tokenize(char* str, int* numTokens, int* numPipes);

/*
* Function: freeTokens
* ---------------------
* frees a list of Tokens to avoid memory leaks
* 
* list: a Token pointer representing the list of Tokens
*
* returns: none
*/
void freeTokens(Token* list);

/*
* Function: printTokens
* ---------------------
* prints a list of Tokens for debugging purposes
* 
* list: a Token pointer representing the list of Tokens
*
* returns: none
*/
void printTokens(Token* list);