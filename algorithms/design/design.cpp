#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <set>
using namespace std;

enum Problem {
    TASK=0,
};

class TaskManager {
    // Time O(nlogn)
    // Space O(n + m)
public:
    TaskManager(vector<vector<int>>& tasks) {
        for (const auto& t : tasks) {
            add(t[0],t[1],t[2]);
        }
    }
    
    void add(int userId, int taskId, int priority) {
        mp[taskId] = {userId, priority};
        st.insert({-priority, -taskId});
    }
    
    void edit(int taskId, int newPriority) {
        auto [userId, prirority] = mp[taskId];
        st.erase({-prirority, -taskId});
        st.insert({-newPriority, -taskId});
        mp[taskId] = {userId, newPriority};
    }
    
    void rmv(int taskId) {
        auto [userId, priority] = mp[taskId];
        st.erase({-priority, -taskId});
        mp.erase(taskId);
    }
    
    int execTop() {
        if (st.empty()) {
            return -1;
        }
        auto e = *st.begin();
        st.erase(st.begin());
        int taskId = -e.second;
        int userId = mp[taskId].first;
        mp.erase(taskId);
        return userId;
    }

private:
    unordered_map<int, pair<int,int>> mp;
    set<pair<int,int>>st;
};


int main() {

    Problem problem = Problem::TASK;

    switch (problem) {
        case TASK: {
            vector<vector<int>> tasks{
                {1, 101, 10},
                {2, 102, 20},
                {3, 103, 15}
            };
            TaskManager taskManager(tasks);

            vector<string> output{"null"};

            taskManager.add(4, 104, 5);
            output.push_back("null");

            taskManager.edit(102, 8);
            output.push_back("null");

            output.push_back(to_string(taskManager.execTop()));

            taskManager.rmv(101);
            output.push_back("null");

            taskManager.add(5, 105, 15);
            output.push_back("null");

            output.push_back(to_string(taskManager.execTop()));

            cout << '[';
            for (int i = 0; i < output.size(); i++) {
                if (i > 0) {
                    cout << ", ";
                }
                cout << output[i];
            }
            cout << ']' << '\n';
            break;
        }
        default:
            break;
    }

    return 0;
}
