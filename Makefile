# Compiles htable testing and dp-solver given a DP target (i.e. knapsack)

CXX = g++
CXXFLAGS = -std=c++11 -lpthread -pedantic -Wall -Wextra -Werror -Wconversion -Wshadow -Wunreachable-code
OBJS = htable/htable.o
TARGETS = knapsack.cpp bellman.cpp
TEST_GENS = knapsack_test_gen.cpp bellman_test_gen.cpp
HTABLE = htable/htable

.PHONY: clean test_htable.cpp $(TARGETS) $(TEST_GENS)

# To use: % make all
# Compiles all target files listed
# 	--> Creates executables for all target files
all: $(TARGETS)

# Compiles <dp-name>.cpp with the hash-table 
# To use: % make <dp-name>.cpp
# Example: % make knapsack.cpp
# 	--> Creates <dp-name> executable
# If it doesn't work, double check <dp-name>.cpp is included in the TARGETS list above
$(TARGETS) : %.cpp : $(OBJS)
		$(CXX) $(CXXFLAGS) -o $* $@ $(OBJS)

# To use: % make test_htable.cpp
# Compiles hash table tests
test_htable.cpp : %.cpp : $(HTABLE).cpp $(HTABLE).o
		$(CXX) $(CXXFLAGS) -o $*.out htable/test_htable.cpp $(HTABLE).o

# Compiles test generator files
# To use: % make knapsack_test_gen.cpp
# Example: % make knapsack_test_gen.cpp
# 	--> Creates <*_test_gen> executable
# If it doesn't work, double check <*_test_gen>.cpp is included in the TEST_GENS list above
$(TEST_GENS) : 
		$(CXX) $(CXXFLAGS) -o $* $@

# Compiles all object files (internally used)
%.o : %.cpp %.h
		$(CXX) $(CXXFLAGS) -o $@ -c $<

# To use: make clean
# Cleans all object files and executables ending in .out
# Also cleans up executables made by make $(TARGETS) and make $(TEST_GENS)
clean:
		-rm -f *.o *.out $(TEST_GENS:.cpp=) $(TARGETS:.cpp=)
