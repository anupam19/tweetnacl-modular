# GTest Test Suite for TweetNaCl-Modular
# Author: Anupam Datta <adbd04@gmail.com>

CXX ?= g++
CXXFLAGS := -Wall -Wextra -g -O2 -I../../includes -I../../arch -I../../pqc -std=c++11
LDFLAGS := -lgtest -lgtest_main -lpthread -L../../.libs -ltweetnacl

# Source files
SRCS := test_tweetnacl.cpp
OBJS := $(SRCS:.cpp=.o)
TARGET := test_tweetnacl

# PQC object
PQC_OBJ := ../../pqc/pqc.o

all: $(TARGET)

$(TARGET): $(OBJS) $(PQC_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(PQC_OBJ) $(LDFLAGS)

%.o: %.cpp test_fixture.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build PQC object if not exists
$(PQC_OBJ): ../../pqc/pqc.c ../../pqc/pqc.h
	$(MAKE) -C ../../pqc

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET) --gtest_brief=1

valgrind: $(TARGET)
	valgrind --tool=memcheck --leak-check=full --error-exitcode=1 ./$(TARGET)

.PHONY: all clean run valgrind
