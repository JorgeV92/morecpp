#include <iostream>
#include <vector>
#include <queue>
#include <functional>
#include <unordered_map>
using namespace std;

enum TREE_PROBLEMS {
    // EASY
    MAX_HEIGHT_BT=0,
    CHILDREN_SUM,
    // MEDIUM
    CONNECT_NODES,
    LCA_BT,
    MAXIMUM_SUM_LEAF,
    // HARD
    K_SUM_PATHS,
    MAX_PATH_SUM_TWO_NODES,
};

struct tree_node {
    int data;
    tree_node* left;
    tree_node* right;
    tree_node* right_next;
    tree_node(int x) : data(x), left(nullptr), right(nullptr), right_next(nullptr) {}
};

auto height_recursion(tree_node* node) -> int {
    // time O(n)
    // space O(n)
    if (!node)return -1;
    int l=height_recursion(node->left);
    int r=height_recursion(node->right);
    return 1+max(l,r);
}

auto height_levelorder(tree_node* root) -> int {
    // time O(n)
    // space O(n)
    if (!root) return 0;
    queue<tree_node*> q;
    q.push(root);
    int depth=0;
    while (!q.empty()) {
        int len=q.size();
        for (int k=0;k<len;k++) {
            auto node=q.front();
            q.pop();
            if (node->left) q.push(node->left);
            if (node->right) q.push(node->right);
        }
        depth++;
    }
    return depth-1;
}

auto children_sum_recursion(tree_node* root) {

    auto dfs = [&](auto&& dfs, tree_node* node)  -> bool {
        if (!node || (!node->left && !node->right)) return true;
        int sum=0;
        if (node->left)
            sum+=node->left->data;
        if (node->right)
            sum+=node->right->data;
        return (sum==node->data && dfs(dfs, node->left) && dfs(dfs, node->right));
    };

    return dfs(dfs, root);
}

auto children_sum_queue(tree_node* root) -> bool {
    if (!root) return true;
    queue<tree_node*> q{{root}};
    while (!q.empty()) {
        auto node=q.front();
        q.pop();
        if (!node->left && !node->right) continue;
        int sum=0;
        if (node->left) sum+=node->left->data;
        if (node->right) sum+=node->right->data;
        if (sum!=node->data) return false;
        if (node->left) q.push(node->left);
        if (node->right) q.push(node->right);
    }
    return true;
}

auto connect_nodes(tree_node* root) -> tree_node* {
    // time O(n)
    // space O(n)
    if (!root) return root;
    queue<tree_node*> q{{root}};
    while (!q.empty()) {
        int len=q.size();
        for (int i=0;i<len;i++){
            auto node=q.front();
            q.pop();
            if (i==len-1) {
                node->right_next=nullptr;
            } else{
                node->right_next=q.front();
            }
            if (node->left) q.push(node->left);
            if (node->right) q.push(node->right);
        }
    }
    return root;
}

auto lca(tree_node* root, tree_node* t1, tree_node* t2) -> tree_node* {
    // time O(n)
    // space O(n)
    if (!root) return nullptr;
    if (root==t1||root==t2) return root;
    auto l_lca=lca(root->left, t1, t2);
    auto r_lca=lca(root->right,t1,t2);
    if (l_lca&&r_lca) return root;
    return l_lca ? l_lca : r_lca;
}

auto max_path_sum(tree_node* root) -> int {
    int mx_path=-1e9;

    function<void(tree_node*,int)> f = [&](tree_node* node, int sum) -> void {
        if (!node) return;
        sum+=node->data;
        if (!node->left&&!node->right) {
            mx_path=max(mx_path,sum);
        }
        f(node->left,sum);
        f(node->right,sum);
    };

    f(root, 0);
    return mx_path;
}

// Given the root of a binary tree and an integer k, Count the number of paths in the tree such that the sum of the nodes in each path equals k.
auto count_all_paths(tree_node* root, int k) -> int {
    // time O(n)
    // space O(n)
    unordered_map<int,int> pref;

    function<int(tree_node*,int)> f = [&](tree_node* node, int sum) -> int {
        if (!node) return 0;
        int path=0;
        sum+=node->data;
        if (sum==k) path++;
        path+=pref[sum-k];
        pref[sum]++;
        path+=f(node->left,sum);
        path+=f(node->right,sum);
        pref[sum]--;
        return path;
    };

    return f(root,0);
}

// Given a binary tree in which each node element contains a number. Find the maximum possible sum from one leaf node to another. 
auto max_path_sum_two_nodes(tree_node* root) -> int {
    // time O(N)
    // space O(h)
    int mx_path=-1e9;

    function<int(tree_node*)> f = [&](tree_node* node) -> int {
        if (!node) return 0;
        int l=f(node->left);
        int r=f(node->right);
        if (node->left&&node->right) {
            int mx = l+r+node->data;
            mx_path=max(mx_path,mx);
            return node->data+max(l,r);
        }
        return node->data+(l?l:r);
    };

    f(root);
    return mx_path;
}

int main() {

    TREE_PROBLEMS problem = TREE_PROBLEMS::MAX_PATH_SUM_TWO_NODES;

    switch (problem) {
        case MAX_HEIGHT_BT: {
            tree_node *root = new tree_node(12);
            root->left = new tree_node(8);
            root->right = new tree_node(18);
            root->left->left = new tree_node(5);
            root->left->right = new tree_node(11);
            cout << height_levelorder(root) << '\n';
            break;
        }
        case CHILDREN_SUM: {
            tree_node* root = new tree_node(35);
            root->left = new tree_node(20);
            root->right = new tree_node(15);
            root->left->left = new tree_node(15);
            root->left->right = new tree_node(5);
            root->right->left = new tree_node(10);
            root->right->right = new tree_node(5);

            cout << children_sum_queue(root) << endl;
            break;
        }
        case CONNECT_NODES: {
            tree_node *root = new tree_node(10);
            root->left = new tree_node(8);
            root->right = new tree_node(2);
            root->left->left = new tree_node(3);
            root->right->left = new tree_node(4);

            connect_nodes(root);

            cout << "Next Right of 8 is " << root->left->right_next->data << endl;
            cout << "Next Right of 3 is " << root->left->left->right_next->data << endl;
            break;
        }
        case LCA_BT: {
            tree_node* root = new tree_node(1);
            root->left = new tree_node(2);
            root->right = new tree_node(3);
            root->right->left = new tree_node(6);
            root->right->right = new tree_node(7);
            root->right->left->left = new tree_node(8);

            tree_node* n1 = root->right->right;
            tree_node* n2 = root->right->left->left;

            tree_node* ans = lca(root, n1, n2);
            cout<<ans->data<<'\n';
            break;
        }
        case MAXIMUM_SUM_LEAF: {
            tree_node *root = new tree_node(10);
            root->left = new tree_node(-2);
            root->right = new tree_node(7);
            root->left->left = new tree_node(8);
            root->left->right = new tree_node(-4);

            int sum = max_path_sum(root);
            cout << sum << endl;
            break;
        }
        case K_SUM_PATHS: {
            tree_node* root = new tree_node(8);
            root->left = new tree_node(4);
            root->right = new tree_node(5);
            root->left->left = new tree_node(3);
            root->left->right = new tree_node(2);
            root->right->right = new tree_node(2);
            root->left->left->left = new tree_node(3);
            root->left->left->right = new tree_node(-2);
            root->left->right->right = new tree_node(1);

            int k = 7; 
            cout<<count_all_paths(root, k)<<endl;
            break;
        }
        case MAX_PATH_SUM_TWO_NODES: {
            tree_node* root = new tree_node(1);
            root->left = new tree_node(-2);
            root->right = new tree_node(3);
            root->left->left = new tree_node(8);
            root->left->right = new tree_node(-1);
            root->right->left = new tree_node(4);
            root->right->right = new tree_node(-5);

            int result = max_path_sum_two_nodes(root);
            cout<<result<<endl;
            break;
        }
        default:
            break;
    }

    return 0;
}