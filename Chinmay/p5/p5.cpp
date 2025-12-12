#include <bits/stdc++.h>
using namespace std;

struct DSU {
    int n;
    vector<int> parent, sz;
    DSU(int n=0): n(n), parent(n), sz(n,1) {
        iota(parent.begin(), parent.end(), 0);
    }
    int find(int x){
        if (parent[x]==x) return x;
        return parent[x]=find(parent[x]);
    }
    bool unite(int a,int b){
        a=find(a); b=find(b);
        if (a==b) return false;
        if (sz[a]<sz[b]) swap(a,b);
        parent[b]=a;
        sz[a]+=sz[b];
        return true;
    }
    bool same(int a,int b){ return find(a)==find(b); }
};

// Connectivity checker using DSU.
// CSV format (first line header):
// num_nodes,num_edges,num_queries
// Second line: N,M,Q
// Next M lines: u,v   (initial open roads; undirected)
// Next Q lines: u,v   (connectivity queries: are u and v connected?)
// Nodes are 0-indexed.

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string header;
    if (!getline(cin, header)) return 0;
    stringstream ss(header);
    int n,m,q; char c;
    ss >> n >> c >> m >> c >> q; // parse header line

    string line;
    if (!getline(cin, line)) return 0; // second line N,M,Q
    stringstream s2(line);
    s2 >> n >> c >> m >> c >> q;

    DSU dsu(n);

    for (int i=0;i<m;i++){
        if (!getline(cin,line)) break;
        if (line.size()==0){ --i; continue; }
        stringstream es(line);
        int u,v; char cc;
        es >> u >> cc >> v;
        if (u<0||u>=n||v<0||v>=n) continue;
        dsu.unite(u,v);
    }

    for (int i=0;i<q;i++){
        if (!getline(cin,line)) break;
        if (line.size()==0){ --i; continue; }
        stringstream es(line);
        int u,v; char cc;
        es >> u >> cc >> v;
        if (u<0||u>=n||v<0||v>=n) {
            cout << "DISCONNECTED\n";
            continue;
        }
        if (dsu.same(u,v)) cout << "CONNECTED\n";
        else cout << "DISCONNECTED\n";
    }

    return 0;
}
