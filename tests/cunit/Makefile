# CUnit Test Makefile for TweetNaCl-Modular

CC ?= gcc
CFLAGS ?= -Wall -Wextra -g -O2
CUNIT_CFLAGS := $(shell pkg-config --cflags libcunit 2>/dev/null || echo "-I/usr/include")
CUNIT_LDFLAGS := $(shell pkg-config --libs libcunit 2>/dev/null || echo "-lcunit")

SRCDIR := ../../src
INCDIR := ../../includes
ARCHDIR := ../../arch
PQCDIR := ../../pqc

# Source files
TEST_SRCS := test_runner.c test_tweetnacl_core.c test_secure_mem.c test_tweetnacl_box.c test_tweetnacl_sign.c test_tweetnacl_secretbox.c
LIB_SRCS := $(SRCDIR)/tweetnacl.c $(SRCDIR)/randombytes.c $(SRCDIR)/secure_mem.c $(SRCDIR)/secure_utils.c $(PQCDIR)/pqc.c

# Object files
TEST_OBJS := $(TEST_SRCS:.c=.o)
LIB_OBJS := tweetnacl.o randombytes.o secure_mem.o secure_utils.o pqc.o

# Target executable
TARGET := test_runner

all: $(TARGET)

$(TARGET): $(TEST_OBJS) $(LIB_OBJS)
$(CC) $(CFLAGS) -o $@ $(TEST_OBJS) $(LIB_OBJS) $(CUNIT_LDFLAGS) -lm

tweetnacl.o: $(SRCDIR)/tweetnacl.c
$(CC) $(CFLAGS) -I$(INCDIR) -I$(ARCHDIR) -I$(PQCDIR) -c $< -o $@

randombytes.o: $(SRCDIR)/randombytes.c
$(CC) $(CFLAGS) -I$(INCDIR) -I$(ARCHDIR) -I$(PQCDIR) -c $< -o $@

secure_mem.o: $(SRCDIR)/secure_mem.c
$(CC) $(CFLAGS) -I$(INCDIR) -I$(ARCHDIR) -I$(PQCDIR) -c $< -o $@

secure_utils.o: $(SRCDIR)/secure_utils.c
$(CC) $(CFLAGS) -I$(INCDIR) -I$(ARCHDIR) -I$(PQCDIR) -c $< -o $@

pqc.o: $(PQCDIR)/pqc.c
$(CC) $(CFLAGS) -I$(INCDIR) -I$(ARCHDIR) -I$(PQCDIR) -c $< -o $@

%.o: %.c
$(CC) $(CFLAGS) -I$(INCDIR) -I$(ARCHDIR) $(CUNIT_CFLAGS) -c $< -o $@

clean:
rm -f $(TARGET) *.o

.PHONY: all clean
