# Makefile for TweetNaCl Unit Tests

CC ?= gcc
CFLAGS ?= -Wall -Wextra -g -O2
LDFLAGS ?= 

INCLUDES := -I../includes -I../arch -I../pqc
LIBS := -L../.libs -ltweetnacl

TEST_TARGET := test_all
TEST_SRC := test_all.c
PQC_SRC := ../pqc/pqc.c
TEST_OBJ := test_all.o
PQC_OBJ := pqc.o

all: $(TEST_TARGET)

$(TEST_OBJ): $(TEST_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(PQC_OBJ): $(PQC_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(TEST_TARGET): $(TEST_OBJ) $(PQC_OBJ)
	$(CC) $(CFLAGS) -o $@ $(TEST_OBJ) $(PQC_OBJ) $(LIBS)

clean:
	rm -f $(TEST_TARGET) $(TEST_OBJ) $(PQC_OBJ) *.o

run: $(TEST_TARGET)
	./$(TEST_TARGET)

.PHONY: all clean run
