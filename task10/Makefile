Start: table_exe
	./table echo -e "\x1b[92;1mhello\x1b[0m"

table_exe: table.o
	cc -o table table.o

table.o:
	cc -c table.c

clean:
	rm table table.o
