#include <iostream>
#include <vector>
#include <queue>
using namespace std;

enum TREE_PROBLEMS {
    // EASY
    MAX_HEIGHT_BT=0,
    CHILDREN_SUM,
    // MEDIUM
    CONNECT_NODES,
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

int main() {

    TREE_PROBLEMS problem = TREE_PROBLEMS::CONNECT_NODES;

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
        default:
            break;
    }

    return 0;
}