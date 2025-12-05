// --- Theme Toggle Logic ---
function toggleTheme() {
    const html = document.documentElement;
    const icon = document.getElementById('theme-icon');
    const mobileIcon = document.getElementById('mobile-theme-icon');
    
    if (html.classList.contains('dark')) {
        html.classList.remove('dark');
        localStorage.setItem('theme', 'light');
        icon.classList.remove('fa-sun');
        icon.classList.add('fa-moon');
        icon.classList.remove('text-yellow-500');
        icon.classList.add('text-gray-600');
        
        mobileIcon.classList.remove('fa-sun');
        mobileIcon.classList.add('fa-moon');
        mobileIcon.classList.remove('text-yellow-500');
        mobileIcon.classList.add('text-gray-600');
    } else {
        html.classList.add('dark');
        localStorage.setItem('theme', 'dark');
        icon.classList.remove('fa-moon');
        icon.classList.add('fa-sun');
        icon.classList.add('text-yellow-500');
        icon.classList.remove('text-gray-600');
        
        mobileIcon.classList.remove('fa-moon');
        mobileIcon.classList.add('fa-sun');
        mobileIcon.classList.add('text-yellow-500');
        mobileIcon.classList.remove('text-gray-600');
    }
}

// Initialize Theme
if (
    localStorage.theme === 'light' ||
    (!('theme' in localStorage) && window.matchMedia('(prefers-color-scheme: light)').matches)
) {
    document.documentElement.classList.remove('dark');
    const themeIcon = document.getElementById('theme-icon');
    const mobileThemeIcon = document.getElementById('mobile-theme-icon');
    if (themeIcon && mobileThemeIcon) {
        themeIcon.classList.remove('fa-sun', 'text-yellow-500');
        themeIcon.classList.add('fa-moon', 'text-gray-600');
        mobileThemeIcon.classList.remove('fa-sun', 'text-yellow-500');
        mobileThemeIcon.classList.add('fa-moon', 'text-gray-600');
    }
} else {
    document.documentElement.classList.add('dark');
    // Default icons are set to sun/yellow in HTML
}

// --- Navigation Logic ---
function showPage(pageId) {
    const pages = document.querySelectorAll('.page-section');
    pages.forEach(page => {
        page.classList.add('hidden');
    });

    const selectedPage = document.getElementById(pageId);
    if (selectedPage) {
        selectedPage.classList.remove('hidden');
        window.scrollTo(0, 0);
    }

    const mobileMenu = document.getElementById('mobile-menu');
    if (!mobileMenu.classList.contains('hidden')) {
        mobileMenu.classList.add('hidden');
    }
}

function toggleMobileMenu() {
    const menu = document.getElementById('mobile-menu');
    menu.classList.toggle('hidden');
}

// --- Modal Logic ---
function openProblemModal(element) {
    const title = element.querySelector('h4').innerText;
    const data = problemDatabase[title];
    
    if (data) {
        document.getElementById('modalTitle').innerText = title;
        document.getElementById('modalProblem').innerText = data.problem;
        document.getElementById('modalSolution').innerText = data.solution;
        
        const codeElement = document.getElementById('modalCode');
        codeElement.textContent = data.code;
        delete codeElement.dataset.highlighted; // Reset highlight state
        if (window.hljs) {
            hljs.highlightElement(codeElement);
        }
        
        document.getElementById('problemModal').classList.add('active');
    } else {
        console.error("No data found for: " + title);
    }
}

function closeProblemModal() {
    document.getElementById('problemModal').classList.remove('active');
}

document.addEventListener('keydown', function(event) {
    if (event.key === "Escape") {
        closeProblemModal();
    }
});

// --- Problem Data Database (Expanded Content) ---
const problemDatabase = {
    // --- Chinmay's Problems ---
    "Shortest Path Navigation": {
        problem: "Electric Vehicles (EVs) suffer from range anxiety. In a city with dynamic traffic conditions, finding the shortest path based on distance alone is insufficient. We need to find the quickest path from a start point to an end point in a city graph where edge weights represent real-time traffic delay, ensuring optimal battery usage and time efficiency.",
        solution: "We model the city as a weighted directed graph where intersections are nodes and roads are edges. Dijkstra's Algorithm is the optimal choice here because edge weights (time delay) are non-negative. We use a Priority Queue to always explore the shortest known path first, expanding layer by layer. This guarantees finding the shortest path with a time complexity of O(E log V).",
        code: `void dijkstra(int start, int end, int n, vector<vector<pair<int, int>>> &adj) {
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
    vector<int> dist(n, INT_MAX);

    dist[start] = 0;
    pq.push({0, start});

    while (!pq.empty()) {
        int d = pq.top().first;
        int u = pq.top().second;
        pq.pop();

        if (d > dist[u]) continue;
        if (u == end) return; 

        for (auto &edge : adj[u]) {
            int v = edge.first;
            int weight = edge.second;
            if (dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight;
                pq.push({dist[v], v});
            }
        }
    }
}`
    },
    "Max Traffic Flow": {
        problem: "During rush hour, specific highways feeding into the city center get clogged. We need to calculate the theoretical maximum number of vehicles that can travel from the residential suburbs (Source) to the central business district (Sink) without exceeding the capacity of any individual road segment.",
        solution: "This is a classic Max-Flow Min-Cut problem. We use the Ford-Fulkerson algorithm (specifically the Edmonds-Karp implementation using BFS) to find augmenting paths in the residual graph. As long as there is a path from source to sink with available capacity, we push more flow. The algorithm terminates when no such path exists, giving us the max flow.",
        code: `int bfs(int s, int t, vector<int>& parent, vector<vector<int>>& capacity, vector<vector<int>>& adj) {
    fill(parent.begin(), parent.end(), -1);
    parent[s] = -2;
    queue<pair<int, int>> q;
    q.push({s, INT_MAX});

    while (!q.empty()) {
        int u = q.front().first;
        int flow = q.front().second;
        q.pop();

        for (int v : adj[u]) {
            if (parent[v] == -1 && capacity[u][v] > 0) {
                parent[v] = u;
                int new_flow = min(flow, capacity[u][v]);
                if (v == t) return new_flow;
                q.push({v, new_flow});
            }
        }
    }
    return 0;
}`
    },
    "Subway Connectivity Check": {
        problem: "The subway system must remain fully connected for efficient transit. If a maintenance event isolates a station or a group of stations from the central hub, passengers could be stranded. We need an automated way to verify that every station is reachable from the main hub.",
        solution: "We perform a Breadth-First Search (BFS) or Depth-First Search (DFS) starting from the central hub (node 0). As we traverse, we mark every reached node as 'visited'. After the traversal is complete, we check if the count of visited nodes equals the total number of stations. If not, the graph is disconnected.",
        code: `bool isConnected(int n, vector<vector<int>>& adj) {
    vector<bool> visited(n, false);
    queue<int> q;
    q.push(0); // Start from hub
    visited[0] = true;
    int count = 0;

    while(!q.empty()){
        int u = q.front(); q.pop();
        count++;
        for(int v : adj[u]){
            if(!visited[v]){
                visited[v] = true;
                q.push(v);
            }
        }
    }
    return count == n;
}`
    },
    "Deadlock Prevention": {
        problem: "In a futuristic grid of autonomous vehicles, vehicles communicate to reserve intersections. If four vehicles arrive at a 4-way stop simultaneously and each waits for the one on its right, a deadlock occurs. We need to detect these circular wait conditions.",
        solution: "We model the resource allocation (intersections) as a directed graph. A deadlock corresponds to a cycle in this graph. We use Depth First Search (DFS) with a recursion stack tracking the current traversal path. If we encounter a node that is currently in the recursion stack, a back-edge exists, confirming a cycle.",
        code: `bool isCyclicUtil(int v, vector<bool> &visited, vector<bool> &recStack, vector<vector<int>> &adj) {
    if(!visited[v]) {
        visited[v] = true;
        recStack[v] = true;
        for(int i : adj[v]) {
            if ( !visited[i] && isCyclicUtil(i, visited, recStack, adj) )
                return true;
            else if (recStack[i])
                return true;
        }
    }
    recStack[v] = false;
    return false;
}`
    },
    "Road Network Maintenance": {
        problem: "Roads are frequently closed for maintenance or due to accidents. We need a system that can quickly determine if two specific districts are still connected via some path, or if they have been separated into disjoint components.",
        solution: "The Disjoint Set Union (DSU) or Union-Find data structure is ideal here. It maintains a collection of disjoint sets. Operations like `union` (merging two sets when a road is open) and `find` (checking which set a district belongs to) are nearly constant time, O(alpha(n)). This is much faster than running BFS/DFS for every query.",
        code: `struct DSU {
    vector<int> parent;
    int find(int i) {
        if (parent[i] == i) return i;
        return parent[i] = find(parent[i]); // Path compression
    }
    void unite(int i, int j) {
        int root_i = find(i);
        int root_j = find(j);
        if (root_i != root_j) parent[root_i] = root_j;
    }
};`
    },
    "Critical Bridge Identification": {
        problem: "To ensure resilience, we must identify 'critical' roads. These are edges in the graph that, if removed or destroyed, would increase the number of connected components (i.e., split the city in two).",
        solution: "We use Tarjan's Bridge-Finding algorithm. It performs a DFS traversal and maintains two arrays: `discovery_time` and `low_link_value`. An edge (u, v) is a bridge if `low[v] > disc[u]`, meaning there is no back-edge from the subtree of v to u or any ancestor of u.",
        code: `void bridgeUtil(int u, vector<int>& disc, vector<int>& low, int& time, vector<vector<int>>& adj) {
    disc[u] = low[u] = ++time;
    for (int v : adj[u]) {
        if (v == parent[u]) continue;
        if (disc[v]) {
            low[u] = min(low[u], disc[v]);
        } else {
            parent[v] = u;
            bridgeUtil(v, disc, low, time, adj);
            low[u] = min(low[u], low[v]);
            if (low[v] > disc[u]) cout << u << "-" << v << " is a bridge\\n";
        }
    }
}`
    },
    "Traffic Light Sync": {
        problem: "We have vehicle count data for hundreds of intersections. To improve flow, we want to prioritize green light duration for the busiest intersections. We need to rank them efficiently.",
        solution: "Sorting algorithms like Merge Sort or Quick Sort are used. Merge Sort O(n log n) is preferred if stability (preserving original order of equal elements) is important. This allows us to process the top K busiest intersections efficiently.",
        code: `void merge(vector<int>& arr, int l, int m, int r) {
    // Merge two sorted subarrays arr[l..m] and arr[m+1..r]
    // ... standard merge logic ...
}`
    },
    "A* Search": {
        problem: "Drone delivery requires 3D pathfinding. Unlike cars, drones can move diagonally and must avoid no-fly zones. We need a path that is not just short but also computationally efficient to calculate in real-time.",
        solution: "A* (A-Star) Search improves on Dijkstra by using a heuristic function (e.g., Euclidean distance to the destination). This 'guess' guides the algorithm towards the goal, significantly reducing the number of nodes explored compared to a blind search.",
        code: `double heuristic(Node a, Node b) {
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}
// Priority Queue stores nodes with priority = cost_so_far + heuristic`
    },
    "Congestion Monitoring": {
        problem: "Traffic sensors send data every second. To detect sustained congestion (not just momentary stops), we need to analyze the average traffic density over a rolling window (e.g., the last 5 minutes).",
        solution: "The Sliding Window technique is used. Instead of re-calculating the sum of the last K elements from scratch every second (O(k*n)), we maintain a running sum. When the window slides, we subtract the element leaving and add the element entering (O(n)).",
        code: `bool checkCongestion(vector<int>& data, int k, int threshold) {
    int sum = 0;
    for(int i=0; i<k; i++) sum += data[i];
    if(sum/k > threshold) return true;
    for(int i=k; i<data.size(); i++) {
        sum += data[i] - data[i-k];
        if(sum/k > threshold) return true;
    }
    return false;
}`
    },
    "EV Charging Stations": {
        problem: "We have a limited budget to build charging stations but want to cover the maximum number of unique road segments. This is an optimization problem.",
        solution: "This is a variant of the Set Cover Problem, which is NP-Hard. We use a Greedy Approximation algorithm. At each step, we pick the station location that covers the highest number of *currently uncovered* road segments, repeating until budget is exhausted or all roads are covered.",
        code: `while(universe_not_empty) {
    pick station that covers most uncovered elements;
    add station to solution;
    remove covered elements from universe;
}`
    },

    // --- Amogh's Problems ---
    "Grid Connectivity": {
        problem: "We need to physically connect all power substations with high-voltage cables. Copper is expensive, so the goal is to minimize the total length of cable used while keeping the entire grid connected.",
        solution: "This is a Minimum Spanning Tree (MST) problem. Prim's Algorithm is efficient for dense graphs. It starts with an arbitrary node and greedily adds the shortest edge connecting a node in the tree to a node outside the tree, ensuring no cycles are formed.",
        code: `int primMST(vector<vector<pair<int, int>>>& adj, int V) {
    priority_queue<pair<int, int>, ...> pq;
    vector<int> key(V, INT_MAX);
    // ... standard Prim's implementation ...
}`
    },
    "Battery Storage Optimization": {
        problem: "Energy storage is limited. During peak solar production, we have various 'energy packets' (generated units) with different values (efficiency) and sizes (storage cost). We need to fill our battery storage to maximize total value.",
        solution: "This maps directly to the 0/1 Knapsack Problem. A Greedy approach fails here. We use Dynamic Programming. `dp[w]` stores the max value achievable with capacity `w`. We iterate through items and update the DP table.",
        code: `int knapSack(int W, vector<int>& wt, vector<int>& val, int n) {
    vector<int> dp(W + 1, 0);
    for(int i=0; i<n; i++)
        for(int w=W; w>=wt[i]; w--)
            dp[w] = max(dp[w], val[i] + dp[w-wt[i]]);
    return dp[W];
}`
    },
    "Load Balancing": {
        problem: "Incoming power requests need to be assigned to substations. We want to assign tasks to the machine with the current minimum load.",
        solution: "A Greedy approach using a Min-Heap. We maintain the current load of all servers in a Min-Heap. For every new task, we extract the server with min load, add the task, and push it back.",
        code: `priority_queue<int, vector<int>, greater<int>> pq;
// Initialize pq with 0 load for all servers
for(int task : tasks) {
    int currentLoad = pq.top();
    pq.pop();
    currentLoad += task;
    pq.push(currentLoad);
}`
    },
    "System Boot Sequence": {
        problem: "Power plant subsystems have dependencies (e.g., Cooling must start before Turbine). We need a valid linear startup order.",
        solution: "This is a Directed Acyclic Graph (DAG) problem. We use Topological Sort (Kahn's Algorithm). We repeatedly pick nodes with in-degree 0 (no dependencies) and remove them from the graph.",
        code: `vector<int> topologicalSort(int V, vector<vector<int>>& adj) {
    vector<int> in_degree(V, 0);
    for (int u = 0; u < V; u++) {
        for (int v : adj[u]) in_degree[v]++;
    }

    queue<int> q;
    for (int i = 0; i < V; i++)
        if (in_degree[i] == 0) q.push(i);

    vector<int> result;
    while (!q.empty()) {
        int u = q.front(); q.pop();
        result.push_back(u);
        for (int v : adj[u])
            if (--in_degree[v] == 0) q.push(v);
    }
    return result;
}`
    },
    "Fault Detection": {
        problem: "A power line is 100km long. We can test voltage at any point. If voltage is 0, the break is before that point. Find the break with minimum tests.",
        solution: "Since the line is linear and the fault condition is monotonic (Good...Good...Break...Bad...Bad), we use Binary Search to find the transition point.",
        code: `int findBreak(int low, int high) {
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (checkVoltage(mid) == 0) // Fault is to the left
            high = mid - 1;
        else // Fault is to the right
            low = mid + 1;
    }
    return low; // Approximate location
}`
    },
    "Peak Usage Query": {
        problem: "We have an array of energy usage per hour. We need to answer many queries: 'What was the max usage between hour L and R?'",
        solution: "A Segment Tree is built on the array. Each node stores the max of its children. This allows Range Maximum Query (RMQ) in O(log n) time.",
        code: `int query(int node, int start, int end, int l, int r) {
    if (r < start || end < l) return INT_MIN;
    if (l <= start && end <= r) return tree[node];
    int mid = (start + end) / 2;
    int p1 = query(2 * node, start, mid, l, r);
    int p2 = query(2 * node + 1, mid + 1, end, l, r);
    return max(p1, p2);
}`
    },
    "Smart Meter Compression": {
        problem: "Smart meters send data constantly. Some values (like 0 usage) occur frequently. We want to reduce data size.",
        solution: "Huffman Coding assigns shorter binary codes to more frequent characters/values. It builds a binary tree where frequent items are near the root.",
        code: `// Standard Huffman logic
struct Node {
    char ch; int freq;
    Node *left, *right;
};
// Use Priority Queue to merge two smallest freq nodes repeatedly
// until one tree remains.`
    },
    "Frequency Allocation": {
        problem: "Substations transmit data. Adjacent stations cannot use the same frequency to avoid interference. Minimize total frequencies used.",
        solution: "This is the Graph Coloring problem. We use a Greedy coloring algorithm: iterate through nodes and assign the smallest color index not used by any neighbor.",
        code: `void greedyColoring(vector<vector<int>> &adj, int V) {
    vector<int> result(V, -1);
    result[0] = 0;
    vector<bool> available(V, true);

    for (int u = 1; u < V; u++) {
        for (int i : adj[u])
            if (result[i] != -1) available[result[i]] = false;

        int cr;
        for (cr = 0; cr < V; cr++)
            if (available[cr]) break;
            
        result[u] = cr;
        fill(available.begin(), available.end(), true); // Reset
    }
}`
    },
    "Real-time Consumption": {
        problem: "We have an array of consumption values. We frequently update values (index i) and query the total consumption from index 0 to i.",
        solution: "A Fenwick Tree (Binary Indexed Tree) provides O(log n) updates and O(log n) prefix sum queries, which is faster than an array for updates.",
        code: `void update(int i, int delta) {
    for (; i < n; i = i | (i + 1))
        bit[i] += delta;
}
int query(int i) {
    int sum = 0;
    for (; i >= 0; i = (i & (i + 1)) - 1)
        sum += bit[i];
    return sum;
}`
    },
    "Billing & User lookup": {
        problem: "We need to retrieve user billing info instantly using their unique User ID.",
        solution: "A Hash Map (Unordered Map in C++) uses a hash function to map keys to indices, providing O(1) average time complexity for lookups.",
        code: `unordered_map<string, UserData> db;
// Insert
db["user_123"] = { "John", 500.50 };
// Lookup
if(db.find("user_123") != db.end()) {
    return db["user_123"].bill;
}`
    },

    // --- Subhash's Problems ---
    "Population Density Analysis": {
        problem: "Calculate total population in a rectangular area (x1, y1) to (x2, y2) of the city grid.",
        solution: "We precompute a 2D Prefix Sum array. `pref[i][j]` stores sum of rectangle from (0,0) to (i,j). Any sub-rectangle sum is calculated in O(1) using the inclusion-exclusion principle.",
        code: `// Precompute
for(int i=1; i<=n; i++)
    for(int j=1; j<=m; j++)
        P[i][j] = matrix[i-1][j-1] + P[i-1][j] + P[i][j-1] - P[i-1][j-1];

// Query
int ans = P[x2][y2] - P[x1-1][y2] - P[x2][y1-1] + P[x1-1][y1-1];`
    },
    "Spatial Indexing": {
        problem: "Find all schools within a radius R of a coordinate. A simple list search is too slow.",
        solution: "A Quadtree recursively divides the 2D plane into four quadrants. Points are stored in leaf nodes. This allows for O(log n) spatial queries by ignoring quadrants that don't intersect the search area.",
        code: `struct Point { int x, y; };
struct QuadTree {
    Point topLeft, botRight;
    QuadTree *nw, *ne, *sw, *se;
    vector<Point> points;
    // Logic to subdivide when points exceed capacity
};`
    },
    "City Boundary Logic": {
        problem: "Given a set of outermost suburb coordinates, determine the polygon that encloses the entire city.",
        solution: "Convex Hull algorithm (e.g., Graham Scan or Monotone Chain). It sorts points and builds upper and lower hulls to find the perimeter points.",
        code: `// Monotone Chain Algorithm
vector<Point> convex_hull(vector<Point>& pts) {
    sort(pts.begin(), pts.end());
    vector<Point> hull;
    // Build lower hull
    for (const auto& p : pts) {
        while (hull.size() >= 2 && cross_product(hull[hull.size()-2], hull.back(), p) <= 0)
            hull.pop_back();
        hull.push_back(p);
    }
    // Build upper hull logic similar...
    return hull;
}`
    },
    "Housing Allocation": {
        problem: "Allocate budget to build different types of houses (Luxury, Affordable) to maximize utility/happiness.",
        solution: "Unbounded Knapsack problem (Dynamic Programming). Similar to 0/1 knapsack but items can be chosen multiple times.",
        code: `vector<int> dp(W + 1, 0);
for (int i = 0; i <= W; i++) {
    for (int j = 0; j < n; j++) {
        if (wt[j] <= i)
            dp[i] = max(dp[i], dp[i - wt[j]] + val[j]);
    }
}`
    },
    "Constraint Satisfaction": {
        problem: "Place N industrial zones on an NxN grid such that no two zones share the same row, column, or diagonal (N-Queens variant).",
        solution: "Backtracking. We place a zone in a row, check validity, and recursively move to the next row. If stuck, we backtrack and change the previous placement.",
        code: `bool solveNQUtil(vector<vector<int>>& board, int col) {
    if (col >= N) return true;
    for (int i = 0; i < N; i++) {
        if (isSafe(board, i, col)) {
            board[i][col] = 1;
            if (solveNQUtil(board, col + 1)) return true;
            board[i][col] = 0; // Backtrack
        }
    }
    return false;
}`
    },
    "Skyline Problem": {
        problem: "Given building coordinates (Left, Right, Height), output the contour of the skyline.",
        solution: "We treat buildings as critical points (left edge start, right edge end). We scan left to right and use a Max-Heap (or Multiset) to keep track of the max height active at the current X coordinate.",
        code: `// Pseudo-code for Skyline
vector<pair<int, int>> getSkyline(buildings) {
    // Transform to points: (x, -h) for start, (x, h) for end
    sort(points);
    multiset<int> heights = {0};
    int prevMax = 0;
    for(p : points) {
        if(start) heights.insert(h);
        else heights.remove(h);
        int curMax = *heights.rbegin();
        if(curMax != prevMax) {
            result.push({p.x, curMax});
            prevMax = curMax;
        }
    }
}`
    },
    "Service Center Locations": {
        problem: "Group residential blocks into K clusters to place a service center at the mean location of each cluster.",
        solution: "K-Means Clustering. Initialize K centroids randomly. 1. Assign each point to nearest centroid. 2. Recalculate centroids. 3. Repeat until convergence.",
        code: `// K-Means Logic
while(changed) {
    // Assignment Step
    for(p : points) {
        p.cluster = nearestCentroid(p);
    }
    // Update Step
    for(k=0; k<K; k++) {
        centroids[k] = average(pointsInCluster[k]);
    }
}`
    },
    "Job Assignment": {
        problem: "Assign N construction crews to N sites based on skill compatibility.",
        solution: "Max Bipartite Matching. Create a bipartite graph (Crews vs Sites). Use DFS to find a matching path.",
        code: `bool bpm(int u, bool visited[], int matchR[]) {
    for (int v = 0; v < N; v++) {
        if (bpGraph[u][v] && !visited[v]) {
            visited[v] = true;
            if (matchR[v] < 0 || bpm(matchR[v], visited, matchR)) {
                matchR[v] = u;
                return true;
            }
        }
    }
    return false;
}`
    },
    "Overlap Detection": {
        problem: "Given N line segments (utility pipes), determine if any two intersect.",
        solution: "Sweep Line Algorithm. Sort endpoints by X. Iterate through points. If left point, add to active set (BST). If right, remove. Check intersection with neighbors in active set.",
        code: `// Sweep Line concept
sort(events); // Events are start or end of segments
set<Segment> active;
for(e : events) {
    if(e.type == START) {
        // check intersection with successor/predecessor in active
        active.insert(e.segment);
    } else {
        active.erase(e.segment);
    }
}`
    },
    "Flood Risk Assessment": {
        problem: "Given a terrain grid where each cell has height. Water flows from High to Low. Identify all cells that water can reach from a river source.",
        solution: "DFS/BFS Traversal on a Matrix. Start DFS from river cells. Move to adjacent cells if `height[next] <= height[current]`.",
        code: `void dfs(int r, int c, vector<vector<int>>& h, vector<vector<bool>>& visited) {
    visited[r][c] = true;
    int dr[] = {0,0,1,-1};
    int dc[] = {1,-1,0,0};
    for(int i=0; i<4; i++) {
        int nr = r + dr[i], nc = c + dc[i];
        if(isValid(nr, nc) && !visited[nr][nc] && h[nr][nc] <= h[r][c])
            dfs(nr, nc, h, visited);
    }
}`
    },

    // --- Abhinav's Problems ---
    "Emergency Dispatch": {
        problem: "When an emergency call comes in, assign the nearest available ambulance. Prioritize incidents with higher severity.",
        solution: "Priority Queue (Min-Heap) for distance or Max-Heap for severity. We store events as `{severity, distance, id}` and pop the top element to dispatch.",
        code: `struct Event {
    int severity;
    int distance;
    bool operator<(const Event& other) const {
        if (severity != other.severity) return severity < other.severity; // Higher severity first
        return distance > other.distance; // Lower distance second
    }
};
priority_queue<Event> pq;`
    },
    "Garbage Truck Routing": {
        problem: "A garbage truck must visit a set of dumpsters and return to base. Minimize distance.",
        solution: "Traveling Salesperson Problem (TSP). Since N is small for a single truck's route, we can use Dynamic Programming with Bitmasking `dp[mask][last_visited]`.",
        code: `int tsp(int mask, int pos) {
    if (mask == VISITED_ALL) return dist[pos][0];
    if (dp[mask][pos] != -1) return dp[mask][pos];

    int ans = INT_MAX;
    for (int city = 0; city < n; city++) {
        if ((mask & (1 << city)) == 0) {
            int newAns = dist[pos][city] + tsp(mask | (1 << city), city);
            ans = min(ans, newAns);
        }
    }
    return dp[mask][pos] = ans;
}`
    },
    "Fire Station Coverage": {
        problem: "Find the minimum time to reach any building from ANY fire station.",
        solution: "Multi-Source BFS. Instead of starting BFS from one node, we push ALL fire stations into the queue initially with distance 0. Then run standard BFS.",
        code: `queue<int> q;
for(int station : stations) {
    q.push(station);
    dist[station] = 0;
}
while(!q.empty()) {
    int u = q.front(); q.pop();
    for(int v : adj[u]) {
        if(dist[v] == INF) {
            dist[v] = dist[u] + 1;
            q.push(v);
        }
    }
}`
    },
    "Crime Hotspot Prediction": {
        problem: "Classify a location as Safe or Risky based on K nearest historical crime reports.",
        solution: "K-Nearest Neighbors (KNN). Calculate distance to all data points, sort by distance, pick top K, and vote.",
        code: `// Simple KNN
vector<pair<double, int>> distances;
for(auto& p : data) {
    distances.push_back({ dist(query, p.coords), p.label });
}
sort(distances.begin(), distances.end());
int riskCount = 0;
for(int i=0; i<K; i++) if(distances[i].second == RISK) riskCount++;
return (riskCount > K/2) ? RISK : SAFE;`
    },
    "Emergency Contact Lookup": {
        problem: "Autocompleting names or addresses for dispatchers.",
        solution: "Trie (Prefix Tree). Each node represents a character. Efficient for prefix-based searching.",
        code: `struct TrieNode {
    TrieNode *children[26];
    bool isEndOfWord;
};
void insert(TrieNode *root, string key) {
    TrieNode *pCrawl = root;
    for (int i = 0; i < key.length(); i++) {
        int index = key[i] - 'a';
        if (!pCrawl->children[index])
            pCrawl->children[index] = new TrieNode();
        pCrawl = pCrawl->children[index];
    }
    pCrawl->isEndOfWord = true;
}`
    },
    "Hospital Bed Management": {
        problem: "Schedule as many surgeries as possible in a specific operating room given start and end times.",
        solution: "Interval Scheduling (Greedy). Sort requests by FINISH time. Always pick the next request that starts after the last one finished.",
        code: `sort(intervals.begin(), intervals.end(), compareEndTime);
int count = 1;
int last_end = intervals[0].end;
for(int i=1; i<n; i++) {
    if(intervals[i].start >= last_end) {
        count++;
        last_end = intervals[i].end;
    }
}`
    },
    "Dispatch Logs": {
        problem: "Keep exactly the last K calls for auditing.",
        solution: "Circular Queue (Ring Buffer). A fixed-size array where the 'next' pointer wraps around to 0 after reaching the end.",
        code: `void enqueue(int val) {
    if ((rear + 1) % size == front) return; // Full
    if (front == -1) front = 0;
    rear = (rear + 1) % size;
    arr[rear] = val;
}`
    },
    "Disaster Evacuation": {
        problem: "Calculate shortest paths between ALL pairs of districts and shelters to plan flexible evacuation routes.",
        solution: "Floyd-Warshall Algorithm. Dynamic programming approach to find all-pairs shortest paths.",
        code: `for (k = 0; k < V; k++) {
    for (i = 0; i < V; i++) {
        for (j = 0; j < V; j++) {
            if (dist[i][k] + dist[k][j] < dist[i][j])
                dist[i][j] = dist[i][k] + dist[k][j];
        }
    }
}`
    },
    "Duplicate Incident Filtering": {
        problem: "Many people call 911 for the same fire. We need to ignore duplicates.",
        solution: "HashSet. Store hashes of active incidents (e.g., location_grid_id + type). Check existence in O(1).",
        code: `unordered_set<string> activeIncidents;
string id = to_string(x) + "_" + to_string(y) + "_" + type;
if(activeIncidents.find(id) == activeIncidents.end()) {
    activeIncidents.insert(id);
    dispatch();
}`
    },
    "Medical Data Access": {
        problem: "Ambulance tablets have limited memory. Keep the most recently accessed patient records.",
        solution: "LRU Cache. Use a Hash Map + Doubly Linked List. Map stores iterators to the list nodes. Move accessed node to front of list.",
        code: `class LRUCache {
    list<int> dq;
    unordered_map<int, list<int>::iterator> ma;
    // get: move iterator to front of list
    // put: if full, pop back of list, push front
};`
    }
};

// --- Init & Routing ---
window.addEventListener('popstate', (event) => {
    if (event.state && event.state.page) {
        showPage(event.state.page);
    } else {
        showPage('home');
    }
});

document.addEventListener('DOMContentLoaded', () => {
    const hash = window.location.hash.replace('#', '');
    if (hash && document.getElementById(hash)) {
        showPage(hash);
    } else {
        showPage('home');
    }
});
