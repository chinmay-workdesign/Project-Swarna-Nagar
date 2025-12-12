#include <bits/stdc++.h>
using namespace std;

// Deadlock detection and analysis tool for directed "wait-for" graphs.
// Extended features:
//  - Robust CSV parsing with comments and blank lines allowed
//  - Command-line options: --input <file>, --output <file>, --generate-sample, --all-cycles, --limit <k>, --quiet
//  - Detects a single cycle (classic deadlock) using DFS + recursion stack (as before)
//  - Optionally enumerates simple cycles up to a configurable limit (Johnson's algorithm)
//  - Reports strongly connected components (SCCs) using Kosaraju
//  - If graph is DAG (no cycles), performs topological sort and prints order
//  - Validates indices and reports statistics and runtime
//
// Input CSV format (first header line):
// num_nodes,num_edges
// Then num_edges lines: u,v
// Means: process/vehicle u is waiting for resource held by v (directed edge u->v).
// Nodes are 0-indexed integers in [0, num_nodes-1].

static inline void ltrim(string &s) { s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char ch){ return !isspace(ch); })); }
static inline void rtrim(string &s) { s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch){ return !isspace(ch); }).base(), s.end()); }
static inline void trim(string &s) { ltrim(s); rtrim(s); }

vector<string> split_csv_line(const string &line) {
    vector<string> tokens;
    string cur;
    for (size_t i = 0; i < line.size(); ++i) {
        char ch = line[i];
        if (ch == ',') { string t = cur; trim(t); tokens.push_back(t); cur.clear(); }
        else cur.push_back(ch);
    }
    string t = cur; trim(t);
    if (!t.empty() || (line.size() && line.back() == ',')) tokens.push_back(t);
    return tokens;
}

optional<long long> parse_int_safe(const string &s) {
    if (s.empty()) return nullopt;
    char *endptr = nullptr;
    errno = 0;
    long long val = strtoll(s.c_str(), &endptr, 10);
    if (errno != 0) return nullopt;
    while (*endptr) { if (!isspace((unsigned char)*endptr)) return nullopt; ++endptr; }
    return val;
}

bool write_sample_csv(const string &filename) {
    ofstream ofs(filename);
    if (!ofs) return false;
    // Sample wait-for graph illustrating a deadlock and some other components
    ofs << "num_nodes,num_edges\n";
    ofs << "12,11\n";
    // A deadlock cycle: 0->1->2->0
    ofs << "0,1\n";
    ofs << "1,2\n";
    ofs << "2,0\n";
    // another cycle: 3->4->5->3
    ofs << "3,4\n";
    ofs << "4,5\n";
    ofs << "5,3\n";
    // acyclic component: 6->7->8
    ofs << "6,7\n";
    ofs << "7,8\n";
    // isolated nodes: 9,10,11 (no edges)
    ofs.close();
    return true;
}

// Kosaraju's algorithm to find SCCs
vector<int> kosaraju_scc(const vector<vector<int>> &adj) {
    int n = (int)adj.size();
    vector<char> vis(n, 0);
    vector<int> order;
    function<void(int)> dfs1 = [&](int u) {
        vis[u] = 1;
        for (int v : adj[u]) if (!vis[v]) dfs1(v);
        order.push_back(u);
    };
    for (int i = 0; i < n; ++i) if (!vis[i]) dfs1(i);
    vector<vector<int>> radj(n);
    for (int u = 0; u < n; ++u) for (int v : adj[u]) radj[v].push_back(u);
    vector<int> comp(n, -1);
    int cid = 0;
    function<void(int)> dfs2 = [&](int u) {
        comp[u] = cid;
        for (int v : radj[u]) if (comp[v] == -1) dfs2(v);
    };
    for (int i = (int)order.size()-1; i >= 0; --i) {
        int u = order[i];
        if (comp[u] == -1) { dfs2(u); ++cid; }
    }
    return comp; // comp[u] gives component id in range [0..cid-1]
}

// Johnson's algorithm for enumerating all simple cycles (returns cycles as vector<vector<int>>)
// We'll implement a reasonably efficient version but cap the number of cycles at a user-provided limit.

class JohnsonSimpleCycles {
public:
    JohnsonSimpleCycles(const vector<vector<int>> &graph_, int limit_ = 1000) : graph(graph_), n((int)graph_.size()), limit(limit_) {
        blocked.assign(n, false);
        B.assign(n, unordered_set<int>());
    }

    vector<vector<int>> run() {
        vector<vector<int>> all_cycles;
        vector<int> stack;
        int s = 0;
        // Work on nodes in lexicographic order
        for (s = 0; s < n; ++s) {
            // Build subgraph induced by nodes >= s
            vector<vector<int>> subg(n);
            for (int u = s; u < n; ++u) {
                for (int v : graph[u]) if (v >= s) subg[u].push_back(v);
            }
            // find cycles in subg starting and ending at s using circuit
            circuit(s, s, subg, stack, all_cycles);
            if ((int)all_cycles.size() >= limit) break;
        }
        return all_cycles;
    }

private:
    const vector<vector<int>> &graph;
    int n;
    int limit;
    vector<char> blocked;
    vector<unordered_set<int>> B;

    bool circuit(int v, int s, const vector<vector<int>> &subg, vector<int> &stack, vector<vector<int>> &all_cycles) {
        bool found = false;
        stack.push_back(v);
        blocked[v] = true;
        for (int w : subg[v]) {
            if (w == s) {
                // output cycle
                vector<int> cycle = stack;
                all_cycles.push_back(cycle);
                if ((int)all_cycles.size() >= limit) return true; // early stop
                found = true;
            } else if (!blocked[w]) {
                if (circuit(w, s, subg, stack, all_cycles)) found = true;
                if ((int)all_cycles.size() >= limit) return true;
            }
        }
        if (found) {
            unblock(v);
        } else {
            for (int w : subg[v]) B[w].insert(v);
        }
        stack.pop_back();
        return found;
    }

    void unblock(int u) {
        blocked[u] = false;
        for (auto it = B[u].begin(); it != B[u].end();) {
            int w = *it;
            it = B[u].erase(it);
            if (blocked[w]) unblock(w);
        }
    }
};

int main(int argc, char **argv) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string input_filename;
    string output_filename;
    bool generate_sample = false;
    bool enumerate_all_cycles = false;
    int cycle_limit = 1000;
    bool quiet = false;

    for (int i = 1; i < argc; ++i) {
        string a = argv[i];
        if (a == "--input" && i+1 < argc) input_filename = argv[++i];
        else if (a == "--output" && i+1 < argc) output_filename = argv[++i];
        else if (a == "--generate-sample") generate_sample = true;
        else if (a == "--all-cycles") enumerate_all_cycles = true;
        else if (a == "--limit" && i+1 < argc) cycle_limit = stoi(argv[++i]);
        else if (a == "--quiet") quiet = true;
        else { cerr << "Unknown arg: " << a << "\n"; cerr << "Usage: " << argv[0] << " [--input file.csv] [--output file.txt] [--generate-sample] [--all-cycles] [--limit k] [--quiet]\n"; return 1; }
    }

    if (generate_sample) {
        const string sample_name = input_filename.empty() ? "sample_waitfor.csv" : input_filename;
        if (write_sample_csv(sample_name)) {
            cout << "Wrote sample CSV to: " << sample_name << "\n";
            if (input_filename.empty()) cout << "Use --input " << sample_name << " to run the detector on it.\n";
            return 0;
        } else { cerr << "Failed to write sample CSV to: " << sample_name << "\n"; return 2; }
    }

    istream *inptr = &cin;
    ifstream ifs;
    if (!input_filename.empty()) {
        ifs.open(input_filename);
        if (!ifs) { cerr << "Failed to open input file: " << input_filename << "\n"; return 3; }
        inptr = &ifs;
    }

    string header;
    if (!getline(*inptr, header)) { cerr << "No input received (empty stream).\n"; return 4; }
    auto tokens = split_csv_line(header);
    if (tokens.size() < 2) { cerr << "Header parsing failed. Expected: num_nodes,num_edges\n"; return 5; }
    auto n_opt = parse_int_safe(tokens[0]);
    auto m_opt = parse_int_safe(tokens[1]);
    if (!n_opt || !m_opt) { cerr << "Header contains invalid integer(s).\n"; return 6; }
    int n = (int)*n_opt;
    int m = (int)*m_opt;
    if (n <= 0) { cerr << "Number of nodes must be positive.\n"; return 7; }
    if (m < 0) { cerr << "Number of edges cannot be negative.\n"; return 8; }

    vector<vector<int>> adj(n);
    int read_edges = 0;
    string line;
    vector<pair<int,int>> raw_edges;
    while (read_edges < m && getline(*inptr, line)) {
        trim(line);
        if (line.empty()) continue;
        if (line[0] == '#') continue;
        auto parts = split_csv_line(line);
        if (parts.size() < 2) { if (!quiet) cerr << "Skipping invalid edge line: '" << line << "'\n"; continue; }
        auto u_opt = parse_int_safe(parts[0]);
        auto v_opt = parse_int_safe(parts[1]);
        if (!u_opt || !v_opt) { if (!quiet) cerr << "Skipping non-integer line: '" << line << "'\n"; continue; }
        int u = (int)*u_opt; int v = (int)*v_opt;
        if (u < 0 || u >= n || v < 0 || v >= n) { if (!quiet) cerr << "Skipping out-of-range edge: " << u << "->" << v << "\n"; continue; }
        adj[u].push_back(v);
        raw_edges.emplace_back(u, v);
        ++read_edges;
    }
    if (read_edges < m) if (!quiet) cerr << "Warning: expected " << m << " edges but read " << read_edges << ". Proceeding.\n";

    auto t0 = chrono::high_resolution_clock::now();

    // 1) Detect a single cycle (classic DFS recursion-stack method)
    vector<int> color(n, 0), parent(n, -1);
    vector<int> cycle;
    function<bool(int)> dfs = [&](int u)->bool {
        color[u] = 1; // visiting
        for (int v : adj[u]) {
            if (color[v] == 0) {
                parent[v] = u;
                if (dfs(v)) return true;
            } else if (color[v] == 1) {
                // back-edge: reconstruct cycle from u back to v
                cycle.clear();
                cycle.push_back(v);
                for (int cur = u; cur != v && cur != -1; cur = parent[cur]) cycle.push_back(cur);
                reverse(cycle.begin(), cycle.end());
                return true;
            }
        }
        color[u] = 2; // visited
        return false;
    };
    bool found_cycle = false;
    for (int i = 0; i < n; ++i) {
        if (color[i] == 0) {
            if (dfs(i)) { found_cycle = true; break; }
        }
    }

    // 2) SCCs via Kosaraju
    vector<int> comp = kosaraju_scc(adj);
    int num_scc = 0;
    for (int x : comp) num_scc = max(num_scc, x+1);
    vector<vector<int>> scc_nodes(num_scc);
    for (int i = 0; i < n; ++i) if (comp[i] >= 0) scc_nodes[comp[i]].push_back(i);

    // 3) Optionally enumerate all simple cycles (Johnson), capped by cycle_limit
    vector<vector<int>> all_cycles;
    if (enumerate_all_cycles) {
        JohnsonSimpleCycles js(adj, cycle_limit);
        all_cycles = js.run();
    }

    // 4) If DAG, topological sort
    vector<int> topo;
    bool is_dag = !found_cycle;
    if (is_dag) {
        // Kahn's algorithm
        vector<int> indeg(n, 0);
        for (int u = 0; u < n; ++u) for (int v : adj[u]) ++indeg[v];
        queue<int> q;
        for (int i = 0; i < n; ++i) if (indeg[i] == 0) q.push(i);
        while (!q.empty()) {
            int u = q.front(); q.pop(); topo.push_back(u);
            for (int v : adj[u]) {
                if (--indeg[v] == 0) q.push(v);
            }
        }
        if ((int)topo.size() != n) is_dag = false; // unexpected, but sanity
    }

    auto t1 = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = t1 - t0;

    // Prepare output
    ostringstream out;
    out << "Deadlock Detection Report\n";
    out << "Nodes: " << n << ", edges (declared): " << m << ", edges (read): " << read_edges << "\n";
    out << "Found cycle (single detect): " << (found_cycle ? "YES" : "NO") << "\n";
    if (found_cycle) {
        out << "Cycle length: " << cycle.size() << "\n";
        out << "Cycle nodes: ";
        for (size_t i = 0; i < cycle.size(); ++i) {
            if (i) out << " -> ";
            out << cycle[i];
        }
        out << "\n";
    }
    out << "Strongly connected components: " << num_scc << "\n";
    for (int i = 0; i < num_scc; ++i) {
        out << "  SCC " << i << " (size=" << scc_nodes[i].size() << "): ";
        for (int v : scc_nodes[i]) out << v << " ";
        out << "\n";
    }

    if (enumerate_all_cycles) {
        out << "\nEnumerated cycles (limit=" << cycle_limit << ", found=" << all_cycles.size() << "):\n";
        for (size_t i = 0; i < all_cycles.size() && i < (size_t)cycle_limit; ++i) {
            auto &cy = all_cycles[i];
            out << "  C" << i << ": ";
            for (size_t j = 0; j < cy.size(); ++j) {
                if (j) out << " -> ";
                out << cy[j];
            }
            out << "\n";
        }
    }

    if (is_dag) {
        out << "\nGraph is a DAG; topological order (one possible):\n";
        for (size_t i = 0; i < topo.size(); ++i) out << topo[i] << (i+1==topo.size()? '\n' : ' ');
    } else {
        out << "\nGraph is not a DAG; topological order not available.\n";
    }

    out << "\nElapsed time (s): " << fixed << setprecision(6) << elapsed.count() << "\n";
    out << "Sample of raw edges (first 30):\n";
    for (size_t i = 0; i < raw_edges.size() && i < 30; ++i) out << raw_edges[i].first << "," << raw_edges[i].second << "\n";

    // Optionally write to a file
    if (!output_filename.empty()) {
        ofstream ofs(output_filename);
        if (!ofs) { cerr << "Failed to open output file: " << output_filename << "\n"; return 10; }
        ofs << out.str(); ofs.close(); if (!quiet) cout << "Wrote report to: " << output_filename << "\n";
    } else {
        cout << out.str();
    }

    return 0;
}

/*
SAMPLE CSV and "suitable changes":

The program expects a header: num_nodes,num_edges
Then m lines of edges: u,v

Example sample produced by --generate-sample:

num_nodes,num_edges
12,11
0,1
1,2
2,0
3,4
4,5
5,3
6,7
7,8
9,10
*/
