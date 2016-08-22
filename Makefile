# File Name : Makefile
# Location : [@top root directory] tweetnacl-modular/
# Author : Anupam Datta
# Mail : adbd04@gmail.com
# Description : compiles tweetnacl library and produces shared library libtweetnacl.so inside [.libs] directory (tweetnacl-modular/.libs/)
# [-cont.] compile your project with '-ltweetnacl'

CC:=$(CC)

CFLAGS:=-Wextra -Wformat-security -Winline -Wmissing-declarations -Wmissing-prototypes -Wnested-externs -Wpointer-arith -Wshadow -Wstrict-prototypes -Wswitch-default -Wswitch-enum -Wunused -Wunused-result -Wlogical-op -g -O0 -DWITH_POSIX -fPIC

LDFLAGS:=-shared

.PHONY:	.libs app install uninstall

PREFIX:=/usr/local
SRC:=src
INCLUDE:=includes
APP:=app
OBJ:=tweetnacl.o randombytes.o
SHARED:=libtweetnacl.so

MKDIR:=mkdir -p
LIB:=.libs

all: $(LIB) $(OBJ) $(SHARED) $(APP)

randombytes.o:	$(SRC)/randombytes.c
	$(CC) -c $(CFLAGS) -I$(INCLUDE) $<

tweetnacl.o:	$(SRC)/tweetnacl.c
	$(CC) -c $(CFLAGS) -I$(INCLUDE) $<

$(LIB):
	$(MKDIR) $(LIB)

$(SHARED): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(SHARED) $(OBJ)
	@rm -f $(PROGRAMS) $(OBJ) *~
	@mv $(SHARED) $(LIB)

$(APP):
	$(MAKE) -C $(APP)

clean:
	@rm -rf $(LIB)
	@rm -f $(PROGRAMS) $(OBJ) *~ *.so
	$(MAKE) -C $(APP) clean

install:
	cp $(LIB)/$(SHARED) $(DESTDIR)$(PREFIX)/lib/$(SHARED)
	$(MKDIR) $(DESTDIR)$(PREFIX)/include/tweetnacl
	cp -a includes/* $(DESTDIR)$(PREFIX)/include/tweetnacl/

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/lib/$(SHARED)
	rm -rf $(DESTDIR)$(PREFIX)/include/tweetnacl/
