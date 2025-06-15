ls: ls.c
	gcc -g ls.c -o ls

exec: ls
	./ls $(directory)

debug: ls
	gdb ls