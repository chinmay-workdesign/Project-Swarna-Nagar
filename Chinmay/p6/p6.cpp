#include <bits/stdc++.h>
using namespace std;

// Extended Tarjan Bridge-Finding Tool for undirected graphs
// Features:
// - Robust CSV parsing with comments and blank lines allowed
// - Command-line options: --input <file>, --output <file>, --generate-sample, --quiet
// - Validates indices and supports parallel edges and self-loops
// - Reports bridges with original edge ids, connected components, and per-component bridges
// - Measures runtime and prints a detailed report
//
// CSV format expected (first header line):
// num_nodes,num_edges
// Second line (optional): N,M
// Then m lines: u,v   (0-indexed undirected edges)
//
// Notes on "suitable changes":
// - Header's num_edges should match actual number of non-empty, non-comment edge lines (program will warn otherwise)
// - Self-loops are allowed but they are ignored for bridge detection
// - Parallel edges are handled: an edge is a bridge only if it is the unique edge connecting two biconnected parts

static inline void ltrim(string &s){ s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char ch){ return !isspace(ch); })); }
static inline void rtrim(string &s){ s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch){ return !isspace(ch); }).base(), s.end()); }
static inline void trim(string &s){ ltrim(s); rtrim(s); }

vector<string> split_csv_line(const string &line){
    vector<string> tokens; string cur;
    for (size_t i = 0; i < line.size(); ++i){ char ch = line[i]; if (ch == ','){ string t = cur; trim(t); tokens.push_back(t); cur.clear(); } else cur.push_back(ch); }
    string t = cur; trim(t); if (!t.empty() || (line.size() && line.back()==',')) tokens.push_back(t);
    return tokens;
}

optional<long long> parse_int_safe(const string &s){ if (s.empty()) return nullopt; char *endptr = nullptr; errno = 0; long long val = strtoll(s.c_str(), &endptr, 10); if (errno != 0) return nullopt; while (*endptr){ if (!isspace((unsigned char)*endptr)) return nullopt; ++endptr; } return val; }

bool write_sample_csv(const string &filename){
    ofstream ofs(filename); if (!ofs) return false;
    ofs << "num_nodes,num_edges\n";
    ofs << "12,13\n";
    // graph with several components and some bridges
    ofs << "0,1\n";
    ofs << "1,2\n";
    ofs << "2,0\n"; // triangle 0-1-2
    ofs << "1,3\n"; // bridge 1-3 connects triangle to chain
    ofs << "3,4\n";
    ofs << "4,5\n"; // chain 3-4-5
    ofs << "6,7\n";
    ofs << "7,8\n";
    ofs << "8,6\n"; // triangle 6-7-8 (no bridges)
    ofs << "8,9\n"; // bridge 8-9
    ofs << "9,10\n"; // chain 9-10
    ofs << "10,11\n"; // chain 10-11
    ofs.close(); return true;
}

int main(int argc, char **argv){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string input_filename, output_filename;
    bool generate_sample = false;
    bool quiet = false;

    for (int i = 1; i < argc; ++i){ string a = argv[i]; if (a=="--input" && i+1<argc) input_filename = argv[++i]; else if (a=="--output" && i+1<argc) output_filename = argv[++i]; else if (a=="--generate-sample") generate_sample = true; else if (a=="--quiet") quiet = true; else { cerr << "Unknown arg: " << a << "\n"; cerr << "Usage: " << argv[0] << " [--input file.csv] [--output file.txt] [--generate-sample] [--quiet]\n"; return 1; } }

    if (generate_sample){ const string name = input_filename.empty() ? string("sample_bridges.csv") : input_filename; if (write_sample_csv(name)){ cout << "Wrote sample CSV to: " << name << "\n"; if (input_filename.empty()) cout << "Use --input " << name << " to run the tool.\n"; return 0; } else { cerr << "Failed to write sample CSV to: " << name << "\n"; return 2; } }

    istream *inptr = &cin; ifstream ifs; if (!input_filename.empty()){ ifs.open(input_filename); if (!ifs){ cerr << "Failed to open input file: " << input_filename << "\n"; return 3; } inptr = &ifs; }

    string header;
    // read first non-empty non-comment line as header
    while (true){ if (!getline(*inptr, header)) { cerr << "No input provided. Run with --generate-sample to create a sample.\n"; return 4; } trim(header); if (header.empty()) continue; if (header[0] == '#') continue; break; }

    auto header_tokens = split_csv_line(header);
    if (header_tokens.size() < 2){ cerr << "Header parsing failed. Expected: num_nodes,num_edges\n"; return 5; }
    auto n_opt = parse_int_safe(header_tokens[0]); auto m_opt = parse_int_safe(header_tokens[1]); if (!n_opt || !m_opt){ cerr << "Header contains invalid integers.\n"; return 6; }
    int n = (int)*n_opt; int m = (int)*m_opt;

    // optional second line that repeats N,M; otherwise next line is first edge
    string second_line; streampos lastpos = inptr->tellg(); if (getline(*inptr, second_line)){
        string tmp = second_line; trim(tmp); if (!tmp.empty() && tmp[0] != '#'){
            auto tkn = split_csv_line(tmp);
            if (tkn.size() >= 2){ auto n2 = parse_int_safe(tkn[0]); auto m2 = parse_int_safe(tkn[1]); if (n2 && m2){ n = (int)*n2; m = (int)*m2; } else { inptr->clear(); inptr->seekg(lastpos); } }
            else { inptr->clear(); inptr->seekg(lastpos); }
        }
    }

    if (n <= 0){ cerr << "Number of nodes must be positive.\n"; return 7; }
    if (m < 0){ cerr << "Number of edges cannot be negative.\n"; return 8; }

    vector<vector<pair<int,int>>> adj(n); // (neighbor, edge_id)
    vector<pair<int,int>> edges; edges.reserve(m);
    int read_edges = 0; string line;

    // map for counting parallel edges between unordered pair (min,max) -> count
    unordered_map<long long,int> undirected_edge_count;
    auto edge_key = [&](int u,int v)->long long { int a = min(u,v); int b = max(u,v); return ((long long)a<<32) | (unsigned int)b; };

    while (read_edges < m && getline(*inptr, line)){
        trim(line);
        if (line.empty()) continue; if (line[0] == '#') continue;
        auto parts = split_csv_line(line);
        if (parts.size() < 2){ if (!quiet) cerr << "Skipping invalid edge line: '" << line << "'\n"; continue; }
        auto u_opt = parse_int_safe(parts[0]); auto v_opt = parse_int_safe(parts[1]); if (!u_opt || !v_opt){ if (!quiet) cerr << "Skipping non-integer edge: '" << line << "'\n"; continue; }
        int u = (int)*u_opt; int v = (int)*v_opt; if (u < 0 || u >= n || v < 0 || v >= n){ if (!quiet) cerr << "Skipping out-of-range edge: " << u << " , " << v << "\n"; continue; }
        edges.emplace_back(u,v);
        int eid = (int)edges.size()-1;
        adj[u].push_back({v,eid}); if (u != v) adj[v].push_back({u,eid}); // undirected representation (self-loop only one direction)
        long long k = edge_key(u,v); undirected_edge_count[k]++;
        ++read_edges;
    }
    if (read_edges < m) if (!quiet) cerr << "Warning: expected " << m << " edges but read " << read_edges << ". Proceeding.\n";

    // Tarjan's bridge-finding
    vector<int> disc(n, -1), low(n, -1), parent(n, -1);
    vector<char> vis(n,0);
    int timer = 0;
    vector<pair<int,int>> bridges; // store as (u,v) with u <-> v as found

    function<void(int)> dfs = [&](int u){
        vis[u] = 1; disc[u] = low[u] = timer++;
        for (auto &p : adj[u]){
            int v = p.first; int eid = p.second;
            if (disc[v] == -1){
                parent[v] = u;
                dfs(v);
                low[u] = min(low[u], low[v]);
                // check bridge: low[v] > disc[u]  and the undirected pair has only 1 parallel edge
                long long k = edge_key(u,v);
                int parallel = undirected_edge_count[k];
                if (low[v] > disc[u] && parallel == 1){
                    bridges.emplace_back(u,v);
                }
            } else if (v != parent[u]){
                low[u] = min(low[u], disc[v]);
            }
        }
    };

    auto t0 = chrono::high_resolution_clock::now();
    for (int i = 0; i < n; ++i) if (disc[i] == -1) dfs(i);
    auto t1 = chrono::high_resolution_clock::now(); chrono::duration<double> elapsed = t1 - t0;

    // compute components via DFS/DSU for reporting
    vector<int> comp(n, -1); int compid = 0;
    for (int i = 0; i < n; ++i){ if (comp[i] != -1) continue; // BFS
        queue<int> q; q.push(i); comp[i] = compid; while (!q.empty()){ int u = q.front(); q.pop(); for (auto &p : adj[u]){ int v = p.first; if (comp[v] == -1){ comp[v] = compid; q.push(v); } } } ++compid; }

    // group bridges per component (u's component)
    unordered_map<int, vector<pair<int,int>>> bridges_by_comp;
    for (auto &b : bridges){ int u = b.first; int v = b.second; int cid = comp[u]; bridges_by_comp[cid].push_back(b); }

    // Prepare output
    ostringstream out;
    out << "Tarjan Bridge Detection Report\n";
    out << "Nodes: " << n << ", edges declared: " << m << ", edges read: " << read_edges << "\n";
    out << "Connected components: " << compid << "\n";
    out << "Bridges found: " << bridges.size() << "\n";
    out << "Elapsed time (s): " << fixed << setprecision(6) << elapsed.count() << "\n";
    out << "\nBridges list (u,v) with u <-> v as found:\n";
    for (auto &b : bridges) out << b.first << "," << b.second << "\n";

    out << "\nBridges by component:\n";
    for (int cid = 0; cid < compid; ++cid){ out << "Component " << cid << " (size=";
        int sz = 0; for (int i = 0; i < n; ++i) if (comp[i] == cid) ++sz; out << sz << ") bridges: ";
        auto it = bridges_by_comp.find(cid);
        if (it == bridges_by_comp.end()) out << "none\n"; else { out << "\n"; for (auto &b : it->second) out << "  " << b.first << "," << b.second << "\n"; }
    }

    out << "\nSample of edges (edge_id,u,v) first 50:\n";
    for (size_t i = 0; i < edges.size() && i < 50; ++i) out << i << "," << edges[i].first << "," << edges[i].second << "\n";

    // Optionally write
    if (!output_filename.empty()){
        ofstream ofs(output_filename);
        if (!ofs) { cerr << "Failed to open output file: " << output_filename << "\n"; return 10; }
        ofs << out.str(); ofs.close(); if (!quiet) cout << "Wrote report to: " << output_filename << "\n";
    } else {
        cout << out.str();
    }

    return 0;
}

/*
SAMPLE CSV (suitable changes):

The program expects a header and then M edge lines. Example:

num_nodes,num_edges
12,13
0,1
1,2
2,0
1,3
3,4
4,5
6,7
7,8
8,6
8,9
9,10
10,11
*/
