# Google Test Makefile for TweetNaCl-Modular

CXX ?= g++
CC ?= gcc
CXXFLAGS ?= -Wall -Wextra -g -O2 -std=c++11
GTEST_CFLAGS := $(shell pkg-config --cflags gtest 2>/dev/null || echo "-I/usr/include")
GTEST_LDFLAGS := $(shell pkg-config --libs gtest gtest_main 2>/dev/null || echo "-lgtest -lgtest_main -lpthread")

SRCDIR := ../../src
INCDIR := ../../includes
ARCHDIR := ../../arch
PQCDIR := ../../pqc

# Source files
TEST_SRCS := test_tweetnacl.cpp test_pqc.cpp test_secure_mem.cpp
LIB_SRCS := $(SRCDIR)/tweetnacl.c $(SRCDIR)/randombytes.c $(SRCDIR)/secure_mem.c $(SRCDIR)/secure_utils.c $(PQCDIR)/pqc.c

# Object files
TEST_OBJS := $(TEST_SRCS:.cpp=.o)
LIB_OBJS := tweetnacl.o randombytes.o secure_mem.o secure_utils.o pqc.o

# Target executable
TARGET := test_tweetnacl

all: $(TARGET)

$(TARGET): $(TEST_OBJS) $(LIB_OBJS)
$(CXX) $(CXXFLAGS) -o $@ $(TEST_OBJS) $(LIB_OBJS) $(GTEST_LDFLAGS) -lm

%.o: %.cpp
$(CXX) $(CXXFLAGS) -I$(INCDIR) -I$(ARCHDIR) $(GTEST_CFLAGS) -c $< -o $@

tweetnacl.o: $(SRCDIR)/tweetnacl.c
$(CC) $(CXXFLAGS) -I$(INCDIR) -I$(ARCHDIR) -I$(PQCDIR) -c $< -o $@

randombytes.o: $(SRCDIR)/randombytes.c
$(CC) $(CXXFLAGS) -I$(INCDIR) -I$(ARCHDIR) -I$(PQCDIR) -c $< -o $@

secure_mem.o: $(SRCDIR)/secure_mem.c
$(CC) $(CXXFLAGS) -I$(INCDIR) -I$(ARCHDIR) -I$(PQCDIR) -c $< -o $@

secure_utils.o: $(SRCDIR)/secure_utils.c
$(CC) $(CXXFLAGS) -I$(INCDIR) -I$(ARCHDIR) -I$(PQCDIR) -c $< -o $@

pqc.o: $(PQCDIR)/pqc.c
$(CC) $(CXXFLAGS) -I$(INCDIR) -I$(ARCHDIR) -I$(PQCDIR) -c $< -o $@

clean:
rm -f $(TARGET) *.o

.PHONY: all clean
