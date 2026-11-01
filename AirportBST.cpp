#include "AirportBST.h"

#include <ostream>
#include <queue>
#include <stdexcept>
#include <utility>

AirportBST::~AirportBST() { clear(); }

void AirportBST::clear() {
    // Rotate away left links, then delete along right links: no recursion/stack.
    while (root_ != nullptr) {
        if (root_->left != nullptr) {
            Node* promoted = root_->left;
            root_->left = promoted->right;
            promoted->right = root_;
            root_ = promoted;
        } else {
            Node* next = root_->right;
            delete root_;
            root_ = next;
        }
    }
    size_ = 0;
}

void AirportBST::swap(AirportBST& other) noexcept {
    std::swap(root_, other.root_);
    std::swap(size_, other.size_);
}

std::string AirportBST::rootCode() const {
    return root_ == nullptr ? "" : root_->airport.code;
}

// Task 1: see docs/pseudocode.md, Build and Insert (I1-I4).
bool AirportBST::insert(const Airport& airport) {
    if (!validCode(airport.code)) throw std::invalid_argument("Invalid airport code");
    Node** link = &root_;
    while (*link != nullptr) {
        int comparison = airport.code.compare((*link)->airport.code);
        if (comparison == 0) return false; // I4: reject; preserve existing record.
        link = comparison < 0 ? &(*link)->left : &(*link)->right;
    }
    *link = new Node(airport);
    size_++;
    return true;
}

// Task 2: iterative search; one logical three-way key comparison per node.
const Airport* AirportBST::lookup(const std::string& code, int* comparisons) const {
    if (comparisons != nullptr) *comparisons = 0;
    const Node* current = root_;
    while (current != nullptr) {
        if (comparisons != nullptr) (*comparisons)++;
        int comparison = code.compare(current->airport.code);
        if (comparison == 0) return &current->airport;
        current = comparison < 0 ? current->left : current->right;
    }
    return nullptr;
}

SearchResult AirportBST::search(const std::string& code) const {
    SearchResult result;
    const Node* current = root_;
    while (current != nullptr) {
        result.path.push_back(current->airport.code);
        int comparison = code.compare(current->airport.code);
        if (comparison == 0) {
            result.found = true;
            result.airport = current->airport;
            break;
        }
        current = comparison < 0 ? current->left : current->right;
    }
    return result;
}

const Airport* AirportBST::minimum() const {
    const Node* current = root_;
    if (current == nullptr) return nullptr;
    while (current->left != nullptr) current = current->left;
    return &current->airport;
}

const Airport* AirportBST::maximum() const {
    const Node* current = root_;
    if (current == nullptr) return nullptr;
    while (current->right != nullptr) current = current->right;
    return &current->airport;
}

// Task 3: D1, both orientations of D2, D3; the same link logic handles the root.
DeleteResult AirportBST::remove(const std::string& code) {
    DeleteResult result;
    Node** link = &root_;
    while (*link != nullptr && (*link)->airport.code != code) {
        link = code < (*link)->airport.code ? &(*link)->left : &(*link)->right;
    }
    if (*link == nullptr) return result;
    Node* target = *link;
    result.removed = true;
    if (target->left != nullptr && target->right != nullptr) {
        result.caseName = "D3: two children";
        Node** successorLink = &target->right;
        while ((*successorLink)->left != nullptr) {
            successorLink = &(*successorLink)->left;
        }
        Node* successor = *successorLink;
        result.replacement = successor->airport.code;
        target->airport = successor->airport; // Copy the complete record, not just key.
        *successorLink = successor->right;
        delete successor;
    } else {
        Node* child = target->left != nullptr ? target->left : target->right;
        if (child == nullptr) result.caseName = "D1: leaf";
        else if (target->left != nullptr) result.caseName = "D2: only a left child";
        else result.caseName = "D2: only a right child";
        if (child != nullptr) result.replacement = child->airport.code;
        *link = child;
        delete target;
    }
    size_--;
    return result;
}

TreeStats AirportBST::stats() const {
    TreeStats result;
    std::vector<std::pair<const Node*, int>> pending;
    if (root_ != nullptr) pending.push_back({root_, 0});
    while (!pending.empty()) {
        const Node* node = pending.back().first;
        int depth = pending.back().second;
        pending.pop_back();
        result.size++;
        result.depthSum += static_cast<std::uint64_t>(depth);
        if (depth > result.height) result.height = depth;
        if (node->right != nullptr) pending.push_back({node->right, depth + 1});
        if (node->left != nullptr) pending.push_back({node->left, depth + 1});
    }
    if (result.size > 0) {
        result.averageDepth = static_cast<double>(result.depthSum) / result.size;
    }
    return result;
}

// Task 4: strict ancestor bounds, not merely comparisons with each parent.
bool AirportBST::validate() const {
    struct Frame {
        const Node* node;
        const std::string* lo;
        const std::string* hi;
    };
    std::vector<Frame> pending;
    if (root_ != nullptr) pending.push_back({root_, nullptr, nullptr});
    int count = 0;
    while (!pending.empty()) {
        Frame frame = pending.back();
        pending.pop_back();
        const std::string& key = frame.node->airport.code;
        if ((frame.lo != nullptr && key <= *frame.lo) ||
            (frame.hi != nullptr && key >= *frame.hi)) return false;
        count++;
        if (frame.node->right != nullptr) {
            pending.push_back({frame.node->right, &key, frame.hi});
        }
        if (frame.node->left != nullptr) {
            pending.push_back({frame.node->left, frame.lo, &key});
        }
    }
    return count == size_;
}

std::vector<Airport> AirportBST::inorder() const {
    std::vector<Airport> result;
    std::vector<const Node*> pending;
    const Node* current = root_;
    while (current != nullptr || !pending.empty()) {
        while (current != nullptr) {
            pending.push_back(current);
            current = current->left;
        }
        current = pending.back();
        pending.pop_back();
        result.push_back(current->airport);
        current = current->right;
    }
    return result;
}

std::vector<Airport> AirportBST::preorder() const {
    std::vector<Airport> result;
    std::vector<const Node*> pending;
    if (root_ != nullptr) pending.push_back(root_);
    while (!pending.empty()) {
        const Node* current = pending.back();
        pending.pop_back();
        result.push_back(current->airport);
        if (current->right != nullptr) pending.push_back(current->right);
        if (current->left != nullptr) pending.push_back(current->left);
    }
    return result;
}

std::vector<std::string> AirportBST::levelOrder() const {
    std::vector<std::string> result;
    std::queue<const Node*> pending;
    if (root_ != nullptr) pending.push(root_);
    while (!pending.empty()) {
        const Node* current = pending.front();
        pending.pop();
        result.push_back(current->airport.code);
        if (current->left != nullptr) pending.push(current->left);
        if (current->right != nullptr) pending.push(current->right);
    }
    return result;
}

std::vector<std::string> AirportBST::shapeSignature() const {
    // Null markers distinguish left-only and right-only shapes unambiguously.
    std::vector<std::string> result;
    std::vector<const Node*> pending = {root_};
    while (!pending.empty()) {
        const Node* current = pending.back();
        pending.pop_back();
        if (current == nullptr) result.push_back("#");
        else {
            result.push_back(current->airport.code);
            pending.push_back(current->right);
            pending.push_back(current->left);
        }
    }
    return result;
}

void AirportBST::printLevels(std::ostream& out, int count) const {
    if (root_ == nullptr) { out << "(empty)\n"; return; }
    std::queue<const Node*> pending;
    pending.push(root_);
    for (int depth = 0; depth < count && !pending.empty(); depth++) {
        int width = static_cast<int>(pending.size());
        out << "Level " << depth << ':';
        for (int i = 0; i < width; i++) {
            const Node* node = pending.front();
            pending.pop();
            out << ' ' << (node == nullptr ? "-" : node->airport.code);
            if (node != nullptr) {
                pending.push(node->left);
                pending.push(node->right);
            }
        }
        out << '\n';
    }
}

// Task 5: pruned inorder; visited counts each examined non-null node once.
RangeResult AirportBST::range(const std::string& from, const std::string& to) const {
    RangeResult result;
    if (from > to) return result;
    const Node* current = root_;
    std::vector<const Node*> pending;
    while (current != nullptr || !pending.empty()) {
        while (current != nullptr) {
            result.visited++;
            const std::string& key = current->airport.code;
            if (key < from) current = current->right;
            else if (key > to) current = current->left;
            else {
                pending.push_back(current);
                current = key > from ? current->left : nullptr;
            }
        }
        if (!pending.empty()) {
            current = pending.back();
            pending.pop_back();
            result.airports.push_back(current->airport);
            current = current->airport.code < to ? current->right : nullptr;
        }
    }
    return result;
}

// Task 6: recursion is safe here (depth O(log n)); other traversals are iterative.
AirportBST::Node* AirportBST::buildMiddle(const std::vector<Airport>& sorted,
                                        int lo, int hi) {
    if (lo >= hi) return nullptr; // Half-open interval [lo, hi).
    int middle = lo + (hi - lo) / 2;
    Node* node = new Node(sorted[middle]);
    try {
        node->left = buildMiddle(sorted, lo, middle);
        node->right = buildMiddle(sorted, middle + 1, hi);
    } catch (...) {
        AirportBST cleanup;
        cleanup.root_ = node;
        throw;
    }
    return node;
}

void AirportBST::buildBalanced(const std::vector<Airport>& sorted) {
    int n = static_cast<int>(sorted.size());
    for (int i = 0; i < n; i++) {
        if (!validCode(sorted[i].code) ||
            (i > 0 && sorted[i - 1].code >= sorted[i].code)) {
            throw std::invalid_argument("Balanced build needs sorted unique codes");
        }
    }
    AirportBST replacement;
    replacement.root_ = buildMiddle(sorted, 0, n);
    replacement.size_ = n;
    swap(replacement);
}

void AirportBST::rebalance() { buildBalanced(inorder()); }

std::string AirportBST::candidate(const std::string& kind) const {
    std::vector<const Node*> pending;
    if (root_ != nullptr) pending.push_back(root_);
    while (!pending.empty()) {
        const Node* node = pending.back();
        pending.pop_back();
        bool left = node->left != nullptr;
        bool right = node->right != nullptr;
        if ((kind == "leaf" && !left && !right) ||
            (kind == "left" && left && !right) ||
            (kind == "right" && !left && right) ||
            (kind == "two" && left && right)) return node->airport.code;
        if (right) pending.push_back(node->right);
        if (left) pending.push_back(node->left);
    }
    return "";
}

std::string AirportBST::deepestCode() const {
    std::queue<const Node*> pending;
    if (root_ != nullptr) pending.push(root_);
    std::string result;
    while (!pending.empty()) {
        const Node* node = pending.front();
        pending.pop();
        result = node->airport.code;
        if (node->left != nullptr) pending.push(node->left);
        if (node->right != nullptr) pending.push(node->right);
    }
    return result;
}
