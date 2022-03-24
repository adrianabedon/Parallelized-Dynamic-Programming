# Parallel Top Down Dynamic Programming through Lock-Free Hash Tables

## Summary
We are going to parallelize top-down dynamic programs by implementing a concurrent lock free hash table. Once we have an implementation of concurrent hash tables, we will analyze the speedup achieved on a selection of dynamic programming recurrence such as shortest path and the knapsack problem. 

## Background
Dynamic programming is a method for solving problems that are easily divisible into smaller subproblems. Oftentimes, many of these problems will need the result of a subproblem multiple times throughout the course of the algorithm. Dynamic programming allows for subproblems to be computed once by storing the results in a dictionary so they can be reused later. Figure 1 shows the recurrence structure for computing the 5th fibonacci number, and Figure 2 shows how dynamic programming can reduce how many times each subproblem is computed.

Hash tables are one possible data structure for the implementation of the dictionary used in dynamic programming. Most hash table implementations do not support concurrent operations. Therefore, one of our project deliverables will be a concurrent hash table.

With a concurrent hash table, dynamic programming can be parallelized by having multiple threads compute subproblems in parallel, and once they have the result, they store it in the hashtable. If a thread needs to compute a subproblem where the result is in the hashtable, it will simply read the result and continue, reducing the number of subproblems that will be computed. While there is a chance that some subproblems may be computed multiple times if two threads work on the subproblem at the same time, through either randomization or dynamic assignment of subproblems, duplicate computations can be reduced. 

## Challenges
We expect this problem to have several challenging aspects. To attain significant speedup, we plan to design a lock-free hash table so that threads are not delayed by concurrent accesses to the hash table. Additionally, we will need to address and evaluate the distribution of subproblems among the threads such that the dependencies are minimized. Some ideas of how we can address this are randomizing the assignment or distributing the subproblems manually among the threads through worker thread queues. Another challenging aspect is that the cache locality greatly depends on the distribution of the subproblems and our implementation of the hash table. If the distribution is scattered, this may lead to a scattered distribution of underlying memory accesses. Lastly, we would like to focus on minimizing the number of subproblems that are computed multiple times while maintaining high utilization of resources. Introducing a worker thread queue may also help with this minimization.

## Resources

## Goals

## Deliverables

## Platform Choice

## Schedule
