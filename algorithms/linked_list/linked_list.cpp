#include <iostream>
#include <vector>
#include <stack>
using namespace std;

enum LINKED_PROBLEMS {
    // EASY
    SWAP_ELEMENTS=0,
    COUNT_ELEMENT,
    STACK,
    // MEDIUM
    DETECT_CYCLE,
    LENGTH_OF_CYCLE,
    BT_TO_DLL,
};

struct Node {
    int data;
    Node* next;
    Node* prev;

    Node(int v) : data(v), next(nullptr), prev(nullptr) {}
};

struct tree_node {
    int data;
    tree_node* left;
    tree_node* right;

    tree_node(int x) : data(x), left(nullptr), right(nullptr) {}
};

auto print_list(Node* head) -> void {
    Node* cur=head;
    while (cur) {
        cout << cur->data << " ";
        if (cur->next) {
            cout << "-> ";
        }
        cur=cur->next;
    }
    cout << '\n';
}

auto print_tree_list(tree_node* root) -> void {
    tree_node* node = root;
    while (node) {
        cout << node->data << " ";
        node=node->right;
    }
}

auto pair_wise_swap(Node* head) -> void {
    // time O(n)
    // space O(n)
    Node* cur = head;
    while (cur&&cur->next) {
        swap(cur->data,cur->next->data);
        cur=cur->next->next;
    }
}

auto count_element(Node* head, int k) -> int {
    // time O(n)
    // space O(1)
    Node* cur = head;
    int cnt = 0;
    while (cur) {
        if (cur->data==k) cnt++;
        cur=cur->next;
    }
    return cnt;
}

class MyStack {
public:
    // time O(1)
    // space O(1)
    MyStack() : cnt(0), top(nullptr) {}

    void push(int x) {
        Node* tmp = new Node(x);
        tmp->next = top;
        top = tmp;
        cnt++;
    }

    int pop() {
        if (empty()) {
            throw runtime_error("empty stack");
        }
        auto tmp = top;
        top = top->next;
        int v = tmp->data;
        delete tmp;
        cnt--;
        return v;
    }

    int peek() const {
        if (empty()) {
            throw runtime_error("stack empty");
        }
        return top->data;
    }

    bool empty() const {
        return top == nullptr;
    }

    int size() const {
        return cnt;
    }

private:
    Node* top;
    int cnt;
};

auto detect_cycle(Node* head) -> bool {
    // time O(n)
    // space O(1)
    Node* slow = head;
    Node* fast = head;
    while (slow&&fast&&fast->next) {
        slow = slow->next;
        fast=fast->next->next;
        if (slow==fast) {
            return true;
        }
    }
    return false;
}

auto length_of_loop(Node* head) -> int {
    // time O(n)
    // space O(1)
    auto count_nodes = [&](Node* node) -> int {
        Node* cur=node;
        int cnt=1;
        while (cur->next != node) {
            cnt++;
            cur=cur->next;
        }
        return cnt;
    };
    Node* slow = head;
    Node* fast = head;
    while (slow && fast && fast->next) {
        slow=slow->next;
        fast=fast->next->next;
        if (slow==fast) {
            return count_nodes(slow);
        }
    }
    return 0;
}

auto b_to_dll_stack(tree_node* root) -> tree_node* {
    // time O(n)
    // space O(n)
    stack<pair<tree_node*, int>> st;
    st.push({root, 0});
    vector<int> res;
    tree_node* head = nullptr;
    tree_node* prev = nullptr;
    bool flag = true;
    while (!st.empty()) {
        auto p = st.top();
        st.pop();
        auto t = p.first;
        int state = p.second;
        if (state == 3 or !t) continue;
        st.push({t, state+1});
        if (state == 0) st.push({t->left, 0});
        else if (state==1) {
            if (prev) prev->right = t;
            t->left = prev;
            prev = t;
            if (flag) {
                head = t;
                flag = false;
            }
        } else if (state==2) st.push({t->right, 0});
    }
    return head;
}

auto bt_to_dll_morris(tree_node* root) -> tree_node* {
    // time O(n)
    // space O(1)
    tree_node* cur=root;
    tree_node* prev=nullptr;
    tree_node* head=nullptr;

    while (cur) {
        if (!cur->left) {
            if (!head) {
                prev=cur;
                head=cur;
            } else {
                prev->right=cur;
                cur->left=prev;
            }
            prev=cur;
            cur=cur->right;
        } else {
            tree_node* pre=cur->left;
            while (pre->right&&pre->right!=cur) {
                pre=pre->right;
            }
            if (!pre->right) {
                pre->right=cur;
                cur=cur->left;
            } else {
                cur=prev->right;
                prev->right=cur;
                cur->left=prev;
                prev=cur;
                cur=cur->right;
            }
        }
    }
    return head;
}

int main() {

    LINKED_PROBLEMS problem = LINKED_PROBLEMS::BT_TO_DLL;

    switch (problem) {
        case SWAP_ELEMENTS: {
            Node* head = new Node(1);
            head->next = new Node(2);
            head->next->next = new Node(3);
            head->next->next->next = new Node(4);
            head->next->next->next->next = new Node(5);
            head->next->next->next->next->next = new Node(6);
            pair_wise_swap(head);
            print_list(head);
            break;
        }
        case COUNT_ELEMENT: {
            Node* head = new Node(1);
            head->next = new Node(2);
            head->next->next = new Node(1);
            head->next->next->next = new Node(2);
            head->next->next->next->next = new Node(1);
	
  	        int key = 1;
            cout << count_element(head, key) << "\n";
            break;
        }
        case STACK: {
            MyStack st;
            st.push(1);
            st.push(2);
            st.push(3);
            st.push(4);

            cout << "Popped: " << st.pop() << endl;
            cout << "Top element: " << st.peek() << endl;
            cout << "Is stack empty: " << (st.empty() ? "Yes" : "No") << endl;
            cout << "Current size: " << st.size() << endl;
            break;
        }
        case DETECT_CYCLE: {
            Node* head = new Node(1);
            head->next = new Node(3);
            head->next->next = new Node(4);
        
            head->next->next->next = head->next;

            if (detect_cycle(head)) cout << "true";
            else cout << "false";
            break;
        }
        case LENGTH_OF_CYCLE: {
            Node* head = new Node(25);
            head->next = new Node(14);
            head->next->next = new Node(19);
            head->next->next->next = new Node(33);
            head->next->next->next->next = new Node(10);
            
            head->next->next->next->next->next = head->next->next;
            
            cout << length_of_loop(head) << endl;
            break;
        }
        case BT_TO_DLL: {
            tree_node* root = new tree_node(10);
            root->left = new tree_node(12);
            root->right = new tree_node(15);
            root->left->left = new tree_node(25);
            root->left->right = new tree_node(30);
            root->right->left = new tree_node(36);

            // tree_node* head = b_to_dll_stack(root);
            tree_node* head = bt_to_dll_morris(root);
            print_tree_list(head);
            break;
        }
        default:
            break;
    }

    return 0;
}

