#include <bits/stdc++.h>
using namespace std;


// =================================================================================================
// SECTION 1 — Helper Functions
// =================================================================================================

string trim(const string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end   = s.find_last_not_of(" \t\r\n");
    if (start == string::npos) return "";
    return s.substr(start, end - start + 1);
}

void printBanner(const string &title) {
    cout << "\n============================================================\n";
    cout << title << "\n";
    cout << "============================================================\n";
}

// =================================================================================================
// SECTION 2 — Node Structure for Huffman Tree
// =================================================================================================

struct Node {
    int value;    
    int freq;     
    Node *left;
    Node *right;

    Node(int v, int f) {
        value = v;
        freq = f;
        left = nullptr;
        right = nullptr;
    }
};

// Comparator for min-heap
struct Compare {
    bool operator()(Node* a, Node* b) {
        return a->freq > b->freq;
    }
};

// =================================================================================================
// SECTION 3 — CSV Loader
// =================================================================================================

class CSVLoader {
public:
    string filename;

    CSVLoader(string fn) : filename(fn) {}

    void load(vector<int> &values) {
        printBanner("Loading CSV Values");

        ifstream file(filename);
        if (!file.is_open()) {
            cout << "ERROR: Cannot open CSV file\n";
            exit(1);
        }

        string line;
        bool skip = true;

        while (getline(file, line)) {
            if (skip) { skip = false; continue; }
            stringstream ss(line);
            string val;
            getline(ss, val, ',');
            values.push_back(stoi(trim(val)));
        }

        cout << "Loaded " << values.size() << " meter readings.\n";
    }
};

// =================================================================================================
// SECTION 4 — Huffman Tree Builder
// =================================================================================================

class Huffman {
public:
    unordered_map<int, int> freq;
    unordered_map<int, string> codes;
    Node* root;

    Huffman() { root = nullptr; }

    void computeFreq(const vector<int> &vals) {
        printBanner("Computing Frequencies");
        for (int v : vals) freq[v]++;
        cout << "Unique values: " << freq.size() << "\n";
    }

    void buildTree() {
        printBanner("Building Huffman Tree");

        priority_queue<Node*, vector<Node*>, Compare> pq;

        for (auto &p : freq) {
            pq.push(new Node(p.first, p.second));
        }

        while (pq.size() > 1) {
            Node* left  = pq.top(); pq.pop();
            Node* right = pq.top(); pq.pop();

            Node* merged = new Node(-1, left->freq + right->freq);
            merged->left = left;
            merged->right = right;

            pq.push(merged);
        }

        root = pq.top();
    }

    void generateCodes(Node* node, string current) {
        if (!node) return;

        if (node->value != -1) {
            codes[node->value] = current;
        }

        generateCodes(node->left,  current + "0");
        generateCodes(node->right, current + "1");
    }

    void printCodes() {
        printBanner("Generated Huffman Codes");
        int count = 0;
        for (auto &p : codes) {
            cout << "Value " << p.first 
                 << " -> Code: " << p.second << "\n";
            if (++count >= 20) break;
        }
    }
};

// =================================================================================================
// SECTION 5 — Main 
// =================================================================================================

int main() {
    printBanner("SMART METER DATA COMPRESSION — HUFFMAN CODING");

    vector<int> readings;

    CSVLoader loader("meter_values_data.csv");
    loader.load(readings);

    Huffman h;
    h.computeFreq(readings);
    h.buildTree();
    h.generateCodes(h.root, "");
    h.printCodes();

    printBanner("PROGRAM COMPLETED");
    return 0;
}

