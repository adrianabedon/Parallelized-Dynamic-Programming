# Compiles htable testing and dp-solver given a DP target (i.e. knapsack)

CXX = g++
CXXFLAGS = -g -std=c++11 -lpthread -pedantic -Wall -Wextra -Werror -Wshadow -Wconversion -Wunreachable-code
OBJS = solver.o htable.o
TARGETS = knapsack.cpp bellman.cpp 

.PHONY: clean test_htable.cpp $(TARGETS)

# To use: % make all
# Compiles all target files listed
# 	--> Creates executables for all target files
all: $(TARGETS)

# To use: % make <dp-name>.cpp
# Example: % make knapsack.cpp
# Compiles <dp-name>.cpp with the dp-solver and hash-table 
# 	--> Creates <dp-name>.out executable
# If it doesn't work, double check <dp-name>.cpp is included in the TARGETS list above
$(TARGETS) : %.cpp : $(OBJS)
		$(CXX) $(CXXFLAGS) -o $*.out $@ $(OBJS)

# To use: % make test_htable.cpp
# Compiles hash table tests
test_htable.cpp : %.cpp : htable.cpp htable.o
		$(CXX) $(CXXFLAGS) -o $*.out test_htable.cpp htable.o

# Compiles all object files (internally used)
%.o : %.cpp %.h
		$(CXX) $(CXXFLAGS) -o $@ -c $<

# To use: make clean
# Cleans all object files and executables
clean:
		-rm -f *.o *.out
