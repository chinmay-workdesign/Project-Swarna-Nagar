#include <bits/stdc++.h>
using namespace std;

/*
====================================================================================================
                        GRAPH COLORING FOR SUBSTATION FREQUENCY ASSIGNMENT
----------------------------------------------------------------------------------------------------
Adjacent substations cannot use the same frequency. We assign the minimum number of frequencies
using a greedy graph-coloring strategy.

Steps:
1. Load edges from CSV file.
2. Build adjacency list.
3. Iterate through nodes:
        For each node, mark colors used by neighbors.
        Assign the smallest available color.
4. Print the assigned colors.
====================================================================================================
*/

// =================================================================================================
// SECTION 1 — String Utilities
// =================================================================================================

string trim(const string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    if (start == string::npos) return "";
    return s.substr(start, end - start + 1);
}

void printBanner(const string &title) {
    cout << "\n============================================================\n";
    cout << title << "\n";
    cout << "============================================================\n";
}

// =================================================================================================
// SECTION 2 — CSV Loader
// =================================================================================================

class CSVLoader {
public:
    string filename;

    CSVLoader(const string &fn) {
        filename = fn;
    }

    void loadEdges(vector<pair<int,int>> &edges) {
        printBanner("Loading Graph Edges");

        ifstream file(filename);
        if (!file.is_open()) {
            cout << "ERROR: Cannot open CSV file\n";
            exit(1);
        }

        string line;
        bool skip = true;

        while (getline(file, line)) {
            if (skip) {
                skip = false;
                continue;
            }

            stringstream ss(line);
            string a, b;
            getline(ss, a, ',');
            getline(ss, b, ',');

            int u = stoi(trim(a));
            int v = stoi(trim(b));

            edges.push_back({u, v});
        }

        cout << "Total edges loaded: " << edges.size() << "\n";
    }
};

// =================================================================================================
// SECTION 3 — Graph Class with Greedy Coloring
// =================================================================================================

class Graph {
public:
    int maxNodes;
    vector<vector<int>> adj;
    vector<int> color;

    Graph(int n) {
        maxNodes = n;
        adj.resize(maxNodes + 1);
        color.assign(maxNodes + 1, -1);
    }

    void addEdge(int u, int v) {
        if (u > maxNodes || v > maxNodes) return;
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    void printSampleEdges() {
        printBanner("Sample Edges");
        int count = 0;
        for (int i = 1; i <= maxNodes; i++) {
            for (int v : adj[i]) {
                cout << i << " -- " << v << "\n";
                if (++count >= 10) return;
            }
        }
    }

    void greedyColoring() {
        printBanner("Running Greedy Coloring Algorithm");

        vector<bool> available(maxNodes + 1, true);

        color[1] = 0;  

        for (int i = 2; i <= maxNodes; i++) {
            fill(available.begin(), available.end(), true);

            for (int neigh : adj[i]) {
                if (color[neigh] != -1) {
                    available[color[neigh]] = false;
                }
            }

            int cr = 0;
            while (cr <= maxNodes && !available[cr]) {
                cr++;
            }

            color[i] = cr;
        }
    }

    void printColors() {
        printBanner("Assigned Frequency for First 20 Substations");
        for (int i = 1; i <= 20; i++) {
            cout << "Substation " << i << " -> Frequency " << color[i] << "\n";
        }
    }
};

// =================================================================================================
// SECTION 4 — MAIN PROGRAM
// =================================================================================================

int main() {
    printBanner("SUBSTATION FREQUENCY MINIMIZATION — GRAPH COLORING");

    vector<pair<int,int>> edges;
    CSVLoader loader("substation_graph_edges.csv");
    loader.loadEdges(edges);

    Graph g(300);

    for (auto &p : edges) {
        g.addEdge(p.first, p.second);
    }

    g.printSampleEdges();

    g.greedyColoring();

    g.printColors();

    printBanner("PROGRAM COMPLETE");
    return 0;
}

/*
====================================================================================================
END OF FILE
====================================================================================================
*/
