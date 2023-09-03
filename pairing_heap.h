#pragma once

#include <functional> // std::less

#ifndef release_no_debug

#include "debug.h"

using namespace std;

template <typename T, typename CMP_T = less<T>>
class PairingHeap {
    struct Node {
        Node() = default;
        Node(T val_) : val(val_) {}
        Node *bro = nullptr;
        Node *son = nullptr;
        T val;
    };
 public:
    PairingHeap() = default;
    explicit PairingHeap(T v) : root(new Node(v)), sz(1) {}
    ~PairingHeap() {
        del_tree(root);
    }
    void push(T val) {
        ++sz;
        root = merge_2_node(root, new Node(val));
    }
    // 调用前必须检查 !empty()
    T top() const {
        return root->val;
    }
    void pop() {
        // 不做 empty 检查
        Node *son = root->son;
        delete root;
        sz -= 1;
        root = meld_bros(son);
    }
    [[nodiscard]] int size() const {
        return sz;
    }
    [[nodiscard]] bool empty() const {
        return sz == 0;
    }
    void clear() {
        del_tree();
        root = nullptr;
        sz = 0;
    }
    void test() {
#ifndef release_no_debug
        traversal(root);
#endif
    }

 private:
    // 默认 root, root2 的 bro 是 nullptr
    Node* merge_2_node(Node *root, Node *root2) {
        if (!root2) return root;
        if (!root) return root2;

        if (!cmp(root->val, root2->val)) {
            swap(root, root2);
        }
        return merge_node_sorted(root, root2);
    }
    // 默认 root, root2 的 bro 是 nullptr
    // 默认root->val < root2->val
    Node* merge_node_sorted(Node *root, Node *root2) {
        root2->bro = root->son;
        root->son = root2;
        return root;
    }
    // root 已经去除, 变成了散的一堆
    // 合并所有兄弟
    Node* meld_bros(Node *root) {
        // 递归法
        if (!root || !root->bro) return root;
        Node *node = root->bro, *c = node->bro;
        root->bro = nullptr;
        node->bro = nullptr;
        // 两两合并, 再从右往左合并(最左边深度最浅)
        return merge_2_node(merge_2_node(root, node), meld_bros(c));
    }
    void del_tree(Node *root) {
        if (!root) return;

        del_tree(root->son);
        del_tree(root->bro);
        delete root;
        deO("delete %p", root);
    }
    void traversal(Node *root) {
#ifndef release_no_debug
        if (!root) return;

        deO("val: %d, me: %p, son: %p, bro: %p", root->val, root, root->son, root->bro)
        traversal(root->son);
        traversal(root->bro);
#endif
    }

    Node *root = nullptr;
    int sz = 0;
    CMP_T cmp;
};

// 参考: https://oi-wiki.org/ds/pairing-heap/

#else

#include "ext/pb_ds/priority_queue.hpp"

template <typename T, typename CMP_T = std::less<T>>
using PairingHeap = __gnu_pbds::priority_queue<T, CMP_T, __gnu_pbds::pairing_heap_tag>;

#endif