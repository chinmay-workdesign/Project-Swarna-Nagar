#include <bits/stdc++.h>
using namespace std;

// Sliding window average calculator.
// CSV input format (first line header):
// num_readings,window_size
// Second line: N,K
// Next N lines: timestamp,value  (timestamp can be integer seconds or ISO string without commas)
// Output: for each reading starting when a full window is available, prints timestamp,average

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string header;
    if (!getline(cin, header)) return 0;
    stringstream ss(header);
    int N, K; char c;
    // parse header line like: num_readings,window_size
    // then expect second line "N,K"
    string line;
    if (!getline(cin, line)) return 0;
    stringstream s2(line);
    s2 >> N >> c >> K;

    vector<string> timestamps;
    vector<double> values;
    timestamps.reserve(N);
    values.reserve(N);

    for (int i = 0; i < N; ++i) {
        if (!getline(cin, line)) break;
        if (line.size() == 0) { --i; continue; }
        // split on first comma
        auto pos = line.find(',');
        if (pos == string::npos) continue;
        string ts = line.substr(0, pos);
        double v = stod(line.substr(pos+1));
        timestamps.push_back(ts);
        values.push_back(v);
    }

    if ((int)values.size() != N) {
        cerr << "Warning: expected " << N << " readings but read " << values.size() << "\n";
    }

    // Data structures used: vector for values, deque for window values, running sum, vector for output averages
    deque<double> window;
    double running_sum = 0.0;
    vector<pair<string,double>> outputs; outputs.reserve(max(0, (int)values.size()-K+1));

    for (int i = 0; i < (int)values.size(); ++i){
        window.push_back(values[i]);
        running_sum += values[i];
        if ((int)window.size() > K){
            running_sum -= window.front();
            window.pop_front();
        }
        if ((int)window.size() == K){
            double avg = running_sum / K;
            outputs.push_back({timestamps[i], avg});
        }
    }

    // Print CSV header and results
    cout << "timestamp,avg_over_window\n";
    for (auto &p : outputs) cout << p.first << "," << p.second << "\n";

    return 0;
}
