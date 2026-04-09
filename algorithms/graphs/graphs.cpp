#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <map>
#include <set>
#include <cstdint>
#include <algorithm>
#include <numeric>
using namespace std;

enum GRAPH_PROBLEM {
    // EASY
    SURROUNDING_XO=0,
    TOPOLOGICAL_SORT,
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

int main() {
    
    GRAPH_PROBLEM problem = GRAPH_PROBLEM::TOPOLOGICAL_SORT;

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
        default:
            break;
    }
    
    return 0;
}