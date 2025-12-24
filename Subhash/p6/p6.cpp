#include <iostream>
#include <vector>
using namespace std;

/*
MODEL 6: IHDOF – Housing Distribution Optimizer
Goal:
Maximize housing utility under land/budget constraints.

Housing Types:
0 -> Affordable
1 -> Premium
2 -> Mixed-use
*/

int main() {

    // ---------------- INPUT DATA ----------------
    int budget = 10;   // total land/budget capacity

    // utility value of each housing type
    int value[] = {30, 70, 50};

    // land/budget cost of each housing type
    int cost[]  = {3, 6, 4};

    int n = 3; // number of housing types

    // ---------------- DP TABLE ----------------
    vector<int> dp(budget + 1, 0);

    // ---------------- DYNAMIC PROGRAMMING ----------------
    // Unbounded Knapsack (each housing type can be used multiple times)
    for (int b = 1; b <= budget; b++) {
        for (int i = 0; i < n; i++) {
            if (cost[i] <= b) {
                dp[b] = max(dp[b], dp[b - cost[i]] + value[i]);
            }
        }
    }

    // ---------------- OUTPUT ----------------
    cout << "Housing Budget Capacity: " << budget << endl;
    cout << "Maximum Housing Utility Achieved: " << dp[budget] << endl;

    cout << "\nDP Table (Budget vs Utility):\n";
    for (int i = 0; i <= budget; i++) {
        cout << "Budget " << i << " -> Utility " << dp[i] << endl;
    }

    return 0;
}
