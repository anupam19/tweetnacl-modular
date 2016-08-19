CFLAGS:=-Wextra -Wformat-security -Winline -Wmissing-declarations -Wmissing-prototypes -Wnested-externs -Wpointer-arith -Wshadow -Wstrict-prototypes -Wswitch-default -Wswitch-enum -Wunused -Wunused-result -Wlogical-op -g -O0 -DWITH_POSIX -fPIC

INCLUDE:=-I../includes

OBJ:=	main.o

LIB:=../.libs

LDFLAGS:=-L$(LIB) -ltweetnacl -Wl,-rpath=$(LIB)

PROGRAMS:=app

all:	$(OBJ) $(PROGRAMS)

main.o:	main.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $< $(LDFLAGS)

app:	$(OBJ)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^ $(LDFLAGS)
	@rm -f $(OBJ) *~

clean:
	@rm -f $(PROGRAMS) $(OBJ) *~
