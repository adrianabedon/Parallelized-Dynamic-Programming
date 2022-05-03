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
#include <limits.h>

using namespace std;

void generate_random_graph(int num_vertices, int num_edges, int max_weight, vector<vector<int> > &graph) {
    random_device dev;
    mt19937 rng(dev());
    uniform_int_distribution<mt19937::result_type> rand_vertex(0, num_vertices-1);
    uniform_int_distribution<mt19937::result_type> rand_weight(1, max_weight);

    int i = 0;
    while (i < num_edges) {
        graph[i][0] = (int)rand_vertex(rng);
        graph[i][1] = (int)rand_vertex(rng);
        graph[i][2] = (int)rand_weight(rng);

        // Disallow self loops
        if (graph[i][0] == graph[i][1])
            continue;
        
        // Check if the edge is already represented
        for (int j = 0; j < i; j++) {
            // Already have this directed edge
            if (graph[i][0] == graph[j][0] && graph[i][1] == graph[j][1])
                i--;
        }

        i++;
    }
}

/* Sourced from the Crazy Programmer */
bool ref_bellman(int num_vertices, int num_edges, int source, vector<vector<int> > &graph, vector<int> &distance) {
    int i, j;
    int u, v, w;
    
    distance[source] = 0;

    // num_vertices - 1 passes
    for (i = 1; i < num_vertices; i++) {
        // for each edge in the graph
        for (j = 0; j < num_edges; j++) {
            u = graph[j][0];
            v = graph[j][1];
            w = graph[j][2];

            // if distance[v] is smaller with this edge, replace it
            if (distance[u] != INT_MAX && distance[u] + w < distance[v]) {
                distance[v] = distance[u] + w;
            }
        }
    }

    for (j = 0; j < num_edges; j++) {
        u = graph[j][0];
        v = graph[j][1];
        w = graph[j][2];

        if (distance[u] != INT_MAX && distance[u] + w < distance[v]) {
            // Found negative loop!
            return false;
        }
    }

    return true;
}

int main() {
    // Number of vertices in graph
    int num_vertices = 100;
    // Max weight an edge can have
    int max_weight = 300;
    // Density of graph (can range from 0 to 1)
    double density = 0.75;
    // Computed number of edges based on density for a directed graph
    int num_edges = (int) (density * (num_vertices) * (num_vertices-1));

    // Each edge is a vector of (u,v,w) = (vertex 1, vertex 2, weight of edge)
    vector<int> edge (3);
    vector<vector<int> > graph (num_edges, edge);

    // Randomly generate graph
    generate_random_graph(num_vertices, num_edges, max_weight, graph);

    // Create test file
    char *test_name = new char[100];
    sprintf(test_name, "tests/bellman/bellman_%d_%d.txt", num_vertices, num_edges);
    FILE *file = fopen(test_name, "w");
    assert(file != NULL);

    // Print |V| |E|
    fprintf(file, "%d %d\n", num_vertices, num_edges);

    // Generate distinct random source and destination
    random_device dev;
    mt19937 rng(dev());
    uniform_int_distribution<mt19937::result_type> rand_vertex(0, num_vertices-1);
    int source = (int)rand_vertex(rng);
    int destination = source;
    while (destination == source)
        destination = (int)rand_vertex(rng);

    // Print source dest
    fprintf(file, "%d %d\n", source, destination);

    // Print u1 v1 w1
    //       u2 v2 w2
    //       ...
    for(int i = 0; i < num_edges; i++) {
        fprintf(file, "%d %d %d\n", graph[i][0], graph[i][1], graph[i][2]);
    }

    // Running reference solution
    // Allocate the distance array
    vector<int> distance (num_vertices, INT_MAX);
    auto start = std::chrono::high_resolution_clock::now();
    ref_bellman(num_vertices, num_edges, source, graph, distance);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> float_ms = end - start;
    printf("refsol took %lf milliseconds\n", float_ms.count());
    fprintf(file, "%d\n", distance[destination]);
    fclose(file);
}