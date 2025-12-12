#include <bits/stdc++.h>
using namespace std;

// Extended Greedy Set Cover approximation tool
// - Robust CSV parsing: supports comments '#', blank lines, optional second header line
// - Command-line options: --input <file>, --output <file>, --generate-sample, --budget B, --topk, --quiet
// - Supports optional location costs and cost-aware greedy (max gain per unit cost)
// - Lazy-update max-heap with periodic rebuild to avoid degenerate cost of many updates
// - Reports coverage statistics, chosen locations, uncovered segments, and timing
// - Writes a sample CSV when requested

static inline void ltrim(string &s){ s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char ch){ return !isspace(ch); })); }
static inline void rtrim(string &s){ s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch){ return !isspace(ch); }).base(), s.end()); }
static inline void trim(string &s){ ltrim(s); rtrim(s); }

vector<string> split_csv_line(const string &line){
    vector<string> tokens; string cur;
    for (size_t i = 0; i < line.size(); ++i){ char ch = line[i]; if (ch == ','){ string t = cur; trim(t); tokens.push_back(t); cur.clear(); } else cur.push_back(ch); }
    string t = cur; trim(t); if (!t.empty() || (line.size() && line.back()==',')) tokens.push_back(t); return tokens;
}

optional<long long> parse_int_safe(const string &s){ if (s.empty()) return nullopt; char *endptr=nullptr; errno=0; long long v = strtoll(s.c_str(), &endptr, 10); if (errno!=0) return nullopt; while (*endptr){ if (!isspace((unsigned char)*endptr)) return nullopt; ++endptr; } return v; }

optional<double> parse_double_safe(const string &s){ if (s.empty()) return nullopt; char *endptr=nullptr; errno=0; double v = strtod(s.c_str(), &endptr); if (errno!=0) return nullopt; while (*endptr){ if (!isspace((unsigned char)*endptr)) return nullopt; ++endptr; } return v; }

bool write_sample_csv(const string &filename){
    ofstream ofs(filename);
    if (!ofs) return false;
    // header: num_locations,num_segments,budget
    ofs << "num_locations,num_segments,budget\n";
    ofs << "12,20,4\n";
    ofs << "# Each following line: loc_id,optional_cost,segA|segB|... (cost omitted or numeric)\n";
    ofs << "0,1.0,0|1|2|3\n";
    ofs << "1,1.5,2|3|4|5\n";
    ofs << "2,0.8,6|7\n";
    ofs << "3,2.0,0|5|9\n";
    ofs << "4,1.0,10|11|12\n";
    ofs << "5,1.2,13|14|15\n";
    ofs << "6,0.9,16|17\n";
    ofs << "7,1.3,18|19|1\n";
    ofs << "8,1.0,4|5|6\n";
    ofs << "9,1.1,7|8|9\n";
    ofs << "10,0.7,2|12|14\n";
    ofs << "11,1.0,3|11|19\n";
    ofs.close();
    return true;
}

int main(int argc, char **argv){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string input_filename, output_filename;
    bool generate_sample = false;
    bool quiet = false;
    int override_budget = -1;
    int topk = -1;
    bool cost_aware = true; // by default consider costs if present
    double rebuild_threshold = 0.2; // fraction of heap pops triggering rebuild

    for (int i = 1; i < argc; ++i){ string a = argv[i]; if (a=="--input" && i+1<argc) input_filename = argv[++i]; else if (a=="--output" && i+1<argc) output_filename = argv[++i]; else if (a=="--generate-sample") generate_sample = true; else if (a=="--quiet") quiet = true; else if (a=="--budget" && i+1<argc) override_budget = atoi(argv[++i]); else if (a=="--topk" && i+1<argc) topk = atoi(argv[++i]); else if (a=="--nocost") cost_aware = false; else if (a=="--rebuild-threshold" && i+1<argc) rebuild_threshold = atof(argv[++i]); else { cerr << "Unknown arg: " << a << "\n"; cerr << "Usage: " << argv[0] << " [--input file.csv] [--output file.txt] [--generate-sample] [--budget B] [--topk K] [--nocost] [--quiet]\n"; return 1; } }

    if (generate_sample){ const string name = input_filename.empty() ? string("sample_setcover.csv") : input_filename; if (write_sample_csv(name)){ cout << "Wrote sample CSV to: " << name << "\n"; if (input_filename.empty()) cout << "Use --input " << name << " to run the solver on it.\n"; return 0; } else { cerr << "Failed to write sample CSV to: " << name << "\n"; return 2; } }

    istream *inptr = &cin; ifstream ifs; if (!input_filename.empty()){ ifs.open(input_filename); if (!ifs){ cerr << "Failed to open input file: " << input_filename << "\n"; return 3; } inptr = &ifs; }

    // read first non-empty non-comment line as header
    string header;
    while (true){ if (!getline(*inptr, header)){ cerr << "No input provided. Use --generate-sample to create one.\n"; return 4; } trim(header); if (header.empty()) continue; if (header[0]=='#') continue; break; }

    auto header_tokens = split_csv_line(header);
    if (header_tokens.size() < 3){ cerr << "Header parse failed. Expected: num_locations,num_segments,budget\n"; return 5; }
    auto Nopt = parse_int_safe(header_tokens[0]); auto Mopt = parse_int_safe(header_tokens[1]); auto Bopt = parse_int_safe(header_tokens[2]);
    if (!Nopt || !Mopt || !Bopt){ // maybe header is literal; read next non-comment line
        string line;
        bool found = false;
        while (getline(*inptr, line)){
            trim(line); if (line.empty()) continue; if (line[0]=='#') continue;
            auto toks = split_csv_line(line);
            if (toks.size() >= 3){ Nopt = parse_int_safe(toks[0]); Mopt = parse_int_safe(toks[1]); Bopt = parse_int_safe(toks[2]); found = true; break; }
        }
        if (!found || !Nopt || !Mopt || !Bopt){ cerr << "Failed to parse N,M,B from header/second line.\n"; return 6; }
    }

    int N = (int)*Nopt; int M = (int)*Mopt; int B = (int)*Bopt;
    if (override_budget > 0) B = override_budget;
    if (N < 0 || M < 0 || B < 0){ cerr << "Invalid N/M/B.\n"; return 7; }

    vector<vector<int>> covers(N);
    vector<double> cost(N, 1.0); // default cost 1.0
    string line;
    int read_locs = 0;
    // read up to N lines describing locations
    while (read_locs < N && getline(*inptr, line)){
        trim(line); if (line.empty()) continue; if (line[0]=='#') continue;
        // format: loc_id,optional_cost,seg|seg|...
        // or loc_id,seg|seg|...  (no cost)
        // allow extra whitespace
        auto parts = split_csv_line(line);
        if (parts.size() < 2){ if (!quiet) cerr << "Skipping invalid location line: '" << line << "'\n"; continue; }
        auto id_opt = parse_int_safe(parts[0]); if (!id_opt){ if (!quiet) cerr << "Skipping line with invalid loc id: '" << line << "'\n"; continue; }
        int loc = (int)*id_opt;
        if (loc < 0 || loc >= N){ if (!quiet) cerr << "Skipping out-of-range loc: " << loc << "\n"; continue; }
        size_t segs_idx = 1;
        // if second token is a double, treat as cost
        auto maybe_cost = parse_double_safe(parts[1]);
        if (maybe_cost){ cost[loc] = *maybe_cost; segs_idx = 2; }
        if (parts.size() <= segs_idx){ if (!quiet) cerr << "No segments for loc " << loc << "\n"; ++read_locs; continue; }
        // segments string may be 's|s|s' so split by '|'
        string seglist = parts[segs_idx];
        vector<int> segs;
        string token;
        stringstream ss(seglist);
        while (getline(ss, token, '|')){
            trim(token); if (token.empty()) continue; auto sopt = parse_int_safe(token); if (!sopt) continue; int s = (int)*sopt; if (s >= 0 && s < M) segs.push_back(s);
        }
        sort(segs.begin(), segs.end()); segs.erase(unique(segs.begin(), segs.end()), segs.end());
        covers[loc] = move(segs);
        ++read_locs;
    }
    if (read_locs < N) if (!quiet) cerr << "Warning: expected " << N << " locations but read " << read_locs << ".\n";

    // Core greedy selection: maintain covered flag and estimate gains
    vector<char> covered(M, false);
    int covered_count = 0;
    vector<int> chosen;
    chosen.reserve(B);
    vector<int> est_gain(N,0);

    for (int i = 0; i < N; ++i) est_gain[i] = (int)covers[i].size();

    // Heap entries: for cost-aware use (gain/cost, gain, loc) otherwise (gain, loc)
    struct HeapEntry { double key; int gain; int loc; };
    struct Cmp { bool operator()(const HeapEntry &a, const HeapEntry &b) const { if (a.key != b.key) return a.key < b.key; if (a.gain != b.gain) return a.gain < b.gain; return a.loc > b.loc; } };

    priority_queue<HeapEntry, vector<HeapEntry>, Cmp> pq;
    for (int i = 0; i < N; ++i){ double k = cost_aware ? (est_gain[i] / max(1e-9, cost[i])) : est_gain[i]; pq.push({k, est_gain[i], i}); }

    auto compute_true_gain = [&](int loc)->int{ int g=0; for (int s: covers[loc]) if (!covered[s]) ++g; return g; };

    int pops_since_rebuild = 0; int total_pops = 0;
    const int MAX_REBUILD_ITER = max(1000, N/2);

    while ((int)chosen.size() < B && covered_count < M && !pq.empty()){
        auto top = pq.top(); pq.pop(); ++pops_since_rebuild; ++total_pops;
        int loc = top.loc;
        int tg = compute_true_gain(loc);
        if (tg == 0) {
            // skip; nothing new
            if (pops_since_rebuild > MAX_REBUILD_ITER || (double)pops_since_rebuild / max(1,total_pops) > rebuild_threshold){
                // rebuild heap to drop stale entries
                priority_queue<HeapEntry, vector<HeapEntry>, Cmp> newpq;
                for (int i = 0; i < N; ++i) if (!covered.empty()){
                    int g = compute_true_gain(i); if (g <= 0) continue; double k = cost_aware ? (g / max(1e-9, cost[i])) : g; newpq.push({k, g, i}); }
                pq.swap(newpq); pops_since_rebuild = 0; total_pops = 0;
            }
            continue;
        }
        double newkey = cost_aware ? (tg / max(1e-9, cost[loc])) : tg;
        if (abs(newkey - top.key) > 1e-12 || tg != top.gain){
            // lazy update: push updated entry
            pq.push({newkey, tg, loc});
            continue;
        }
        // accept loc
        chosen.push_back(loc);
        for (int s: covers[loc]){
            if (!covered[s]){ covered[s] = 1; ++covered_count; }
        }
        // Optionally periodically rebuild to remove many stale entries
        if (pops_since_rebuild > MAX_REBUILD_ITER){
            priority_queue<HeapEntry, vector<HeapEntry>, Cmp> newpq;
            for (int i = 0; i < N; ++i){ int g = compute_true_gain(i); if (g <= 0) continue; double k = cost_aware ? (g / max(1e-9, cost[i])) : g; newpq.push({k, g, i}); }
            pq.swap(newpq); pops_since_rebuild = 0; total_pops = 0;
        }
    }

    // Prepare uncovered segments list
    vector<int> uncovered_segments;
    for (int s = 0; s < M; ++s) if (!covered[s]) uncovered_segments.push_back(s);

    // Build output
    ostringstream out;
    out << "Set Cover Greedy Report\n";
    out << "Locations declared: " << N << ", Segments: " << M << ", Budget: " << B << "\n";
    out << "Locations read: " << read_locs << "\n";
    out << "Chosen locations: " << chosen.size() << "\n";
    out << "Covered segments: " << covered_count << " / " << M << "\n";
    out << "Coverage fraction: " << fixed << setprecision(4) << (M==0?0.0: (double)covered_count / M) << "\n";
    out << "Cost-aware mode: " << (cost_aware?"ON":"OFF") << "\n";
    out << "Top chosen locations (up to 100):\n";
    for (size_t i = 0; i < chosen.size() && i < 100; ++i) out << chosen[i] << (i+1==chosen.size()? '\n' : ',');
    out << "\nUncovered segments count: " << uncovered_segments.size() << "\n";
    if (!uncovered_segments.empty()){
        out << "Uncovered segments (first 200):\n";
        for (size_t i = 0; i < uncovered_segments.size() && i < 200; ++i) out << uncovered_segments[i] << (i+1==uncovered_segments.size()? '\n' : ',');
        out << "\n";
    }

    // Optionally output in CSV-like format: chosen per-line
    out << "\nChosenLocationsList\n";
    for (int x : chosen) out << x << "\n";

    // If user requested topk, also output suggested additional candidates by true gain
    if (topk > 0){
        vector<pair<int,int>> cand; // (true_gain, loc)
        for (int i = 0; i < N; ++i){ int g = 0; for (int s: covers[i]) if (!covered[s]) ++g; if (g>0) cand.emplace_back(g, i); }
        sort(cand.begin(), cand.end(), greater<>());
        out << "\nTop " << topk << " candidate locations by marginal gain (after chosen):\n";
        for (int i = 0; i < (int)cand.size() && i < topk; ++i) out << cand[i].second << ",gain=" << cand[i].first << "\n";
    }

    // Write to file or stdout
    if (!output_filename.empty()){
        ofstream ofs(output_filename);
        if (!ofs){ cerr << "Failed to open output file: " << output_filename << "\n"; return 11; }
        ofs << out.str(); ofs.close(); if (!quiet) cout << "Wrote report to: " << output_filename << "\n";
    } else {
        cout << out.str();
    }

    return 0;
}

/*
SAMPLE CSV (suitable changes):

Header:
num_locations,num_segments,budget

Example (generated by --generate-sample in this file):

num_locations,num_segments,budget
12,20,4
# Each following line: loc_id,optional_cost,segA|segB|... (cost omitted or numeric)
0,1.0,0|1|2|3
1,1.5,2|3|4|5
2,0.8,6|7
3,2.0,0|5|9
4,1.0,10|11|12
5,1.2,13|14|15
6,0.9,16|17
7,1.3,18|19|1
8,1.0,4|5|6
9,1.1,7|8|9
10,0.7,2|12|14
11,1.0,3|11|19
*/
