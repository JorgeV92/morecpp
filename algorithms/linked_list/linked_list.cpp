#include <iostream>
#include <vector>
using namespace std;

enum LINKED_PROBLEMS {
    SWAP_ELEMENTS=0,
    COUNT_ELEMENT,
};

struct Node {
    int data;
    Node* next;

    Node(int v) : data(v), next(nullptr){}
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

int main() {

    LINKED_PROBLEMS problem = LINKED_PROBLEMS::COUNT_ELEMENT;

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
        default:
            break;
    }

    return 0;
}

