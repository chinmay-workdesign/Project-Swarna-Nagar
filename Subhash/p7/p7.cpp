#include <iostream>
#include <vector>
#include <stack>
#include <set>
#include <unordered_map>
#include <algorithm>
using namespace std;

/*
MODEL 7: RC-ZFS – Zoning Feasibility Solver
Uses Backtracking + Graph + Pruning + Multiple DS
*/

#define ZONES 4
#define TYPES 3   // 0: Residential, 1: Industrial, 2: Green

struct Zone {
    int id;
    string name;
};

// ---------------- GLOBAL STRUCTURES ----------------
vector<vector<int>> conflictGraph;   // adjacency list
vector<int> assignment(ZONES, -1);   // zone -> type
set<int> usedTypes;
stack<int> stateStack;
unordered_map<int, string> zoneTypeName;

// ---------------- SAFETY CHECK ----------------
bool isSafe(int zone, int type) {

    // Constraint 1: no same type in adjacent zones
    for (int adj : conflictGraph[zone]) {
        if (assignment[adj] == type)
            return false;
    }

    // Constraint 2: Industrial not next to Residential
    if (type == 1) {
        for (int adj : conflictGraph[zone]) {
            if (assignment[adj] == 0)
                return false;
        }
    }

    return true;
}

// ---------------- BACKTRACKING SOLVER ----------------
bool solveZoning(int zone) {

    // BASE CASE
    if (zone == ZONES)
        return true;

    // TRY ALL TYPES
    for (int type = 0; type < TYPES; type++) {

        // PRUNING
        if (!isSafe(zone, type))
            continue;

        // ASSIGN
        assignment[zone] = type;
        usedTypes.insert(type);
        stateStack.push(zone);

        // DFS RECURSION
        if (solveZoning(zone + 1))
            return true;

        // BACKTRACK
        assignment[zone] = -1;
        usedTypes.erase(type);
        stateStack.pop();
    }

    return false;
}

// ---------------- MAIN ----------------
int main() {

    // ZONE TYPE NAMES
    zoneTypeName[0] = "Residential";
    zoneTypeName[1] = "Industrial";
    zoneTypeName[2] = "Green";

    // BUILD CONFLICT GRAPH
    conflictGraph.resize(ZONES);
    conflictGraph[0] = {1};
    conflictGraph[1] = {0,2};
    conflictGraph[2] = {1,3};
    conflictGraph[3] = {2};

    cout << "=== RC-ZFS : Zoning Feasibility Solver ===\n\n";

    // RUN SOLVER
    if (solveZoning(0)) {

        cout << "Valid Zoning Plan Found:\n\n";

        for (int i = 0; i < ZONES; i++) {
            cout << "Zone " << i
                 << " -> " << zoneTypeName[assignment[i]] << endl;
        }

        cout << "\nZones assigned using Backtracking & Pruning.\n";

    } else {
        cout << "No valid zoning configuration possible.\n";
    }

    // STACK TRACE
    cout << "\nState Stack Trace (DFS order):\n";
    while (!stateStack.empty()) {
        cout << "Zone " << stateStack.top() << endl;
        stateStack.pop();
    }

    return 0;
}
