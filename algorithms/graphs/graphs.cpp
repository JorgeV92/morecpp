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
using namespace std;

enum GRAPH_PROBLEM {
    // EASY
    SURROUNDING_XO=0,
    TOPOLOGICAL_SORT,
    // MEDIUM 
    LINE_CHART,
    // HARD
    OBSTACLE_REMOVAL,
    INCREASING_PATHS,
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

int main() {
    
    GRAPH_PROBLEM problem = GRAPH_PROBLEM::INCREASING_PATHS;

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
        default:
            break;
    }
    return 0;
}