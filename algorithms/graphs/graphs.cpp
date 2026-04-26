#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <map>
#include <set>
#include <cstdint>
#include <algorithm>
#include <numeric>
#include <cstring>
#include <deque>
#include <unordered_set>
using namespace std;

enum GRAPH_PROBLEM {
    // EASY
    SURROUNDING_XO=0,
    TOPOLOGICAL_SORT,
    // MEDIUM 
    LINE_CHART,
    WAYS_ARRIVE_DESTINATION,
    // HARD
    OBSTACLE_REMOVAL,
    INCREASING_PATHS,
    PARALLEL_COURSE2,
    MINIMIZE_MALWARE_SPREAD,
    NUMBER_OF_GOOD_PATHS,
    FIND_EDGES_SHORTEST_PATH,
    SUDOKU_SOLVER,
};

auto fill(vector<vector<char>>& grid) -> void {
    // time O(m*n)
    // space O(m*n)
    int m=grid.size(),n=grid[0].size();
    int dir[]{-1,0,1,0,-1};

    function<void(int,int)> dfs = [&](int i, int j) -> void {
        if (i <0||i>=m||j<0||j>=n||grid[i][j]!='O') return;
        grid[i][j]='.';
        for (int k=0;k<4;k++) {
            dfs(i+dir[k],j+dir[k+1]);
        }
    };

    for (int i=0;i<m;i++) {
        dfs(i,0);
        dfs(i,n-1);
    }
    for (int j=1;j<m-1;j++) {
        dfs(0,j);
        dfs(m-1,j);
    }

    for (int i=0;i<m;i++) {
        for (int j=0;j<n;j++) {
            if (grid[i][j]=='.') {
                grid[i][j]='O';
            } else if (grid[i][j]=='O') {
                grid[i][j]='X';
            }
        }
    }
}

auto toposort(int V, const vector<vector<int>>& edges) -> vector<int> {
    // time O(V+E)
    // space O(V)
    vector<vector<int>> e(V);
    vector<int> in(V);
    for (auto& ee : edges) {
        e[ee[0]].push_back(ee[1]);
        in[ee[1]]++;
    }
    queue<int> q;
    for (int v=0;v<V;v++) {
        if (!in[v]) 
            q.push(v);
    }
    vector<int> topo;
    while (!q.empty()) {
        int v=q.front();
        q.pop();
        topo.push_back(v);
        for (auto u : e[v]) {
            if (!--in[u])
                q.push(u);
        }
    }
    return topo;
}

auto minimum_lines(vector<vector<int>>& stockPrices) -> int {
    // time O(nlogn)
    // space O(1) if considering sort then O(n)
    if (stockPrices.size()<=1) return 0;
    sort(stockPrices.begin(),stockPrices.end());
    int ans=0;
    int dx=0,dy=1;
    for (int i=1;i<(int)stockPrices.size();i++) {
        int x=stockPrices[i-1][0], y=stockPrices[i-1][1];
        int x1=stockPrices[i][0], y1=stockPrices[i][1];
        int dx1=x1-x, dy1=y1-y;
        if ((int64_t)dy1*dx!=(int64_t)dx1*dy) ans++;
        dx=dx1;
        dy=dy1;
    }
    return ans;
}

auto min_obstacles(vector<vector<int>>& grid) -> int {
    // time O(m*n)
    // space O(m*n)
    int m=grid.size(), n=grid[0].size();
    int vis[m][n];
    memset(vis,false,sizeof(vis));
    int dir[]{-1,0,1,0,-1};
    deque<tuple<int,int,int>> dq{{0,0,0}};
    while (!dq.empty()) {
        tuple<int,int,int> t = dq.front();
        int i,j,k;
        tie(i,j,k) = t;
        dq.pop_front();
        if (i==m-1 && j==n-1) {
            return k;
        }
        if (vis[i][j]) continue;
        vis[i][j]=true;
        for (int h=0;h<4;h++) {
            int x=i+dir[h], y=j+dir[h+1];
            if (x>=0 && x<m && y>=0 && y<n) {
                if (grid[x][y] == 0) {
                    dq.push_front({x,y,k});
                } else {
                    dq.push_back({x,y,k+1});
                }
            }
        }
    }
    return -1;
}

auto count_paths(vector<vector<int>>& grid) -> int { 
    // time O(m*n)
    // space O(m*n)
    const int mod = 1e9+7;
    int m=grid.size(), n=grid[0].size();
    int f[m][n];
    memset(f,0,sizeof(f));
    int dir[]{-1,0,1,0,-1};

    function<int(int,int)> dfs = [&](int i, int j) -> int {
        if (f[i][j]) 
            return f[i][j];
        int ans = 1;
        for (int k=0;k<4;k++) {
            int x=i+dir[k], y=j+dir[k+1];
            if (x>=0 && x<m && y>=0 && y<n && grid[i][j]<grid[x][y]) {
                ans=(ans+dfs(x,y))%mod;
            }
        }
        return f[i][j]=ans;
    };

    int ans=0;
    for (int i=0;i<m;i++) {
        for (int j=0;j<n;j++)  {
            ans=(ans+dfs(i,j))% mod;
        }
    }
    return ans;
}

auto min_num_of_semesters(int n, vector<vector<int>>& relations, int k) -> int {
     // d[i] = bitmask of prerequisites for course i
        //
        // If we have an edge [a, b], then course a must be taken before b,
        // so we set the a-th bit inside d[b].
        //
        // Example:
        // if relations = [[2, 1], [3, 1]],
        // then d[1] will have bits 2 and 3 turned on,
        // meaning course 1 requires courses 2 and 3 first.
        vector<int> d(n + 1);

        for (auto& e : relations) {
            d[e[1]] |= 1 << e[0];
        }

        // BFS queue:
        // each state is {cur, t}
        //
        // cur = bitmask of courses already taken
        // t   = number of semesters used to reach this state
        queue<pair<int, int>> q;

        // Start with no courses taken, and 0 semesters used.
        q.push({0, 0});

        // vis stores which "taken-course masks" we have already processed.
        //
        // Since BFS explores in increasing number of semesters,
        // the first time we reach a mask is the minimum semesters needed
        // to reach that exact set of completed courses.
        unordered_set<int> vis{{0}};

        while (!q.empty()) {
            auto [cur, t] = q.front();
            q.pop();

            // Goal check:
            // We want all course bits 1..n to be 1.
            //
            // (1 << (n + 1)) gives binary 1 followed by n+1 zeros.
            // subtract 2 => bits 1..n become 1, bit 0 stays 0.
            //
            // Example n = 4:
            // (1 << 5) - 2 = 11110 in binary
            //
            // This means all courses 1..n are taken.
            if (cur == (1 << (n + 1)) - 2) {
                return t;
            }

            // nxt will store the set of courses that are available
            // to be taken in the NEXT semester.
            int nxt = 0;

            // Check every course from 1..n.
            for (int i = 1; i <= n; ++i) {
                // A course i can be taken if ALL its prerequisite bits
                // are already present in cur.
                //
                // (cur & d[i]) extracts from cur only the bits that are prerequisites of i.
                // If that equals d[i], then every prerequisite has been completed.
                if ((cur & d[i]) == d[i]) {
                    nxt |= 1 << i;
                }
            }

            // At this point, nxt contains all courses whose prerequisites
            // are satisfied, INCLUDING courses we may have already taken before.
            //
            // We only want courses that are:
            //   1) available now
            //   2) not already taken
            //
            // So remove all bits that are already in cur.
            nxt &= ~cur;

            // If the number of available untaken courses is <= k,
            // we can take all of them this semester.
            if (__builtin_popcount(nxt) <= k) {
                // nxt | cur = new mask after taking all currently available courses
                if (!vis.count(nxt | cur)) {
                    vis.insert(nxt | cur);
                    q.push({nxt | cur, t + 1});
                }
            } else {
                // More than k courses are available.
                //
                // We cannot take all of them, so we must choose exactly k of them.
                //
                // x stores the full available set, and then we enumerate
                // all submasks of x using the classic trick:
                //
                // sub = (sub - 1) & x
                //
                // This iterates over every non-empty subset of x.
                int x = nxt;

                while (nxt) {
                    // Only consider subsets of size exactly k.
                    //
                    // Why exactly k and not fewer?
                    // Because if more than k courses are available,
                    // taking fewer than k is never better than taking k:
                    // taking more available courses can only help or keep things equal.
                    if (__builtin_popcount(nxt) == k && !vis.count(nxt | cur)) {
                        vis.insert(nxt | cur);
                        q.push({nxt | cur, t + 1});
                    }

                    // Move to the next submask of x.
                    nxt = (nxt - 1) & x;
                }
            }
        }

        // Problem guarantees it is possible to finish all courses,
        // so in practice we should never get here.
        return 0;
}

auto min_malware_spread(vector<vector<int>>& graph, vector<int>& initial) -> int {
    // Time O(n^2 + alpha(n))
    // Space O(n)
    struct dsu {
        vector<int> e; void init(int n) { e = vector<int>(n,-1); }
        int get(int x) { return e[x] < 0 ? x : e[x] = get(e[x]); }
        int sameSet(int x, int y) { return get(x) == get(y); }
        int size(int x) { return -e[get(x)]; }
        bool unite(int x, int y) {
            x = get(x), y = get(y); if (x == y) return false;
            if (e[x] > e[y]) swap(x, y);
            e[x] += e[y]; e[y] =x; return true;
        }
    };

    int n = graph.size();
    dsu d;
    d.init(n);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (graph[i][j]) {
                d.unite(i, j);
            }
        }
    }
    vector<int> cnt(n);
    for (int x : initial) {
        ++cnt[d.get(x)];
    }
    int ans = n, mx = 0;
    for (int x : initial) {
        int root = d.get(x);
        if (cnt[root] == 1) {
            int sz = d.size(root);
            if (sz > mx || (sz == mx && ans > x)) {
                ans = x;
                mx = sz;
            }
        }
    }
    return ans == n ? *min_element(initial.begin(), initial.end()) : ans;
}

auto min_cost_time(int maxTime, vector<vector<int>>& edges, vector<int>& passingFees) -> int {
    // Time O(maxTime * (n + m))
    // Space (maxTime*n)
    int m = maxTime, n = passingFees.size();
    const int inf = 1 << 30;
    vector<vector<int>> f(m+1, vector<int>(n, inf));
    f[0][0] = passingFees[0];
    for (int i = 1; i <= m; i++) {
        for (const auto& e : edges) {
            int x = e[0], y = e[1], t = e[2];
            if (t <= i) {
                f[i][x] = min(f[i][x], f[i-t][y] + passingFees[x]);
                f[i][y] = min(f[i][y], f[i-t][x] + passingFees[y]);
            }
        }
    }
    int ans = inf;
    for (int i = 1; i <= m; i++) {
        ans = min(ans, f[i][n-1]);
    }
    return ans == inf ? -1 : ans;
}

auto num_of_good_paths(vector<int>& vals, vector<vector<int>>& edges) -> int {
    // Time O(n log n)
	// Space O(n* m)
    int n = vals.size();
    vector<int> p(n);
    iota(p.begin(), p.end(),0 );
    vector<vector<int>> g(n);
    for (const auto& e : edges) {
        g[e[0]].push_back(e[1]);
        g[e[1]].push_back(e[0]);
    }
    function<int(int)> get;
    get = [&](int x) -> int {
        if (x == p[x]) return x;
        return p[x] = get(p[x]);
    };
    vector<pair<int,int>> arr(n);
    unordered_map<int, unordered_map<int,int>> size;
    for (int i = 0; i < n; i++) {
        arr[i] = {vals[i], i};
        size[i][vals[i]] = 1;
    }
    sort(arr.begin(), arr.end());
    int ans = n;
    for (auto& [v, x] : arr) {
        for (int y : g[x]) {
            if (vals[y] > v) continue;
            x = get(x), y = get(y);
            if (x != y) {
                ans += size[x][v]* size[y][v];
                p[y] = x;
                size[x][v] += size[y][v];
            }
        }
    }
    return ans;
}

auto find_shortese_edges(int n, vector<vector<int>>& edges) -> vector<bool> {
    // Time O(m log n)
    // Space O(n + m)
    int m = edges.size();
    vector<vector<array<int, 3>>> g(n);
    for (int i = 0; i < m; i++) {
        auto e = edges[i];
        int a = e[0], b = e[1], w = e[2];
        g[a].push_back({b, w, i});
        g[b].push_back({a, w, i});
    }
    const int inf = 1 << 30;
    vector<int> dist(n, inf);
    dist[0] = 0;
    using pii = pair<int,int>;
    priority_queue<pii, vector<pii>, greater<pii>> pq;
    pq.push({0,0});
    while (!pq.empty()) {
        auto [d, from] = pq.top();
        pq.pop();
        if (d != dist[from]) continue;

        for (const auto& [to, w, _] : g[from]) {
            if (dist[from] + w < dist[to]) {
                dist[to] = dist[from] + w;
                pq.push({dist[to], to});
            }
        }
    }
    vector<bool> ans(m, false);
    vector<char> vis(n,0);
    if (dist[n-1] == inf) {
        return ans;
    }
    queue<int> q;
    q.push(n-1);
    vis[n-1] = 1;
    while (!q.empty()) {
        int to = q.front();
        q.pop();
        for (const auto& [from, w, i] : g[to]) {
            if (dist[to] == dist[from] + w) {
                ans[i] = true;
                if (!vis[from]) {
                    vis[from] = 1;
                    q.push(from);
                }
            } 
        }
    }
    return ans;
}

auto count_path(int n, vector<vector<int>>& roads) -> int {
    // Time O(n^2)
    // Space O(n^2)
    const int mod = 1e9 + 7;
    const int64_t inf = 1e18;
    vector<vector<pair<int,int>>> g(n);
    for (const auto& e : roads) {
        g[e[0]].push_back({e[1], e[2]});
        g[e[1]].push_back({e[0], e[2]});
    }
    
    vector<int64_t> d(n, inf);
    vector<bool> u(n,false);
    vector<int64_t> f(n,0);

    d[0] = 0;
    f[0] = 1;

    for (int i = 0; i < n; i++) {
        int v = -1;
        for (int j = 0; j < n; j++) {
            if (!u[j] && (v == -1 || d[j] < d[v])) {
                v = j;
            }
        }

        if (d[v] == inf) continue;
        u[v] = true;
        
        for (auto& e : g[v]) {
            int to = e.first;
            int64_t len = e.second;

            if (d[v] + len < d[to]) {
                d[to] = d[v] + len;
                f[to] = f[v];
            } else if (d[to] == d[v] + len) {
                f[to] = (f[to] + f[v]) % mod;
            }
        }
    }
    return (int)f[n-1];
}

auto solve_sudoku(vector<vector<int>>& board) -> void {
    // Time O(9^m) - m is the size of empty cells
    // Space O(m)

    int row[9][9];
    int col[9][9];
    int box[3][3][9];
    vector<pair<int,int>> e;
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (board[i][j] == '.') {
                e.push_back({i,j});
            } else {
                int v = board[i][j] - '1';
                row[i][v] = col[j][v] = box[i/3][j/3][v] = true;
            }
        }
    }
    bool ok = false;
    auto dfs = [&](auto&& dfs, int k) -> void {
        if (k == e.size()) {
            ok = true;
            return;
        }
        int i = e[k].first, j = e[k].second;
        for (int v = 0; v < 9; v++) {
            if (!row[i][v] && !col[j][v] && !box[i/3][j/3][v]) {
                row[i][v] = col[j][v] = box[i/3][j/3][v] = true;
                board[i][j] = v + '1';
                dfs(dfs, k+1);
                row[i][v] = col[j][v] = box[i/3][j/3][v] = false;
            }
            if (ok) {
                return;
            }
        }
    };
    dfs(dfs,0);
}

int main() {
    
    GRAPH_PROBLEM problem = GRAPH_PROBLEM::MINIMIZE_MALWARE_SPREAD;

    switch (problem) {
        case SURROUNDING_XO: {
            vector<vector<char>> grid = {
                {'X', 'O', 'X', 'O', 'X', 'X'},
                {'X', 'O', 'X', 'X', 'O', 'X'},
                {'X', 'X', 'X', 'O', 'X', 'X'},
                {'O', 'X', 'X', 'X', 'X', 'X'},
                {'X', 'X', 'X', 'O', 'X', 'O'},
                {'O', 'O', 'X', 'O', 'O', 'O'},
            };
            fill(grid);
            for (auto& row : grid) {
                for (char c : row) {
                    cout << c << " ";
                }
                cout << endl;
            }
            cout<<'\n';
            break;
        }
        case TOPOLOGICAL_SORT: {
            auto add_edge = [&](vector<vector<int>>& e, int x, int y) -> void {
                e[x].push_back(y);
            };
            int n=6;
            // vector<vector<int>> adj(n);
            // add_edge(adj, 0, 1);
            // add_edge(adj, 1, 2);
            // add_edge(adj, 2, 3);
            // add_edge(adj, 4, 5);
            // add_edge(adj, 5, 1);
            // add_edge(adj, 5, 2);

            vector<vector<int>> edges = {
                {0, 1},
                {1, 2},
                {2, 3},
                {4, 5},
                {5, 1},
                {5, 2}
            };

            auto topo=toposort(n,edges);
            for (int x : topo) {
                cout<<x<<' ';
            }
            cout<<'\n';
            break;
        }
        case LINE_CHART: {
            vector<vector<int>>stockPrices{{1,7},{2,6},{3,5},{4,4},{5,4},{6,3},{7,2},{8,1}};
            cout<<minimum_lines(stockPrices)<<'\n';
            break;
        }
        case OBSTACLE_REMOVAL: {
            vector<vector<int>>grid{{0,1,1},{1,1,0},{1,1,0}};
            cout<<min_obstacles(grid)<<'\n';
            vector<vector<int>> grid2{{0,1,0,0,0},{0,1,0,1,0},{0,0,0,1,0}};
            cout<<min_obstacles(grid2)<<'\n';
        }
        case INCREASING_PATHS: {
            vector<vector<int>>grid{{1,1},{3,4}};
            cout<<count_paths(grid)<<'\n';
            break;
        }
        case MINIMIZE_MALWARE_SPREAD: {
            vector<vector<int>> graph{{1,1,0},{1,1,0},{0,0,1}};
            vector<int> in {0,1};
            cout << min_malware_spread(graph, in) << '\n';
        }
        default:
            break;
    }
    return 0;
}