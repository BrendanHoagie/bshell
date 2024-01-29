run : build
	./bshell

build :
	gcc bshell.c tokenizer.c -o bshell

.PHONY : clean

clean :
	\rm bshell