Start: table_exe
	./table table.c

table_exe: table.o
	cc -o table table.o

table.o:
	cc -c table.c

clean:
	rm table table.o
