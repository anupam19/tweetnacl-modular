# File Name : Makefile
# Location : [@top root directory] tweetnacl-modular/
# Author : Anupam Datta
# Mail : adbd04@gmail.com
# Description : compiles tweetnacl library and produces shared library libtweetnacl.so inside [.libs] directory (tweetnacl-modular/.libs/)
# [-cont.] compile your project with '-ltweetnacl'

CC:=$(CC)

CFLAGS:=-Wextra -Wformat-security -Winline -Wmissing-declarations -Wmissing-prototypes -Wnested-externs -Wpointer-arith -Wshadow -Wstrict-prototypes -Wswitch-default -Wswitch-enum -Wunused -Wunused-result -Wlogical-op -g -O0 -DWITH_POSIX -fPIC

LDFLAGS:=-shared

SRC:=src/

INCLUDE:=-Iincludes

OBJ:=tweetnacl.o randombytes.o

SHARED:=libtweetnacl.so

MKDIR_P:=mkdir -p

.PHONY:	.libs

LIB:=.libs

$(LIB):
	$(MKDIR_P) $(LIB)

all:	$(LIB) $(OBJ) $(SHARED)

randombytes.o:	$(SRC)randombytes.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $<

tweetnacl.o:	$(SRC)tweetnacl.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $<

$(SHARED): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(SHARED) $(OBJ)
	@rm -f $(PROGRAMS) $(OBJ) *~
	@mv $(SHARED) $(LIB)

clean:
	@rm -rf $(LIB)
	@rm -f $(PROGRAMS) $(OBJ) *~ *.so
