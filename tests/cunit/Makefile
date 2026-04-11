# CUnit Test Suite for TweetNaCl-Modular
# Author: Anupam Datta <adbd04@gmail.com>

CC ?= gcc
CFLAGS := -Wall -Wextra -g -O2 -I../../includes -I../../arch -I../../pqc
LDFLAGS := -lcunit -L../../.libs -ltweetnacl

# Source files
SRCS := test_runner.c \
        test_tweetnacl_core.c \
        test_tweetnacl_box.c \
        test_tweetnacl_sign.c \
        test_tweetnacl_secretbox.c \
        test_tweetnacl_pqc.c \
        test_secure_mem.c \
        test_arch_detection.c \
        test_constant_time.c

OBJS := $(SRCS:.c=.o)
TARGET := test_runner

# PQC object
PQC_OBJ := ../../pqc/pqc.o

all: $(TARGET)

$(TARGET): $(OBJS) $(PQC_OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(PQC_OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Build PQC object if not exists
$(PQC_OBJ): ../../pqc/pqc.c ../../pqc/pqc.h
	$(MAKE) -C ../../pqc

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

valgrind: $(TARGET)
	valgrind --tool=memcheck --leak-check=full --error-exitcode=1 ./$(TARGET)

.PHONY: all clean run valgrind
