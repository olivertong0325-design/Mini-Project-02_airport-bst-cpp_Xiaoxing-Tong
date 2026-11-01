#ifndef AIRPORT_BST_H
#define AIRPORT_BST_H

#include "Airport.h"

#include <cstdint>
#include <iosfwd>
#include <string>
#include <vector>

struct TreeStats {
    int size = 0;
    int height = -1;                 // Number of edges; root has depth zero.
    std::uint64_t depthSum = 0;
    double averageDepth = 0.0;
};

struct SearchResult {
    bool found = false;
    Airport airport;
    std::vector<std::string> path;
};

struct DeleteResult {
    bool removed = false;
    std::string caseName;
    std::string replacement;
};

struct RangeResult {
    std::vector<Airport> airports;
    int visited = 0;
};

class AirportBST {
private:
    struct Node {
        Airport airport;
        Node* left = nullptr;
        Node* right = nullptr;
        explicit Node(const Airport& value) : airport(value) {}
    };

    Node* root_ = nullptr;
    int size_ = 0;
    static Node* buildMiddle(const std::vector<Airport>& sorted, int lo, int hi);

public:
    AirportBST() = default;
    ~AirportBST();
    AirportBST(const AirportBST&) = delete;
    AirportBST& operator=(const AirportBST&) = delete;

    void clear();
    void swap(AirportBST& other) noexcept;
    int size() const { return size_; }
    std::string rootCode() const;
    bool insert(const Airport& airport);
    const Airport* lookup(const std::string& code, int* comparisons = nullptr) const;
    SearchResult search(const std::string& code) const;
    DeleteResult remove(const std::string& code);
    const Airport* minimum() const;
    const Airport* maximum() const;
    TreeStats stats() const;
    bool validate() const;
    std::vector<Airport> inorder() const;
    std::vector<Airport> preorder() const;
    std::vector<std::string> levelOrder() const;
    std::vector<std::string> shapeSignature() const;
    void printLevels(std::ostream& out, int count = 3) const;
    RangeResult range(const std::string& from, const std::string& to) const;
    void buildBalanced(const std::vector<Airport>& sorted);
    void rebalance();
    std::string candidate(const std::string& kind) const;
    std::string deepestCode() const;
};

#endif
