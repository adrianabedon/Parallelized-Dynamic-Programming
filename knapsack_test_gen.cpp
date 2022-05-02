#include <cstdio>
#include <vector>
#include <algorithm>
#include <utility>
#include <cassert> 
#include <random>
#include <iostream>
#include <cmath>
#include <chrono>
#include <unistd.h>

using namespace std;

// int max(int a, int b)
// {
//     return (a > b) ? a : b;
// }

/* Sourced from geeksforgeeks */
int ref_knapsack(int W, vector<int> wt, vector<int> val, int n)
{
    int i, w;
    vector<vector<int>> K(n + 1, vector<int>(W + 1));
  
    // Build table K[][] in bottom up manner
    for(i = 0; i <= n; i++)
    {
        for(w = 0; w <= W; w++)
        {
            if (i == 0 || w == 0)
                K[i][w] = 0;
            else if (wt[i - 1] <= w)
                K[i][w] = max(val[i - 1] + 
                                K[i - 1][w - wt[i - 1]],
                                K[i - 1][w]);
            else
                K[i][w] = K[i - 1][w];
        }
    }
    return K[n][W];
}

int main() {
    int num_obj = 100;
    int capacity = 400;
    char *test_name = new char[100];
    sprintf(test_name, "tests/knapsack/knapsack_%d_%d.txt", num_obj, capacity);
    FILE *file = fopen(test_name, "w");
    assert(file != NULL);

    random_device dev;
    mt19937 rng(dev());
    uniform_int_distribution<mt19937::result_type> rand_value(1,1000);
    uniform_int_distribution<mt19937::result_type> rand_weight(1,300);

    vector<int> wt;
    vector<int> val;

    fprintf(file, "%d\n", capacity);
    fprintf(file, "%d\n", num_obj);

    for(int i = 0 ; i < num_obj ; i++) {
        int rand_val = (int)rand_value(rng);
        fprintf(file, "%d\n", rand_val);
        val.push_back(rand_val);
    }

    for(int i = 0 ; i < num_obj ; i++) {
        int rand_val = (int)rand_weight(rng);
        fprintf(file, "%d\n", rand_val);
        wt.push_back(rand_val);
    }

    auto start = std::chrono::high_resolution_clock::now();
    int ref_sol = ref_knapsack(capacity, wt, val, num_obj);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> float_ms = end - start;
    printf("refsol took %lf milliseconds\n", float_ms.count());
    fprintf(file, "%d\n", ref_sol);
    fclose(file);
}