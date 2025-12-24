#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <set>
#include <unordered_map>
#include <cmath>
using namespace std;

/*
MODEL 9: MFAPO – Multi-Facility Accessibility & Placement Optimizer
Uses K-Means + Graph + Prim's MST + BFS + DFS
*/

// ---------------- STRUCT ----------------
struct Point {
    int x, y;
};

struct Edge {
    int v, w;
};

// ---------------- GLOBALS ----------------
int V = 6;
vector<vector<Edge>> graph(V);
bool visitedDFS[10] = {false};
bool visitedBFS[10] = {false};

// ---------------- DISTANCE FUNCTION ----------------
int dist(Point a, Point b) {
    return abs(a.x - b.x) + abs(a.y - b.y);
}

// ---------------- K-MEANS (2 CLUSTERS) ----------------
void kMeans(vector<Point>& zones, Point &c1, Point &c2) {
    cout << "\nK-Means Clustering:\n";
    for (auto &z : zones) {
        if (dist(z, c1) < dist(z, c2))
            cout << "(" << z.x << "," << z.y << ") -> Cluster 1\n";
        else
            cout << "(" << z.x << "," << z.y << ") -> Cluster 2\n";
    }
}

// ---------------- PRIM'S MST ----------------
void primMST() {
    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;
    vector<int> key(V, 1e9);
    vector<bool> inMST(V, false);

    key[0] = 0;
    pq.push({0,0});

    cout << "\nPrim's MST Edges:\n";

    while(!pq.empty()) {
        int u = pq.top().second;
        pq.pop();

        if(inMST[u]) continue;
        inMST[u] = true;

        for(auto &e : graph[u]) {
            int v = e.v, w = e.w;
            if(!inMST[v] && w < key[v]) {
                key[v] = w;
                pq.push({key[v], v});
                cout << "Edge " << u << " - " << v << " Weight " << w << endl;
            }
        }
    }
}

// ---------------- BFS ACCESSIBILITY ----------------
void bfsAccess(int start) {
    queue<int> q;
    q.push(start);
    visitedBFS[start] = true;

    cout << "\nBFS Accessibility from Facility " << start << ":\n";

    while(!q.empty()) {
        int u = q.front(); q.pop();
        cout << u << " ";

        for(auto &e : graph[u]) {
            if(!visitedBFS[e.v]) {
                visitedBFS[e.v] = true;
                q.push(e.v);
            }
        }
    }
    cout << endl;
}

// ---------------- DFS REACHABILITY ----------------
void dfsReach(int u) {
    visitedDFS[u] = true;
    cout << u << " ";

    for(auto &e : graph[u]) {
        if(!visitedDFS[e.v])
            dfsReach(e.v);
    }
}

// ---------------- MAIN ----------------
int main() {

    cout << "=== MFAPO : Facility Placement Optimizer ===\n";

    // ---------- ZONE DEMAND ----------
    unordered_map<int,int> demand = {
        {0, 30}, {1, 50}, {2, 20},
        {3, 40}, {4, 25}, {5, 35}
    };

    // ---------- GRAPH CONSTRUCTION ----------
    graph[0].push_back({1,4});
    graph[1].push_back({2,6});
    graph[2].push_back({3,5});
    graph[3].push_back({4,3});
    graph[4].push_back({5,7});
    graph[5].push_back({0,2});

    // ---------- K-MEANS ----------
    vector<Point> zones = {{1,2},{2,3},{5,4},{7,8},{8,9}};
    Point c1 = {2,2}, c2 = {8,8};
    kMeans(zones, c1, c2);

    // ---------- MST ----------
    primMST();

    // ---------- BFS ----------
    bfsAccess(0);

    // ---------- DFS ----------
    cout << "\nDFS Reachability:\n";
    dfsReach(0);
    cout << endl;

    cout << "\nFacility placement optimization completed.\n";
    return 0;
}
