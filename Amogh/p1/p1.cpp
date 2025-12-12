#include <bits/stdc++.h>
using namespace std;

int main() {
    string filename = "graph_1000.csv";
    ifstream file(filename);

    if (!file.is_open()) {
        cout << "Error opening CSV file!" << endl;
        return 1;
    }

    string line;
    vector<vector<pair<int, int>>> adj(101); // nodes 1–100
    bool skipHeader = true;

    // Read CSV file line by line
    while (getline(file, line)) {
        if (skipHeader) { 
            skipHeader = false; 
            continue; 
        }

        stringstream ss(line);
        string u_str, v_str, w_str;

        getline(ss, u_str, ',');
        getline(ss, v_str, ',');
        getline(ss, w_str, ',');

        int u = stoi(u_str);
        int v = stoi(v_str);
        int w = stoi(w_str);

        adj[u].push_back({v, w});
        adj[v].push_back({u, w});
    }

    file.close();

    vector<bool> visited(101, false);
    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;

    int start = 1;
    pq.push({0, start});

    long long totalCost = 0;

    while (!pq.empty()) {
        auto [wt, node] = pq.top();
        pq.pop();

        if (visited[node]) continue;
        visited[node] = true;
        totalCost += wt;

        for (auto &edge : adj[node]) {
            int next = edge.first;
            int weight = edge.second;
            if (!visited[next]) {
                pq.push({weight, next});
            }
        }
    }

    cout << "Minimum Cost to Connect All Substations = " << totalCost << endl;
    return 0;
}
