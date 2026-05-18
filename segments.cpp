
#include <iostream>

const int INF = 2147483647;

// Structure for Adjacency List
struct Node {
    int vertex;
    int weight;
    Node* next;
    Node(int v, int w) : vertex(v), weight(w), next(nullptr) {}
};

class TSPGraph {
public:
    int numCities;
    int** matrix;      // Adjacency Matrix
    Node** adjList;    // Adjacency List Array

    TSPGraph(int n) : numCities(n) {
        // Allocate Matrix
        matrix = new int* [n];
        // Allocate List Array
        adjList = new Node * [n];

        for (int i = 0; i < n; i++) {
            matrix[i] = new int[n](); // Initialize to 0
            adjList[i] = nullptr;
        }
    }

    // Example of adding an edge to both
    void addEdge(int u, int v, int weight) {
        if (weight > 0) {
            // Matrix Insertion: O(1)
            matrix[u][v] = weight;

            // List Insertion (Insert at head): O(1)
            Node* newNode = new Node(v, weight);
            newNode->next = adjList[u];
            adjList[u] = newNode;
        }
    }
};
//Adjacency Matrix : A 2D array.Space complexity is $O(V ^ 2)$.Checking the weight between city $u$ and city $v$ is instant $O(1)$.
// Adjacency List : An array of linked lists.Space complexity is $O(V + E)$.To find the weight between $u$ and $v$,
// you must traverse $u$'s linked list, taking $O(V)$ time in the worst case.

int greedyTSPMatrix(int startNode, int* outRoute, TSPGraph* graph) {
    int n = graph->numCities;
    if (n <= 1) { outRoute[0] = 0; return 0; }

    bool* visited = new bool[n]();
    visited[startNode] = true;
    outRoute[0] = startNode;

    int currentCost = 0;
    int currentNode = startNode;
    int routeIndex = 1;

    for (int i = 0; i < n - 1; i++) {
        int nextNode = -1;
        int minEdge = INF;

        // Search all neighbors for the cheapest unvisited edge
        for (int j = 0; j < n; j++) {
            if (!visited[j] && graph->matrix[currentNode][j] > 0 && graph->matrix[currentNode][j] < minEdge) {
                minEdge = graph->matrix[currentNode][j];
                nextNode = j;
            }
        }

        visited[nextNode] = true;
        outRoute[routeIndex++] = nextNode;
        currentCost += minEdge;
        currentNode = nextNode;
    }

    // Add cost to return to the starting city
    outRoute[routeIndex] = startNode;
    currentCost += graph->matrix[currentNode][startNode];

    delete[] visited;
    return currentCost;
}

//The time complexity is $O(V ^ 2)$ because for each of the $V$ cities,
//we loop through all $V$ possible neighbors.It is incredibly fast but usually yields a sub - optimal route
//because it doesn't plan ahead; it might paint itself into a corner and be forced to take a massively expensive edge back to the start.

int calculateCost(int* route, int n, TSPGraph* graph) {
    int cost = 0;
    for (int i = 0; i < n; i++) {
        cost += graph->matrix[route[i]][route[i + 1]];
    }
    return cost;
}

int optimizedGreedyTSP(int startNode, int* outRoute, TSPGraph* graph) {
    int n = graph->numCities;

    // Step 1: Get an initial route (e.g., from Nearest Neighbor)
    int bestCost = greedyTSPMatrix(startNode, outRoute, graph);
    bool improved = true;

    // Step 2: Continuously try swapping edges until no improvements can be made
    while (improved) {
        improved = false;
        for (int i = 1; i < n - 1; i++) {
            for (int j = i + 1; j < n; j++) {

                // Reverse the sub-array between i and j (This performs the 2-Opt swap)
                int l = i, r = j;
                while (l < r) {
                    int temp = outRoute[l]; outRoute[l] = outRoute[r]; outRoute[r] = temp;
                    l++; r--;
                }

                // Calculate new cost
                int newCost = calculateCost(outRoute, n, graph);

                if (newCost < bestCost) {
                    bestCost = newCost;
                    improved = true;
                }
                else {
                    // If not better, reverse the array back to its original state
                    l = i; r = j;
                    while (l < r) {
                        int temp = outRoute[l]; outRoute[l] = outRoute[r]; outRoute[r] = temp;
                        l++; r--;
                    }
                }
            }
        }
    }
    return bestCost;
}

//2 - Opt fixes the mistakes of the Greedy algorithm.By reversing the sub - array between nodes $i$ and $j$,
//it effectively deletes two crossing edges and reconnects them so they don't cross.
//The complexity is roughly $O(V^3)$ per pass because it contains nested loops checking pairs of edges, and recalculating the path cost takes $O(V)$.

void permute(int* currRoute, bool* visited, int currPos, int currCost, int& minCost, int* bestRoute, TSPGraph* graph) {
    int n = graph->numCities;

    // Base Case: All cities visited
    if (currPos == n) {
        int returnCost = graph->matrix[currRoute[currPos - 1]][currRoute[0]];
        if ((currCost + returnCost) < minCost) {
            minCost = currCost + returnCost;
            for (int i = 0; i < n; i++) bestRoute[i] = currRoute[i];
            bestRoute[n] = currRoute[0]; // Add start node to the end
        }
        return;
    }

    // Recursive Case: Try all unvisited cities
    for (int i = 0; i < n; i++) {
        if (!visited[i] && graph->matrix[currRoute[currPos - 1]][i] > 0) {

            visited[i] = true;
            currRoute[currPos] = i;

            // Recurse deeper into the tree
            permute(currRoute, visited, currPos + 1, currCost + graph->matrix[currRoute[currPos - 1]][i], minCost, bestRoute, graph);

            // Backtrack
            visited[i] = false;
        }
    }
}

//Uses Depth - First Search(DFS) with backtracking to generate all permutations of the cities.Because the first city is fixed,
//there are $(V - 1)!$ possible routes.The time complexity is $O(V!)$.
//It guarantees a 100 % optimal answer but becomes computationally impossible for anything roughly over 12 cities due to combinatorial explosion.


#include <random>

void generateRandomGraph(TSPGraph* graph) {
    int n = graph->numCities;

    // Set up standard C++ random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(10, 100); // Random weights between 10 and 100

    for (int i = 0; i < n; i++) {
        for (int j = i; j < n; j++) {
            if (i == j) {
                // Distance to itself is 0
                graph->matrix[i][j] = 0;
            }
            else {
                // Generate a symmetric weight (A to B equals B to A)
                int weight = dist(gen);
                graph->matrix[i][j] = weight;
                graph->matrix[j][i] = weight;

                // Add to Adjacency list representations as well
                graph->addEdge(i, j, weight);
                graph->addEdge(j, i, weight);
            }
        }
    }
}

//The loop only iterates from $j = i$ to $V$, rather than resetting $j = 0$.This ensures we generate a weight once,
//and assign it symmetrically to matrix[i][j] and matrix[j][i].
//This mirrors real - world Euclidean distances where the road from City A to City B is the same length as City B to City A.