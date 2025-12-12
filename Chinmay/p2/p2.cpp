#include <bits/stdc++.h>
using namespace std;

// Edmonds-Karp (BFS) implementation for Max-Flow.
// Input CSV format (first line header):
// num_nodes,num_edges,source,sink
// Then num_edges lines: u,v,capacity
// Nodes are 0-indexed integers in [0, num_nodes-1].

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string header;
    if (!getline(cin, header)) return 0;
    stringstream ss(header);
    int n, m, s, t;
    char comma;
    ss >> n >> comma >> m >> comma >> s >> comma >> t;

    vector<vector<int>> capacity(n, vector<int>(n, 0));
    vector<vector<int>> adj(n);

    for (int i = 0; i < m; ++i) {
        string line;
        if (!getline(cin, line)) break;
        if (line.size() == 0) { --i; continue; }
        stringstream es(line);
        int u, v, c;
        char csep;
        es >> u >> csep >> v >> csep >> c;
        if (u < 0 || u >= n || v < 0 || v >= n) continue;
        capacity[u][v] += c; // in case multiple edges exist, sum capacities
        // add to adjacency once
        adj[u].push_back(v);
        adj[v].push_back(u); // add reverse edge for residual graph traversal
    }

    auto bfs = [&](vector<int>& parent) -> int {
        fill(parent.begin(), parent.end(), -1);
        parent[s] = -2; // mark source as visited with no parent
        queue<pair<int,int>> q; // node, flow
        q.push({s, INT_MAX});

        while (!q.empty()) {
            auto [cur, flow] = q.front(); q.pop();
            for (int next : adj[cur]) {
                if (parent[next] == -1 && capacity[cur][next] > 0) {
                    parent[next] = cur;
                    int new_flow = min(flow, capacity[cur][next]);
                    if (next == t) return new_flow;
                    q.push({next, new_flow});
                }
            }
        }
        return 0;
    };

    int flow = 0;
    vector<int> parent(n);
    int new_flow;
    while ((new_flow = bfs(parent)) > 0) {
        flow += new_flow;
        int cur = t;
        while (cur != s) {
            int prev = parent[cur];
            capacity[prev][cur] -= new_flow;
            capacity[cur][prev] += new_flow;
            cur = prev;
        }
    }

    cout << "Max Flow: " << flow << '\n';
    return 0;
}
