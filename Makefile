CC=gcc

shell: shell.c
	$(CC) -o shell shell.c

clear:
	rm shell
