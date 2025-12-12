#include <bits/stdc++.h>
using namespace std;

// Deadlock detection in a directed "wait-for" graph using DFS recursion stack.
// Input CSV format (first line header):
// num_nodes,num_edges
// Then num_edges lines: u,v
// Means: process/vehicle u is waiting for resource held by v (directed edge u->v).
// Nodes are 0-indexed integers in [0, num_nodes-1].

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string header;
    if (!getline(cin, header)) return 0;
    stringstream ss(header);
    int n, m;
    char comma;
    ss >> n >> comma >> m;

    vector<vector<int>> adj(n);
    for (int i = 0; i < m; ++i) {
        string line;
        if (!getline(cin, line)) break;
        if (line.size() == 0) { --i; continue; }
        stringstream es(line);
        int u, v; char c;
        es >> u >> c >> v;
        if (u < 0 || u >= n || v < 0 || v >= n) continue;
        adj[u].push_back(v);
    }

    vector<int> color(n, 0); // 0=unvisited,1=visiting,2=visited
    vector<int> parent(n, -1);
    vector<int> cycle;
    function<bool(int)> dfs = [&](int u) -> bool {
        color[u] = 1;
        for (int v : adj[u]) {
            if (color[v] == 0) {
                parent[v] = u;
                if (dfs(v)) return true;
            } else if (color[v] == 1) {
                // back-edge found: reconstruct cycle from u -> ... -> v
                cycle.clear();
                cycle.push_back(v);
                for (int cur = u; cur != v && cur != -1; cur = parent[cur])
                    cycle.push_back(cur);
                reverse(cycle.begin(), cycle.end());
                return true;
            }
        }
        color[u] = 2;
        return false;
    };

    bool found = false;
    for (int i = 0; i < n; ++i) {
        if (color[i] == 0) {
            if (dfs(i)) { found = true; break; }
        }
    }

    if (!found) {
        cout << "NO_DEADLOCK\n";
    } else {
        cout << "DEADLOCK\n";
        cout << "Cycle length: " << cycle.size() << "\n";
        cout << "Cycle nodes: ";
        for (size_t i = 0; i < cycle.size(); ++i) {
            if (i) cout << " -> ";
            cout << cycle[i];
        }
        cout << "\n";
    }

    return 0;
}
