// multi_source_bfs_expanded.cpp
// Expanded and fully commented Multi-Source BFS implementation.
// Compile with: g++ -std=c++17 -O2 -o multi_source_bfs_expanded multi_source_bfs_expanded.cpp
// Usage: ./multi_source_bfs_expanded graph_with_stations.csv
//
// Input CSV format (header required):
// type,u,v
// - type = "E" for undirected edge between integer nodes u and v
// - type = "S" for a fire station at node u (v column can be empty)
// Nodes should be positive integers (1..N). The program auto-detects the max node id.
// Output: distances.csv with columns node_id,distance,nearest_station

#include <bits/stdc++.h>
using namespace std;

// ------------------------------ Utilities ----------------------------------

// Trim whitespace from both ends of a string (in-place)
static inline void trim_inplace(string &s) {
    const char* ws = " \t\r\n";
    size_t start = s.find_first_not_of(ws);
    if (start == string::npos) { s.clear(); return; }
    size_t end = s.find_last_not_of(ws);
    s = s.substr(start, end - start + 1);
}

// Simple safe stoi/stol wrapper that returns -1 on error
static inline int safe_stoi(const string &s) {
    try {
        return stoi(s);
    } catch (...) {
        return -1;
    }
}

// Split CSV line into up to 3 fields (type,u,v). This avoids pulling in a CSV library.
// It does not support quoted commas; it's enough for our simple format.
void split_line_csv3(const string &line, string &a, string &b, string &c) {
    a.clear(); b.clear(); c.clear();
    size_t i = 0;
    size_t n = line.size();

    // field a
    while (i < n && line[i] != ',') { a.push_back(line[i]); ++i; }
    if (i < n && line[i] == ',') ++i;
    // field b
    while (i < n && line[i] != ',') { b.push_back(line[i]); ++i; }
    if (i < n && line[i] == ',') ++i;
    // field c (rest)
    while (i < n) { c.push_back(line[i]); ++i; }

    trim_inplace(a); trim_inplace(b); trim_inplace(c);
}

// --------------------------- Graph Reader ----------------------------------

// Structure to hold parsing results
struct GraphData {
    int max_node = 0;                 // maximum node index discovered
    vector<pair<int,int>> edges;      // undirected edges (u,v)
    vector<int> stations;             // list of station node IDs
};

// Read CSV file and produce GraphData. Returns true on success.
bool read_graph_csv(const string &filename, GraphData &out, string &err) {
    ifstream fin(filename);
    if (!fin.is_open()) {
        err = "Cannot open file: " + filename;
        return false;
    }

    string header;
    if (!getline(fin, header)) {
        err = "Empty file or unable to read header.";
        return false;
    }
    // Basic check - header should contain "type" and "u"
    {
        string lower_header = header;
        for (auto &ch : lower_header) ch = (char)tolower((unsigned char)ch);
        if (lower_header.find("type") == string::npos || lower_header.find("u") == string::npos) {
            // Not fatal; continue but warn
            cerr << "Warning: CSV header doesn't contain 'type'/'u' fields. Continuing anyway.\n";
        }
    }

    string line;
    int line_no = 1;
    while (getline(fin, line)) {
        ++line_no;
        if (line.empty()) continue;

        string t, us, vs;
        split_line_csv3(line, t, us, vs);
        if (t.empty() && us.empty() && vs.empty()) continue; // blank line

        // Accept type values "E" or "S" (case-insensitive)
        string type = t;
        for (auto &ch : type) ch = (char)toupper((unsigned char)ch);

        if (type == "E") {
            if (us.empty() || vs.empty()) {
                // malformed edge row; skip but warn
                cerr << "Warning: malformed edge at line " << line_no << " (missing u or v). Skipping.\n";
                continue;
            }
            int u = safe_stoi(us);
            int v = safe_stoi(vs);
            if (u <= 0 || v <= 0) {
                cerr << "Warning: invalid node ids at line " << line_no << ". Skipping.\n";
                continue;
            }
            if (u == v) {
                // self-loop not useful for shortest path; skip quietly
                continue;
            }
            out.edges.emplace_back(u, v);
            out.max_node = max(out.max_node, max(u, v));
        } else if (type == "S") {
            if (us.empty()) {
                cerr << "Warning: malformed station row at line " << line_no << " (missing u). Skipping.\n";
                continue;
            }
            int s = safe_stoi(us);
            if (s <= 0) {
                cerr << "Warning: invalid station id at line " << line_no << ". Skipping.\n";
                continue;
            }
            out.stations.push_back(s);
            out.max_node = max(out.max_node, s);
        } else {
            // Unknown row type; ignore with a warning
            cerr << "Warning: unknown type '" << t << "' at line " << line_no << ". Skipping.\n";
            continue;
        }
    }

    fin.close();
    return true;
}

// ---------------------- Multi-source BFS Logic -----------------------------

// Run multi-source BFS on adjacency list adj (1-indexed nodes).
// stations: list of starting nodes
// Returns: pair(distances vector, origin stations vector)
// distances: -1 for unreachable, otherwise min distance (#edges) from nearest station
// origin: for each node the station id that is nearest (0 if unreachable)
pair<vector<int>, vector<int>> multi_source_bfs(const vector<vector<int>> &adj, const vector<int> &stations) {
    int n = (int)adj.size() - 1; // adj is 1..n
    vector<int> dist(n + 1, -1);
    vector<int> origin(n + 1, 0);
    deque<int> q;

    // Initialize with stations
    for (int s : stations) {
        if (s <= 0 || s > n) continue;
        if (dist[s] == 0) continue; // duplicate station entry skip
        dist[s] = 0;
        origin[s] = s;
        q.push_back(s);
    }

    // Standard BFS
    while (!q.empty()) {
        int u = q.front();
        q.pop_front();
        // iterate neighbors
        for (int v : adj[u]) {
            if (v <= 0 || v > n) continue;
            if (dist[v] == -1) {
                dist[v] = dist[u] + 1;
                origin[v] = origin[u]; // propagate which station reached it
                q.push_back(v);
            }
        }
    }
    return {dist, origin};
}

// --------------------------- Main Program ---------------------------------

int main(int argc, char** argv) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cout << "Multi-Source BFS (expanded implementation)\n";
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " graph_with_stations.csv\n";
        return 1;
    }
    string csv_file = argv[1];

    // 1) Read CSV and build GraphData
    GraphData gd;
    string err;
    cout << "Reading CSV '" << csv_file << "' ...\n";
    if (!read_graph_csv(csv_file, gd, err)) {
        cerr << "Error reading CSV: " << err << "\n";
        return 1;
    }

    if (gd.max_node <= 0) {
        cerr << "No nodes found in input CSV. Exiting.\n";
        return 1;
    }

    cout << "Parsed graph: max_node = " << gd.max_node
         << ", edges = " << gd.edges.size()
         << ", station rows = " << gd.stations.size() << "\n";

    // Remove duplicate stations while preserving order
    {
        sort(gd.stations.begin(), gd.stations.end());
        gd.stations.erase(unique(gd.stations.begin(), gd.stations.end()), gd.stations.end());
        // If you want to preserve original order instead of sort/unique:
        // use an ordered set or boolean visited array to filter duplicates.
    }
    cout << "Unique stations count = " << gd.stations.size() << "\n";

    // 2) Build adjacency list
    int N = gd.max_node;
    vector<vector<int>> adj(N + 1);
    // Reserve some space heuristically: average degree ~ 4 maybe
    for (int i = 1; i <= N; ++i) adj[i].reserve(4);

    // Add edges (undirected), avoid duplicating exact pairs (u,v) and self-loops
    // We'll track seen edges with unordered_set of 64-bit keys: key = ((uint64)min<<32) | max
    unordered_set<uint64_t> seen_edges;
    seen_edges.reserve(gd.edges.size() * 2 + 10);

    for (auto &e : gd.edges) {
        int u = e.first, v = e.second;
        if (u <= 0 || v <= 0 || u > N || v > N) continue;
        if (u == v) continue;
        int a = min(u,v), b = max(u,v);
        uint64_t key = ( (uint64_t)a << 32 ) | (uint64_t)b;
        if (seen_edges.find(key) != seen_edges.end()) continue; // duplicate
        seen_edges.insert(key);
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    // 3) Ensure stations exist as nodes even if isolated (adj list exists)
    for (int s : gd.stations) {
        if (s > N) {
            // enlarge adjacency (shouldn't normally happen because max_node tracked)
            adj.resize(s + 1);
            N = s;
        }
    }

    cout << "Adjacency built. Effective nodes: 1.." << N << ".\n";

    // 4) Run multi-source BFS
    cout << "Starting multi-source BFS from " << gd.stations.size() << " stations ...\n";
    auto result = multi_source_bfs(adj, gd.stations);
    vector<int> dist = move(result.first);
    vector<int> origin = move(result.second);

    // 5) Basic statistics
    int reachable = 0;
    int unreachable = 0;
    int max_dist = -1;
    int farthest_node = -1;
    unordered_map<int,int> count_by_station;
    for (int node = 1; node <= N; ++node) {
        int d = dist[node];
        if (d >= 0) {
            ++reachable;
            if (d > max_dist) {
                max_dist = d;
                farthest_node = node;
            }
            int st = origin[node];
            if (st > 0) ++count_by_station[st];
        } else {
            ++unreachable;
        }
    }

    cout << "Reachable nodes = " << reachable << ", unreachable = " << unreachable << "\n";
    if (max_dist >= 0) {
        cout << "Farthest node from any station: node " << farthest_node << " at distance " << max_dist << " edges.\n";
    } else {
        cout << "No node reached (no stations or empty graph).\n";
    }

    // 6) Write distances CSV
    string out_csv = "distances.csv";
    cout << "Writing distances to '" << out_csv << "' ...\n";
    ofstream fout(out_csv);
    if (!fout.is_open()) {
        cerr << "Failed to open output file for writing: " << out_csv << "\n";
        return 1;
    }
    fout << "node_id,distance,nearest_station\n";
    for (int node = 1; node <= N; ++node) {
        fout << node << "," << dist[node] << "," << origin[node] << "\n";
    }
    fout.close();
    cout << "Wrote " << N << " rows to '" << out_csv << "'.\n";

    // 7) Optional: write per-station coverage summary
    string summary_csv = "station_coverage.csv";
    cout << "Writing per-station coverage to '" << summary_csv << "' ...\n";
    ofstream fs(summary_csv);
    if (fs.is_open()) {
        fs << "station_id,covered_nodes\n";
        for (auto &p : count_by_station) {
            fs << p.first << "," << p.second << "\n";
        }
        fs.close();
        cout << "Wrote station coverage summary to '" << summary_csv << "'.\n";
    } else {
        cerr << "Warning: could not open '" << summary_csv << "' for writing.\n";
    }

    cout << "Done.\n";
    return 0;
}
