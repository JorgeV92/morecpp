#include <iostream>
#include <stdexcept>
using namespace std;

class DoublyLinkedList {
public:
    struct Node {
        int val;
        Node* prev;
        Node* next;

        Node(int v) : val(v), prev(nullptr), next(nullptr) {}
    };

private:
    Node* head;
    Node* tail;
    int sz;

public:
    DoublyLinkedList() : head(nullptr), tail(nullptr), sz(0) {}

    ~DoublyLinkedList() {
        clear();
    }

    int size() const {
        return sz;
    }

    bool empty() const {
        return sz == 0;
    }

    Node* getHead() const {
        return head;
    }

    Node* getTail() const {
        return tail;
    }

    int front() const {
        if (empty()) throw runtime_error("List is empty");
        return head->val;
    }

    int back() const {
        if (empty()) throw runtime_error("List is empty");
        return tail->val;
    }

    Node* push_front(int value) {
        Node* node = new Node(value);

        if (empty()) {
            head = tail = node;
        } else {
            node->next = head;
            head->prev = node;
            head = node;
        }

        sz++;
        return node;
    }

    Node* push_back(int value) {
        Node* node = new Node(value);

        if (empty()) {
            head = tail = node;
        } else {
            tail->next = node;
            node->prev = tail;
            tail = node;
        }

        sz++;
        return node;
    }

    void pop_front() {
        if (empty()) return;

        Node* oldHead = head;

        if (head == tail) {
            head = tail = nullptr;
        } else {
            head = head->next;
            head->prev = nullptr;
        }

        delete oldHead;
        sz--;
    }

    void pop_back() {
        if (empty()) return;

        Node* oldTail = tail;

        if (head == tail) {
            head = tail = nullptr;
        } else {
            tail = tail->prev;
            tail->next = nullptr;
        }

        delete oldTail;
        sz--;
    }

    Node* insert_after(Node* pos, int value) {
        if (pos == nullptr) {
            return push_front(value);
        }

        if (pos == tail) {
            return push_back(value);
        }

        Node* node = new Node(value);

        Node* after = pos->next;

        node->prev = pos;
        node->next = after;

        pos->next = node;
        after->prev = node;

        sz++;
        return node;
    }

    Node* insert_before(Node* pos, int value) {
        if (pos == nullptr) {
            return push_back(value);
        }

        if (pos == head) {
            return push_front(value);
        }

        Node* node = new Node(value);

        Node* before = pos->prev;

        node->prev = before;
        node->next = pos;

        before->next = node;
        pos->prev = node;

        sz++;
        return node;
    }

    void erase(Node* node) {
        if (node == nullptr) return;

        if (node == head) {
            pop_front();
            return;
        }

        if (node == tail) {
            pop_back();
            return;
        }

        Node* before = node->prev;
        Node* after = node->next;

        before->next = after;
        after->prev = before;

        delete node;
        sz--;
    }

    Node* find(int value) const {
        Node* curr = head;

        while (curr != nullptr) {
            if (curr->val == value) {
                return curr;
            }

            curr = curr->next;
        }

        return nullptr;
    }

    bool contains(int value) const {
        return find(value) != nullptr;
    }

    void clear() {
        Node* curr = head;

        while (curr != nullptr) {
            Node* nextNode = curr->next;
            delete curr;
            curr = nextNode;
        }

        head = nullptr;
        tail = nullptr;
        sz = 0;
    }

    void print_forward() const {
        Node* curr = head;

        while (curr != nullptr) {
            cout << curr->val << " ";
            curr = curr->next;
        }

        cout << "\n";
    }

    void print_backward() const {
        Node* curr = tail;

        while (curr != nullptr) {
            cout << curr->val << " ";
            curr = curr->prev;
        }

        cout << "\n";
    }
};

int main() {
    DoublyLinkedList list;

    auto n1 = list.push_back(10);
    auto n2 = list.push_back(20);
    auto n3 = list.push_back(30);

    list.push_front(5);

    list.print_forward();   // 5 10 20 30
    list.print_backward();  // 30 20 10 5

    list.insert_after(n2, 25);
    list.print_forward();   // 5 10 20 25 30

    list.insert_before(n1, 7);
    list.print_forward();   // 5 7 10 20 25 30

    list.erase(n2);
    list.print_forward();   // 5 7 10 25 30

    list.pop_front();
    list.pop_back();

    list.print_forward();   // 7 10 25

    cout << "Size: " << list.size() << "\n";
}