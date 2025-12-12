#include <bits/stdc++.h>
using namespace std;

/*
====================================================================================================
                        BILLING SYSTEM LOOKUP USING UNORDERED MAP
----------------------------------------------------------------------------------------------------
We need fast retrieval of user billing info using User ID.
Unordered Map in C++ provides:
    - Average O(1) lookup
    - Average O(1) insertion

This program:
    1. Loads a CSV file "user_billing_data.csv"
    2. Stores (user_id → billing_amount) in an unordered_map
    3. Allows multiple sample lookups
====================================================================================================
*/

// =================================================================================================
// SECTION 1 — Utility Functions
// =================================================================================================

string trim(const string &s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    size_t end   = s.find_last_not_of(" \t\n\r");
    if (start == string::npos) return "";
    return s.substr(start, end - start + 1);
}

void printBanner(const string &msg) {
    cout << "\n============================================================\n";
    cout << msg << "\n";
    cout << "============================================================\n";
}

// =================================================================================================
// SECTION 2 — CSV Loader
// =================================================================================================

class CSVLoader {
public:
    string filename;

    CSVLoader(string fn) {
        filename = fn;
    }

    void load(vector<int> &user_ids, vector<int> &billings) {
        printBanner("Loading User Billing Data");

        ifstream file(filename);

        if (!file.is_open()) {
            cout << "ERROR: Could not open CSV file.\n";
            exit(1);
        }

        string line;
        bool skipHeader = true;

        while (getline(file, line)) {
            if (skipHeader) {
                skipHeader = false;
                continue;
            }

            stringstream ss(line);
            string id_str, bill_str;

            getline(ss, id_str, ',');
            getline(ss, bill_str, ',');

            int user_id = stoi(trim(id_str));
            int bill = stoi(trim(bill_str));

            user_ids.push_back(user_id);
            billings.push_back(bill);
        }

        cout << "Loaded " << user_ids.size() << " records.\n";
    }
};

// =================================================================================================
// SECTION 3 — Billing Database Using Hash Map
// =================================================================================================

class BillingDatabase {
public:
    unordered_map<int, int> db;

    void build(const vector<int> &ids, const vector<int> &bills) {
        printBanner("Building Hash Map");

        for (int i = 0; i < ids.size(); i++) {
            db[ids[i]] = bills[i];
        }

        cout << "Hash map size: " << db.size() << "\n";
    }

    int lookup(int user_id) {
        if (db.find(user_id) != db.end()) {
            return db[user_id];
        }
        return -1;
    }

    void printSampleLookups(const vector<int> &ids) {
        printBanner("Sample Lookups");

        for (int i = 0; i < min((int)ids.size(), 10); i++) {
            int uid = ids[i];
            int amount = lookup(uid);
            cout << "User ID " << uid << " → Billing: " << amount << "\n";
        }
    }
};

// =================================================================================================
// SECTION 4 — MAIN PROGRAM
// =================================================================================================

int main() {
    printBanner("BILLING LOOKUP SYSTEM — HASH MAP");

    vector<int> user_ids;
    vector<int> billing_amounts;

    CSVLoader loader("user_billing_data.csv");
    loader.load(user_ids, billing_amounts);

    BillingDatabase db;
    db.build(user_ids, billing_amounts);
    db.printSampleLookups(user_ids);

    printBanner("Enter a User ID to Retrieve Billing (Example Lookup)");
    int q;
    cin >> q;

    int result = db.lookup(q);

    if (result != -1) {
        cout << "Billing Amount: " << result << "\n";
    } else {
        cout << "User ID not found.\n";
    }

    printBanner("PROGRAM COMPLETE");
    return 0;
}

/*
====================================================================================================
End of Program
====================================================================================================
*/
