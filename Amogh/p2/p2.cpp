#include <bits/stdc++.h>
using namespace std;

int main() {
    // Battery capacity (you can change this as needed)
    int W = 1000;

    ifstream file("knapsack_1000.csv");
    if (!file.is_open()) {
        cout << "Error opening CSV file!" << endl;
        return 1;
    }

    vector<int> weights;
    vector<int> values;

    string line;
    bool skip = true;

    while (getline(file, line)) {
        if (skip) { skip = false; continue; }

        stringstream ss(line);
        string w_str, v_str;

        getline(ss, w_str, ',');
        getline(ss, v_str, ',');

        weights.push_back(stoi(w_str));
        values.push_back(stoi(v_str));
    }
    file.close();

    int n = weights.size();
    vector<int> dp(W + 1, 0);

    for (int i = 0; i < n; i++) {
        for (int w = W; w >= weights[i]; w--) {
            dp[w] = max(dp[w], values[i] + dp[w - weights[i]]);
        }
    }

    cout << "Maximum energy value stored = " << dp[W] << endl;
    return 0;
}
