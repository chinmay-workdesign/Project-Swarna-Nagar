#include <bits/stdc++.h>
using namespace std;

// Extended DSU Connectivity Tool
// Features:
// - Robust CSV parsing (comments, blank lines, flexible separators)
// - Command-line options: --input <file>, --output <file>, --generate-sample, --quiet
// - Supports three query types in CSV: CONNECTED, COMPONENT_SIZE, MERGE
// - Reports final number of components, largest component size, and timing
// - Optionally prints per-query timing and debug info
//
// CSV format (recommended):
// Header (first non-comment, non-empty line): num_nodes,num_edges,num_queries
// Second line (optional repeat): N,M,Q   (if present, it will be parsed and used)
// Next M lines: u,v              (initial undirected edges)
// Next Q lines: <type>,a,b       (query lines; type is one of: CONNECTED, COMPONENT_SIZE, MERGE)
//   - CONNECTED,a,b   => prints CONNECTED or DISCONNECTED
//   - COMPONENT_SIZE,a,0 => prints size of component containing a (b is ignored)
//   - MERGE,a,b       => perform union(a,b) and optionally print status
// Comments may be added using '#' at start of a line. Blank lines are ignored.

struct DSU {
    int n;
    vector<int> parent, sz;
    int components;
    DSU(int n=0): n(n), parent(n), sz(n,1), components(n) {
        iota(parent.begin(), parent.end(), 0);
    }
    void reset(int n_) {
        n = n_;
        parent.assign(n,0);
        iota(parent.begin(), parent.end(), 0);
        sz.assign(n,1);
        components = n;
    }
    int find(int x){
        // iterative path compression (safer on deep recursion)
        int root = x;
        while (parent[root] != root) root = parent[root];
        while (parent[x] != x) {
            int nxt = parent[x];
            parent[x] = root;
            x = nxt;
        }
        return root;
    }
    bool unite(int a,int b){
        a=find(a); b=find(b);
        if (a==b) return false;
        if (sz[a]<sz[b]) swap(a,b);
        parent[b]=a;
        sz[a]+=sz[b];
        --components;
        return true;
    }
    bool same(int a,int b){ return find(a)==find(b); }
    int size(int a){ return sz[find(a)]; }
};

// small CSV utilities
static inline void ltrim(string &s){ s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char ch){ return !isspace(ch); })); }
static inline void rtrim(string &s){ s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch){ return !isspace(ch); }).base(), s.end()); }
static inline void trim(string &s){ ltrim(s); rtrim(s); }

vector<string> split_csv_line(const string &line) {
    vector<string> tokens;
    string cur;
    for (size_t i=0;i<line.size();++i) {
        char ch=line[i];
        if (ch == ',') { string t = cur; trim(t); tokens.push_back(t); cur.clear(); }
        else cur.push_back(ch);
    }
    string t = cur; trim(t);
    if (!t.empty() || (!line.empty()&&line.back()==',')) tokens.push_back(t);
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
    // sample: 12 nodes, 7 initial edges, 9 queries demonstrating types
    ofs << "num_nodes,num_edges,num_queries\n";
    ofs << "12,7,9\n";
    ofs << "# initial edges (undirected)\n";
    ofs << "0,1\n";
    ofs << "1,2\n";
    ofs << "2,3\n";
    ofs << "4,5\n";
    ofs << "5,6\n";
    ofs << "7,8\n";
    ofs << "9,10\n";
    ofs << "# queries: type,a,b\n";
    ofs << "CONNECTED,0,3\n"; // yes
    ofs << "CONNECTED,0,4\n"; // no
    ofs << "COMPONENT_SIZE,1,0\n"; // size of component containing 1
    ofs << "MERGE,3,4\n"; // merge two components
    ofs << "CONNECTED,0,5\n"; // now yes after merge
    ofs << "MERGE,8,9\n";
    ofs << "COMPONENT_SIZE,8,0\n";
    ofs << "MERGE,11,0\n"; // merge singleton node 11 into big comp
    ofs << "COMPONENT_SIZE,0,0\n";
    ofs.close();
    return true;
}

int main(int argc, char **argv){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string input_filename, output_filename;
    bool generate_sample = false;
    bool quiet = false;
    bool verbose_queries = false;

    for (int i=1;i<argc;++i){
        string a = argv[i];
        if (a=="--input" && i+1<argc) input_filename = argv[++i];
        else if (a=="--output" && i+1<argc) output_filename = argv[++i];
        else if (a=="--generate-sample") generate_sample = true;
        else if (a=="--quiet") quiet = true;
        else if (a=="--verbose-queries") verbose_queries = true;
        else { cerr << "Unknown arg: " << a << "\n"; cerr << "Usage: " << argv[0] << " [--input file.csv] [--output file.txt] [--generate-sample] [--quiet] [--verbose-queries]\n"; return 1; }
    }

    if (generate_sample) {
        const string sample_name = input_filename.empty() ? "sample_dsu.csv" : input_filename;
        if (write_sample_csv(sample_name)) {
            cout << "Wrote sample CSV to: " << sample_name << "\n";
            if (input_filename.empty()) cout << "Use --input " << sample_name << " to run the tool on it.\n";
            return 0;
        } else { cerr << "Failed to write sample CSV to: " << sample_name << "\n"; return 2; }
    }

    istream *inptr = &cin;
    ifstream ifs;
    if (!input_filename.empty()){
        ifs.open(input_filename);
        if (!ifs) { cerr << "Failed to open input file: " << input_filename << "\n"; return 3; }
        inptr = &ifs;
    }

    string header;
    // read first non-empty, non-comment line as header
    while (true) {
        if (!getline(*inptr, header)) { cerr << "No input provided. Use --generate-sample to create one.\n"; return 4; }
        trim(header);
        if (header.empty()) continue;
        if (header[0] == '#') continue;
        break;
    }

    auto header_tokens = split_csv_line(header);
    if (header_tokens.size() < 3) { cerr << "Header parsing failed. Expected: num_nodes,num_edges,num_queries\n"; return 5; }
    auto n_opt = parse_int_safe(header_tokens[0]);
    auto m_opt = parse_int_safe(header_tokens[1]);
    auto q_opt = parse_int_safe(header_tokens[2]);
    if (!n_opt || !m_opt || !q_opt) { cerr << "Header contains invalid integer(s).\n"; return 6; }
    int n = (int)*n_opt; int m = (int)*m_opt; int q = (int)*q_opt;

    // allow optional second line repeating N,M,Q
    string second_line;
    streampos last_pos = inptr->tellg();
    if (getline(*inptr, second_line)){
        string tmp = second_line; trim(tmp);
        auto tokens2 = split_csv_line(tmp);
        if (tokens2.size() >= 3) {
            auto n2 = parse_int_safe(tokens2[0]);
            auto m2 = parse_int_safe(tokens2[1]);
            auto q2 = parse_int_safe(tokens2[2]);
            if (n2 && m2 && q2) {
                // override if present
                n = (int)*n2; m = (int)*m2; q = (int)*q2;
            } else {
                // this line is likely the first edge; rewind
                inptr->clear();
                inptr->seekg(last_pos);
            }
        } else {
            inptr->clear();
            inptr->seekg(last_pos);
        }
    }

    if (n <= 0) { cerr << "Number of nodes must be positive.\n"; return 7; }
    if (m < 0) { cerr << "Number of edges cannot be negative.\n"; return 8; }
    if (q < 0) { cerr << "Number of queries cannot be negative.\n"; return 9; }

    DSU dsu(n);

    int read_edges = 0;
    string line;
    vector<pair<int,int>> edges;
    // read m edges
    while (read_edges < m && getline(*inptr, line)) {
        trim(line);
        if (line.empty()) continue;
        if (line[0] == '#') continue;
        auto parts = split_csv_line(line);
        if (parts.size() < 2) { if (!quiet) cerr << "Skipping invalid edge line: '" << line << "'\n"; continue; }
        auto u_opt = parse_int_safe(parts[0]);
        auto v_opt = parse_int_safe(parts[1]);
        if (!u_opt || !v_opt) { if (!quiet) cerr << "Skipping non-integer edge line: '" << line << "'\n"; continue; }
        int u = (int)*u_opt; int v = (int)*v_opt;
        if (u < 0 || u >= n || v < 0 || v >= n) { if (!quiet) cerr << "Skipping out-of-range edge: " << u << " - " << v << "\n"; continue; }
        edges.emplace_back(u,v);
        dsu.unite(u,v);
        ++read_edges;
    }
    if (read_edges < m) if (!quiet) cerr << "Warning: expected " << m << " edges but read " << read_edges << ". Proceeding.\n";

    // process queries
    int read_queries = 0;
    vector<string> query_results;
    query_results.reserve(q+4);
    auto t0 = chrono::high_resolution_clock::now();

    for (int i=0;i<q && getline(*inptr, line); ++i) {
        trim(line);
        if (line.empty()) { --i; continue; }
        if (line[0] == '#') { --i; continue; }
        auto parts = split_csv_line(line);
        if (parts.empty()) { if (!quiet) cerr << "Skipping empty query line.\n"; continue; }
        string qtype = parts[0];
        for (auto &ch : qtype) ch = toupper((unsigned char)ch);
        if (qtype == "CONNECTED") {
            if (parts.size() < 3) { query_results.push_back("DISCONNECTED"); ++read_queries; continue; }
            auto a_opt = parse_int_safe(parts[1]); auto b_opt = parse_int_safe(parts[2]);
            if (!a_opt || !b_opt) { query_results.push_back("DISCONNECTED"); ++read_queries; continue; }
            int a = (int)*a_opt; int b = (int)*b_opt;
            if (a<0||a>=n||b<0||b>=n) { query_results.push_back("DISCONNECTED"); ++read_queries; continue; }
            bool same = dsu.same(a,b);
            query_results.push_back(same ? string("CONNECTED") : string("DISCONNECTED"));
            if (verbose_queries && !quiet) cerr << "Query CONNECTED " << a << "," << b << " => " << query_results.back() << "\n";
        } else if (qtype == "COMPONENT_SIZE") {
            if (parts.size() < 2) { query_results.push_back("0"); ++read_queries; continue; }
            auto a_opt = parse_int_safe(parts[1]);
            if (!a_opt) { query_results.push_back("0"); ++read_queries; continue; }
            int a = (int)*a_opt;
            if (a<0||a>=n) { query_results.push_back("0"); ++read_queries; continue; }
            int s = dsu.size(a);
            query_results.push_back(to_string(s));
            if (verbose_queries && !quiet) cerr << "Query COMPONENT_SIZE " << a << " => " << s << "\n";
        } else if (qtype == "MERGE" || qtype == "UNION") {
            if (parts.size() < 3) { query_results.push_back("IGNORED"); ++read_queries; continue; }
            auto a_opt = parse_int_safe(parts[1]); auto b_opt = parse_int_safe(parts[2]);
            if (!a_opt || !b_opt) { query_results.push_back("IGNORED"); ++read_queries; continue; }
            int a = (int)*a_opt; int b = (int)*b_opt;
            if (a<0||a>=n||b<0||b>=n) { query_results.push_back("IGNORED"); ++read_queries; continue; }
            bool changed = dsu.unite(a,b);
            query_results.push_back(changed ? string("MERGED") : string("ALREADY_CONNECTED"));
            if (verbose_queries && !quiet) cerr << "Query MERGE " << a << "," << b << " => " << query_results.back() << "\n";
        } else {
            // try to interpret as two integers (legacy format CONNECTED)
            if (parts.size() >= 2) {
                auto a_opt = parse_int_safe(parts[0]); auto b_opt = parse_int_safe(parts[1]);
                if (a_opt && b_opt) {
                    int a = (int)*a_opt; int b = (int)*b_opt;
                    if (a<0||a>=n||b<0||b>=n) { query_results.push_back("DISCONNECTED"); }
                    else { query_results.push_back(dsu.same(a,b) ? string("CONNECTED") : string("DISCONNECTED")); }
                    if (verbose_queries && !quiet) cerr << "Legacy query " << a << "," << b << " => " << query_results.back() << "\n";
                } else {
                    if (!quiet) cerr << "Unknown query type or malformed line: '" << line << "'\n";
                    query_results.push_back(string("IGNORED"));
                }
            } else { if (!quiet) cerr << "Unknown query type: '" << line << "'\n"; query_results.push_back(string("IGNORED")); }
        }
        ++read_queries;
    }

    auto t1 = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = t1 - t0;

    // Prepare summary
    ostringstream out;
    out << "DSU Connectivity Report\n";
    out << "Nodes: " << n << ", initial edges read: " << read_edges << ", queries processed: " << read_queries << "\n";
    out << "Final components: " << dsu.components << "\n";
    int maxsz = 0; for (int i=0;i<n;++i) if (dsu.find(i)==i) maxsz = max(maxsz, dsu.sz[i]);
    out << "Largest component size: " << maxsz << "\n";
    out << "Elapsed query processing time (s): " << fixed << setprecision(6) << elapsed.count() << "\n";

    out << "\nQuery results (in order):\n";
    for (auto &r : query_results) out << r << "\n";

    out << "\nSample of initial edges (first 50):\n";
    for (size_t i=0;i<edges.size() && i<50;++i) out << edges[i].first << "," << edges[i].second << "\n";

    // write to output file or stdout
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

The program expects a header and then M edge lines followed by Q query lines. Example:

num_nodes,num_edges,num_queries
12,7,9
# initial edges (undirected)
0,1
1,2
2,3
4,5
5,6
7,8
9,10
# queries: type,a,b
CONNECTED,0,3
CONNECTED,0,4
COMPONENT_SIZE,1,0
MERGE,3,4
CONNECTED,0,5
MERGE,8,9
COMPONENT_SIZE,8,0
MERGE,11,0
COMPONENT_SIZE,0,0

*/
