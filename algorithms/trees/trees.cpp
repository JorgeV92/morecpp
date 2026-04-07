#include <iostream>
#include <vector>
#include <queue>
using namespace std;

enum TREE_PROBLEMS {
    MAX_HEIGHT_BT=0,
};

struct tree_node {
    int data;
    tree_node* left;
    tree_node* right;

    tree_node(int x) : data(x), left(nullptr), right(nullptr) {}
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

int main() {

    TREE_PROBLEMS problem = TREE_PROBLEMS::MAX_HEIGHT_BT;

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
        default:
            break;
    }

    return 0;
}