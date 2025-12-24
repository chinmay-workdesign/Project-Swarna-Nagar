#include <iostream>
#include <vector>
using namespace std;

/*
MODEL 4: Population Intelligence System (MSPIS)
Techniques:
1) 2D Prefix Sum
2) 2D Fenwick Tree
*/

int N = 5;
vector<vector<int>> city;
vector<vector<int>> prefix;
vector<vector<int>> fenwick;

/* ---------- BUILD 2D PREFIX SUM ---------- */
void buildPrefixSum() {
    prefix.assign(N, vector<int>(N, 0));

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            prefix[i][j] = city[i][j]
                + (i ? prefix[i-1][j] : 0)
                + (j ? prefix[i][j-1] : 0)
                - (i && j ? prefix[i-1][j-1] : 0);
        }
    }
}

/* ---------- QUERY PREFIX SUM ---------- */
int queryPrefix(int r1, int c1, int r2, int c2) {
    return prefix[r2][c2]
        - (r1 ? prefix[r1-1][c2] : 0)
        - (c1 ? prefix[r2][c1-1] : 0)
        + (r1 && c1 ? prefix[r1-1][c1-1] : 0);
}

/* ---------- FENWICK TREE HELPERS ---------- */
void fenwickUpdate(int x, int y, int val) {
    for (int i = x + 1; i <= N; i += (i & -i))
        for (int j = y + 1; j <= N; j += (j & -j))
            fenwick[i][j] += val;
}

int fenwickQuery(int x, int y) {
    int sum = 0;
    for (int i = x + 1; i > 0; i -= (i & -i))
        for (int j = y + 1; j > 0; j -= (j & -j))
            sum += fenwick[i][j];
    return sum;
}

int fenwickRange(int r1, int c1, int r2, int c2) {
    return fenwickQuery(r2, c2)
         - fenwickQuery(r1-1, c2)
         - fenwickQuery(r2, c1-1)
         + fenwickQuery(r1-1, c1-1);
}

/* ---------- MAIN ---------- */
int main() {

    // Sample population grid (city zones)
    city = {
        {10, 20, 30, 40, 50},
        {15, 25, 35, 45, 55},
        {20, 30, 40, 50, 60},
        {25, 35, 45, 55, 65},
        {30, 40, 50, 60, 70}
    };

    fenwick.assign(N + 1, vector<int>(N + 1, 0));

    // Build Fenwick Tree
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            fenwickUpdate(i, j, city[i][j]);

    // Build Prefix Sum
    buildPrefixSum();

    cout << "Population in region (1,1) to (3,3) using Prefix Sum: ";
    cout << queryPrefix(1, 1, 3, 3) << endl;

    cout << "Population in region (1,1) to (3,3) using Fenwick Tree: ";
    cout << fenwickRange(1, 1, 3, 3) << endl;

    return 0;
}

