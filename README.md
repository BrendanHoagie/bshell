# bshell
A simple shell made in C intended to be run at the command line in Linux

## Structure
This project is split into 2 main files: `bshell.c` and `tokenizer.c`: 

- In `bshell.c`, I've kept all of the main driver code. This includes taking input and dealing with the tokenized input from the user. All commands (`ls`, `cat`, etc) are implemented as calls to unistd.h, except for `cd`, which is a builtin.

- In `tokenizer.c`, I've put all of the code for tokenizing input. This includes seperating out tokens by spaces, allowing for properly handling pipes and redirections, as well as allowing for programs input on a single line to run concurrently or sequentially using the `&&` and `;` characters respectively.

## Execution
The provided makefile handles building and running bshell. It features three commands:

- `make build` builds the project as bshell
- `make run` builds and runs bshell
- `make clean` removes the executable
