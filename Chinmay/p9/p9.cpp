#include <bits/stdc++.h>
using namespace std;

// Sliding Window Average Calculator — Extended
// Features added:
//  - Robust CSV parsing with comments and blank lines
//  - Command-line options: --input <file>, --output <file>, --generate-sample, --quiet
//  - Supports two window modes: fixed-count (K most recent readings) and time-based (last T seconds)
//  - Optional weighted average using timestamps (exponential decay) via --decay alpha
//  - Reports summary statistics (min, max, mean of averages) and timing
//  - Handles ISO-like timestamps (non-comma strings) or integer epoch seconds
//  - Produces CSV output header and optionally rank of averages
//
// CSV input format (basic):
// First non-comment line (header): num_readings,window_size_or_timespan
// Second non-comment line (optional repeat): N,K  (if using count window) OR N,T (if using time window indicator)
// Next N lines: timestamp,value
// timestamp may be an integer (epoch seconds) or an ISO-like string without commas (e.g., 2023-07-01T12:00:00)
//
// Example (count-based K=3):
// num_readings,window_size
// 5,3
// 0,10.0
// 1,20.0
// 2,30.0
// 3,40.0
// 4,50.0
//
// Example (time-based T=60 seconds):
// num_readings,window_size
// 6,60
// 169...,15.0
// 169...,20.0
// ...

static inline void ltrim(string &s){ s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char ch){ return !isspace(ch); })); }
static inline void rtrim(string &s){ s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch){ return !isspace(ch); }).base(), s.end()); }
static inline void trim(string &s){ ltrim(s); rtrim(s); }

vector<string> split_csv_line(const string &line){
    vector<string> tokens; string cur;
    for (size_t i = 0;i<line.size();++i){ char ch = line[i]; if (ch == ','){ string t = cur; trim(t); tokens.push_back(t); cur.clear(); } else cur.push_back(ch); }
    string t = cur; trim(t); if (!t.empty() || (!line.empty() && line.back()==',')) tokens.push_back(t); return tokens;
}

optional<long long> parse_int_safe(const string &s){ if (s.empty()) return nullopt; char *endptr=nullptr; errno=0; long long v=strtoll(s.c_str(), &endptr, 10); if (errno!=0) return nullopt; while (*endptr){ if (!isspace((unsigned char)*endptr)) return nullopt; ++endptr; } return v; }

optional<double> parse_double_safe(const string &s){ if (s.empty()) return nullopt; char *endptr=nullptr; errno=0; double v=strtod(s.c_str(), &endptr); if (errno!=0) return nullopt; while (*endptr){ if (!isspace((unsigned char)*endptr)) return nullopt; ++endptr; } return v; }

bool write_sample_csv(const string &filename){
    ofstream ofs(filename); if (!ofs) return false;
    ofs << "num_readings,window_size\n";
    ofs << "15,5\n"; // 15 readings, window size 5 (count-based)
    // timestamps as integer seconds for simplicity
    long long t0 = 1700000000LL; // arbitrary epoch
    vector<double> vals = {10,12,11,13,20,21,19,18,25,30,28,27,26,40,35};
    for (size_t i=0;i<vals.size();++i) ofs << (t0 + (int)i*30) << "," << vals[i] << "\n";
    ofs.close(); return true;
}

// Try to interpret timestamp string as integer seconds; return optional<long long>
optional<long long> timestamp_to_epoch(const string &s){
    // If the token is purely digits (maybe with leading + or -) parse as integer
    auto t = parse_int_safe(s);
    if (t) return *t;
    // Could implement ISO8601 parsing here if needed. For now, return nullopt to indicate non-numeric timestamp
    return nullopt;
}

int main(int argc, char **argv){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string input_filename, output_filename;
    bool generate_sample = false;
    bool quiet = false;
    bool time_based = false; // if true, second header number interpreted as seconds timespan
    bool decay_mode = false; double decay_alpha = 0.0; // exponential decay weight: weight = exp(-alpha * (t_now - t_i))
    int top_k = -1; // output top K averages

    for (int i=1;i<argc;++i){ string a = argv[i]; if (a=="--input" && i+1<argc) input_filename = argv[++i]; else if (a=="--output" && i+1<argc) output_filename = argv[++i]; else if (a=="--generate-sample") generate_sample = true; else if (a=="--quiet") quiet = true; else if (a=="--time-window") time_based = true; else if (a=="--decay" && i+1<argc){ decay_mode = true; decay_alpha = atof(argv[++i]); } else if (a=="--top" && i+1<argc){ top_k = atoi(argv[++i]); } else { cerr << "Unknown arg: " << a << "\n"; cerr << "Usage: " << argv[0] << " [--input file.csv] [--output out.csv] [--generate-sample] [--time-window] [--decay alpha] [--top K] [--quiet]\n"; return 1; } }

    if (generate_sample){ const string name = input_filename.empty() ? string("sample_sliding.csv") : input_filename; if (write_sample_csv(name)){ cout << "Wrote sample CSV to: " << name << "\n"; if (input_filename.empty()) cout << "Use --input " << name << " to run the program.\n"; return 0;} else { cerr << "Failed to write sample CSV to: " << name << "\n"; return 2; } }

    istream *inptr = &cin; ifstream ifs; if (!input_filename.empty()){ ifs.open(input_filename); if (!ifs){ cerr << "Failed to open input file: " << input_filename << "\n"; return 3; } inptr = &ifs; }

    // Read first non-comment non-empty line as header
    string header;
    while (true){ if (!getline(*inptr, header)){ cerr << "No input provided. Use --generate-sample to create one.\n"; return 4; } trim(header); if (header.empty()) continue; if (header[0]=='#') continue; break; }

    auto header_tokens = split_csv_line(header);
    if (header_tokens.size() < 2){ cerr << "Header parsing failed. Expected: num_readings,window_size\n"; return 5; }
    auto N_opt = parse_int_safe(header_tokens[0]); auto W_opt = parse_int_safe(header_tokens[1]);
    if (!N_opt || !W_opt){ // header might be literal like "num_readings,window_size" so read next line
        string second_line; while (true){ if (!getline(*inptr, second_line)){ cerr << "Missing N,K line.\n"; return 6; } trim(second_line); if (second_line.empty()) continue; if (second_line[0]=='#') continue; break; }
        auto tokens2 = split_csv_line(second_line);
        if (tokens2.size() < 2){ cerr << "Second line parsing failed. Expected: N,K\n"; return 7; }
        N_opt = parse_int_safe(tokens2[0]); W_opt = parse_int_safe(tokens2[1]);
        if (!N_opt || !W_opt) { cerr << "Could not parse N or K from header/second line.\n"; return 8; }
    }

    int N = (int)*N_opt; long long W = *W_opt; // W interpreted as K if count-based, or seconds if time-based
    if (N < 0) { cerr << "Invalid N: " << N << "\n"; return 9; }
    if (W <= 0) { cerr << "Invalid window size/timespan: " << W << "\n"; return 10; }

    vector<string> timestamps; timestamps.reserve(max(0,N));
    vector<double> values; values.reserve(max(0,N));
    vector<long long> epochs; epochs.reserve(max(0,N)); // epoch seconds when numeric; otherwise -1

    string line;
    int read_count = 0;

    while (read_count < N && getline(*inptr, line)){
        trim(line);
        if (line.empty()) continue;
        if (line[0] == '#') continue;
        // split by first comma
        auto pos = line.find(',');
        if (pos == string::npos){ if (!quiet) cerr << "Skipping malformed line: '" << line << "'\n"; continue; }
        string ts = line.substr(0, pos); string valstr = line.substr(pos+1);
        trim(ts); trim(valstr);
        auto vopt = parse_double_safe(valstr);
        if (!vopt){ if (!quiet) cerr << "Skipping line with invalid value: '" << line << "'\n"; continue; }
        double v = *vopt;
        timestamps.push_back(ts);
        values.push_back(v);
        auto ep = timestamp_to_epoch(ts);
        if (ep) epochs.push_back(*ep); else epochs.push_back(-1);
        ++read_count;
    }

    if (read_count < N) if (!quiet) cerr << "Warning: expected " << N << " readings but read " << read_count << ". Proceeding.\n";

    // Choose mode: if time_based is true, window is W seconds; else window is count K=W
    int K = (int)W;
    long long T_seconds = (long long)W;

    // If decay_mode true, we compute weighted average using weights w_i = exp(-alpha * (t_now - t_i))
    // For count-based windows with non-numeric timestamps, decay uses index distance: w = exp(-alpha * idx_diff)

    // Prepare outputs: vector of (timestamp_of_current, average)
    vector<pair<string,double>> outputs;
    outputs.reserve(max(0, (int)values.size()));

    // For count-based simple sliding average: use queue/deque and running sum
    if (!time_based && !decay_mode){
        deque<double> window;
        double running = 0.0;
        for (int i=0;i<(int)values.size();++i){
            window.push_back(values[i]); running += values[i];
            if ((int)window.size() > K){ running -= window.front(); window.pop_front(); }
            if ((int)window.size() == K){ double avg = running / K; outputs.emplace_back(timestamps[i], avg); }
        }
    } else if (!time_based && decay_mode){
        // count-based decay: weights by index distance from current reading
        for (int i=0;i<(int)values.size();++i){
            double wsum = 0.0; double vwsum = 0.0;
            int start = max(0, i - K + 1);
            for (int j = start; j <= i; ++j){ double d = (double)(i - j); double w = exp(-decay_alpha * d); wsum += w; vwsum += w * values[j]; }
            double avg = vwsum / (wsum + 1e-18);
            if (i - start + 1 == K) outputs.emplace_back(timestamps[i], avg); // only output when full count window
        }
    } else if (time_based && !decay_mode){
        // time-based simple sliding average: include values in [t_now - T_seconds, t_now]
        // if timestamps are non-numeric for any reading, we fall back to index-based window of size K
        bool all_numeric = true; for (auto e : epochs) if (e < 0) { all_numeric = false; break; }
        if (!all_numeric){ if (!quiet) cerr << "Time-window requested but timestamps are not numeric; falling back to count-window of size K=" << K << "\n"; // treat as count-based
            deque<double> window; double running = 0.0; for (int i=0;i<(int)values.size();++i){ window.push_back(values[i]); running+=values[i]; if ((int)window.size() > K){ running -= window.front(); window.pop_front(); } if ((int)window.size() == K) outputs.emplace_back(timestamps[i], running / K); }
        } else {
            deque<int> idxdq; double running=0.0; for (int i=0;i<(int)values.size();++i){ long long tnow = epochs[i]; // push i
                idxdq.push_back(i); running += values[i]; while (!idxdq.empty() && epochs[idxdq.front()] < tnow - T_seconds){ running -= values[idxdq.front()]; idxdq.pop_front(); }
                if (!idxdq.empty()){
                    // only output when we have at least one value in window; if you want only "full" windows, check condition
                    outputs.emplace_back(timestamps[i], running / (double)idxdq.size());
                }
            }
        }
    } else { // time_based && decay_mode
        bool all_numeric = true; for (auto e : epochs) if (e < 0) { all_numeric = false; break; }
        if (!all_numeric){ if (!quiet) cerr << "Time-window + decay requested but timestamps are not numeric; falling back to index-decay with K=" << K << "\n"; for (int i=0;i<(int)values.size();++i){ double wsum=0.0,vwsum=0.0; int start=max(0,i-K+1); for (int j=start;j<=i;++j){ double d=(double)(i-j); double w=exp(-decay_alpha*d); wsum+=w; vwsum+=w*values[j]; } if (i-start+1==K) outputs.emplace_back(timestamps[i], vwsum/(wsum+1e-18)); } }
        else {
            deque<int> idxdq; for (int i=0;i<(int)values.size();++i){ long long tnow=epochs[i]; idxdq.push_back(i); // remove older than tnow - T_seconds
                while (!idxdq.empty() && epochs[idxdq.front()] < tnow - T_seconds) idxdq.pop_front(); double wsum=0.0,vwsum=0.0; for (int idx : idxdq){ double dt = (double)(tnow - epochs[idx]); double w = exp(-decay_alpha * dt); wsum += w; vwsum += w * values[idx]; } if (!idxdq.empty()) outputs.emplace_back(timestamps[i], vwsum / (wsum + 1e-18)); }
        }
    }

    // Output CSV header
    vector<string> out_lines;
    out_lines.reserve(outputs.size() + 10);
    out_lines.push_back(string("timestamp,avg_over_window"));
    for (auto &p : outputs){ ostringstream oss; oss << p.first << "," << fixed << setprecision(6) << p.second; out_lines.push_back(oss.str()); }

    // Summary statistics of averages
    vector<double> avgs; avgs.reserve(outputs.size()); for (auto &p : outputs) avgs.push_back(p.second);
    double avg_min = avgs.empty() ? 0.0 : *min_element(avgs.begin(), avgs.end());
    double avg_max = avgs.empty() ? 0.0 : *max_element(avgs.begin(), avgs.end());
    double avg_sum = 0.0; for (double v : avgs) avg_sum += v;
    double avg_mean = avgs.empty() ? 0.0 : avg_sum / avgs.size();

    ostringstream summary;
    summary << "\nSliding Window Summary\n";
    summary << "Readings processed: " << read_count << "\n";
    summary << "Window mode: " << (time_based ? string("time-based (seconds)") : string("count-based")) << "\n";
    if (!time_based) summary << "Window size (count): " << K << "\n"; else summary << "Window timespan (s): " << T_seconds << "\n";
    if (decay_mode) summary << "Decay mode: alpha=" << decay_alpha << "\n";
    summary << "Averages computed: " << outputs.size() << "\n";
    summary << "Averages min: " << avg_min << ", max: " << avg_max << ", mean: " << avg_mean << "\n";

    // Optionally produce top-K averages ranking
    if (top_k > 0 && !outputs.empty()){
        vector<pair<double,int>> idxs; idxs.reserve(outputs.size());
        for (int i=0;i<(int)outputs.size();++i) idxs.emplace_back(outputs[i].second, i);
        sort(idxs.begin(), idxs.end(), greater<>());
        summary << "\nTop " << top_k << " averages:\n";
        for (int i=0;i<min((int)idxs.size(), top_k); ++i){ int idx = idxs[i].second; summary << "  " << outputs[idx].first << "," << idxs[i].first << "\n"; }
    }

    // Write out
    if (!output_filename.empty()){
        ofstream ofs(output_filename);
        if (!ofs){ cerr << "Failed to open output file: " << output_filename << "\n"; return 11; }
        for (auto &ln : out_lines) ofs << ln << "\n";
        ofs << summary.str(); ofs.close(); if (!quiet) cout << "Wrote output and summary to: " << output_filename << "\n";
    } else {
        for (auto &ln : out_lines) cout << ln << "\n";
        cout << summary.str();
    }

    return 0;
}
