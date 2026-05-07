#pragma once 

#include <vector>
#include <queue>
#include <iostream>

void graph_is_bipartite(std::vector<std::vector<int>>& adj) {
    int n = adj.size();
    
    std::vector<int> side(n, -1);
    // Checks for all componenets if the graph 
    // is not fully connected.
    for (int st = 0; st < n; st++) {
        if (side[st] == -1) {
            std::queue<int> q;
            q.push(st);
            side[st] = 0;

            bool is_bipartite = true;
            while (!q.empty()) {
                int x = q.front();
                q.pop();

                for (auto y : adj[x]) {
                    if (side[y] == -1) {
                        side[y] = side[x] ^ 1;
                        q.push(y);
                    } else {
                        is_bipartite &= side[y] != side[x];
                    }
                }
            }
            if (is_bipartite) {
                std::cout << "this component is bipartite\n";
            } else {
                std::cout << "this component is not bipartie\n";
            }
        }
    }
}