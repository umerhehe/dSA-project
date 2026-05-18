#ifndef TSP_GRAPH_H
#define TSP_GRAPH_H

const int INF = 2147483647;

struct Node {
    int vertex;
    int weight;
    Node* next;
    Node(int v, int w) : vertex(v), weight(w), next(nullptr) {}
};

class TSPGraph {
public:
    int numCities;
    int edgeCount;
    int** matrix;
    Node** adjList;

    TSPGraph(int n) : numCities(n), edgeCount(0) {
        matrix = new int*[n];
        adjList = new Node*[n];
        for (int i = 0; i < n; i++) {
            matrix[i] = new int[n];
            adjList[i] = nullptr;
            for (int j = 0; j < n; j++) matrix[i][j] = 0;
        }
    }

    ~TSPGraph() {
        for (int i = 0; i < numCities; i++) {
            delete[] matrix[i];
            Node* curr = adjList[i];
            while (curr) {
                Node* temp = curr;
                curr = curr->next;
                delete temp;
            }
        }
        delete[] matrix;
        delete[] adjList;
    }

    void addEdge(int u, int v, int weight) {
        if (weight > 0) {
            matrix[u][v] = weight;
            Node* newNode = new Node(v, weight);
            newNode->next = adjList[u];
            adjList[u] = newNode;
            edgeCount++;
        }
    }

    int getListWeight(int u, int v) {
        Node* curr = adjList[u];
        while (curr) {
            if (curr->vertex == v) return curr->weight;
            curr = curr->next;
        }
        return 0;
    }

    int calculateCostMatrix(int* route) {
        int cost = 0;
        for (int i = 0; i < numCities; i++) {
            if (matrix[route[i]][route[i+1]] == 0) return INF;
            cost += matrix[route[i]][route[i+1]];
        }
        return cost;
    }

    int calculateCostList(int* route) {
        int cost = 0;
        for (int i = 0; i < numCities; i++) {
            int w = getListWeight(route[i], route[i+1]);
            if (w == 0) return INF;
            cost += w;
        }
        return cost;
    }

    int greedyTSPMatrix(int startNode, int* outRoute) {
        if (numCities <= 1) { outRoute[0] = 0; return 0; }
        bool* visited = new bool[numCities]();
        visited[startNode] = true; outRoute[0] = startNode;
        int currentCost = 0, currentNode = startNode, routeIndex = 1;

        for (int i = 0; i < numCities - 1; i++) {
            int nextNode = -1, minEdge = INF;
            for (int j = 0; j < numCities; j++) {
                if (!visited[j] && matrix[currentNode][j] > 0 && matrix[currentNode][j] < minEdge) {
                    minEdge = matrix[currentNode][j]; nextNode = j;
                }
            }
            if (nextNode == -1) { delete[] visited; return -1; }
            visited[nextNode] = true; outRoute[routeIndex++] = nextNode;
            currentCost += minEdge; currentNode = nextNode;
        }
        if (matrix[currentNode][startNode] == 0) { delete[] visited; return -1; }
        outRoute[routeIndex] = startNode; currentCost += matrix[currentNode][startNode];
        delete[] visited; return currentCost;
    }

    int greedyTSPList(int startNode, int* outRoute) {
        if (numCities <= 1) { outRoute[0] = 0; return 0; }
        bool* visited = new bool[numCities]();
        visited[startNode] = true; outRoute[0] = startNode;
        int currentCost = 0, currentNode = startNode, routeIndex = 1;

        for (int i = 0; i < numCities - 1; i++) {
            int nextNode = -1, minEdge = INF;
            Node* neighbor = adjList[currentNode];
            while (neighbor != nullptr) {
                if (!visited[neighbor->vertex] && neighbor->weight < minEdge) {
                    minEdge = neighbor->weight; nextNode = neighbor->vertex;
                }
                neighbor = neighbor->next;
            }
            if (nextNode == -1) { delete[] visited; return -1; }
            visited[nextNode] = true; outRoute[routeIndex++] = nextNode;
            currentCost += minEdge; currentNode = nextNode;
        }
        int returnCost = getListWeight(currentNode, startNode);
        if (returnCost == 0) { delete[] visited; return -1; }
        outRoute[routeIndex] = startNode; currentCost += returnCost;
        delete[] visited; return currentCost;
    }

    int optimizedGreedyTSPMatrix(int startNode, int* outRoute) {
        int bestCost = greedyTSPMatrix(startNode, outRoute);
        if (bestCost == -1) return -1;
        bool improved = true;
        while (improved) {
            improved = false;
            for (int i = 1; i < numCities - 1; i++) {
                for (int j = i + 1; j < numCities; j++) {
                    int l = i, r = j;
                    while (l < r) { int t = outRoute[l]; outRoute[l] = outRoute[r]; outRoute[r] = t; l++; r--; }
                    int newCost = calculateCostMatrix(outRoute);
                    if (newCost < bestCost) { bestCost = newCost; improved = true; }
                    else { l = i; r = j; while (l < r) { int t = outRoute[l]; outRoute[l] = outRoute[r]; outRoute[r] = t; l++; r--; } }
                }
            }
        }
        return bestCost;
    }

    int optimizedGreedyTSPList(int startNode, int* outRoute) {
        int bestCost = greedyTSPList(startNode, outRoute);
        if (bestCost == -1) return -1;
        bool improved = true;
        while (improved) {
            improved = false;
            for (int i = 1; i < numCities - 1; i++) {
                for (int j = i + 1; j < numCities; j++) {
                    int l = i, r = j;
                    while (l < r) { int t = outRoute[l]; outRoute[l] = outRoute[r]; outRoute[r] = t; l++; r--; }
                    int newCost = calculateCostList(outRoute);
                    if (newCost < bestCost) { bestCost = newCost; improved = true; }
                    else { l = i; r = j; while (l < r) { int t = outRoute[l]; outRoute[l] = outRoute[r]; outRoute[r] = t; l++; r--; } }
                }
            }
        }
        return bestCost;
    }

    void permuteMatrix(int* currRoute, bool* visited, int currPos, int currCost, int& minCost, int* bestRoute) {
        if (currPos == numCities) {
            int returnCost = matrix[currRoute[currPos - 1]][currRoute[0]];
            if (returnCost > 0 && (currCost + returnCost) < minCost) {
                minCost = currCost + returnCost;
                for (int i = 0; i < numCities; i++) bestRoute[i] = currRoute[i];
                bestRoute[numCities] = currRoute[0];
            }
            return;
        }
        for (int i = 0; i < numCities; i++) {
            if (!visited[i] && matrix[currRoute[currPos - 1]][i] > 0) {
                visited[i] = true; currRoute[currPos] = i;
                permuteMatrix(currRoute, visited, currPos + 1, currCost + matrix[currRoute[currPos - 1]][i], minCost, bestRoute);
                visited[i] = false;
            }
        }
    }

    int bruteForceTSPMatrix(int startNode, int* outRoute) {
        if (numCities <= 1) { outRoute[0] = 0; return 0; }
        int* currRoute = new int[numCities]; bool* visited = new bool[numCities]();
        currRoute[0] = startNode; visited[startNode] = true; int minCost = INF;
        permuteMatrix(currRoute, visited, 1, 0, minCost, outRoute);
        delete[] currRoute; delete[] visited; return minCost == INF ? -1 : minCost;
    }

    void permuteList(int* currRoute, bool* visited, int currPos, int currCost, int& minCost, int* bestRoute) {
        if (currPos == numCities) {
            int returnCost = getListWeight(currRoute[currPos - 1], currRoute[0]);
            if (returnCost > 0 && (currCost + returnCost) < minCost) {
                minCost = currCost + returnCost;
                for (int i = 0; i < numCities; i++) bestRoute[i] = currRoute[i];
                bestRoute[numCities] = currRoute[0];
            }
            return;
        }
        Node* neighbor = adjList[currRoute[currPos - 1]];
        while (neighbor) {
            int i = neighbor->vertex;
            if (!visited[i]) {
                visited[i] = true; currRoute[currPos] = i;
                permuteList(currRoute, visited, currPos + 1, currCost + neighbor->weight, minCost, bestRoute);
                visited[i] = false;
            }
            neighbor = neighbor->next;
        }
    }

    int bruteForceTSPList(int startNode, int* outRoute) {
        if (numCities <= 1) { outRoute[0] = 0; return 0; }
        int* currRoute = new int[numCities]; bool* visited = new bool[numCities]();
        currRoute[0] = startNode; visited[startNode] = true; int minCost = INF;
        permuteList(currRoute, visited, 1, 0, minCost, outRoute);
        delete[] currRoute; delete[] visited; return minCost == INF ? -1 : minCost;
    }
};
#endif // TSP_GRAPH_H