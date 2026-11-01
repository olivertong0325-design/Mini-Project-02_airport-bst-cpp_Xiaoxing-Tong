# Pseudocode written before each task

Keys are unique, case-sensitive uppercase three-character strings; DU9 from the
course file is retained. Height counts edges: empty tree = -1, root depth = 0.
Finite counted loops use `for (int i = 0; i < n; i++)` in the C++ implementation.
Insertion, search, deletion, validation and traversals use iteration so that a
6,072-node chain does not consume 6,072 recursive call frames.

## Task 1: read the data, build by insertion, and measure

### Parse a CSV row

```text
fields = empty array; field = empty string; insideQuotes = false
FOR each character position i, from 0 to row length - 1:
    IF insideQuotes:
        IF current character is a quote:
            IF next character is also a quote:
                append one quote to field; advance i past the second quote
            ELSE leave quoted mode
        ELSE append character to field
    ELSE IF character is a comma:
        append field to fields; reset field
    ELSE IF character opens a quoted field:
        enter quoted mode
    ELSE append character to field
Reject unmatched quotes and invalid text after closing quotes
Append the last field and return fields
```

### Build

```text
records = empty vector of Airport
FOR each line in the supplied airports.dat:
    fields = PARSE_CSV(line)
    IF IATA field is missing (\N): increment skipped; continue
    Construct Airport(name, city, country, IATA, ICAO, latitude, longitude)
    append Airport to records
tree = empty BST
FOR i = 0 TO records.size - 1:
    IF INSERT(tree, records[i]) succeeds: increment inserted
    ELSE report duplicate code (I4); keep the original record
Report inserted, skipped, duplicates, height and average depth
```

### Insert: I1, I2, I3 and I4

```text
link = address of root
WHILE link points to a node:
    Compare new airport.code with node.code
    IF equal: return false (I4, duplicate rejected)
    IF smaller: link = address of node.left (I2)
    ELSE: link = address of node.right (I3)
Create a node at link (also handles an empty root, I1)
Increment size; return true
```

### Height and average depth

```text
pending = [(root, 0)] if root exists, otherwise []
height = -1; depthSum = 0; count = 0
WHILE pending is not empty:
    (node, depth) = pop pending
    height = max(height, depth); depthSum += depth; count++
    push each non-null child with depth + 1
averageDepth = depthSum / count, or 0 for an empty tree
```

## Task 2: search and extremes

```text
SEARCH(code):
    node = root; path = []; comparisons = 0
    WHILE node is not null:
        append node.code to path; comparisons++
        comparison = compare(code, node.code)
        IF comparison == 0: return node.airport, path, comparisons
        IF comparison < 0: node = node.left
        ELSE: node = node.right
    return not found, path, comparisons (null is not a key comparison)

MINIMUM: start at root; follow left pointers until no left child
MAXIMUM: start at root; follow right pointers until no right child

Test root, a leaf, a deepest node, XYZ, 000 (below min), and ZZZ (above max)
```

One comparison means one logical three-way comparison with a visited key, rather
than the number of machine instructions or individual characters compared.

## Task 3: delete and add a fictional airport

```text
REMOVE(code):
    link = address of root
    WHILE link points to a node whose code differs:
        follow address of left or right child according to comparison
    IF link is null: return not found
    target = node at link
    IF target has two children (D3):
        successorLink = address of target.right
        WHILE successorLink's node has a left child:
            successorLink = address of that left child
        successor = node at successorLink
        copy successor's COMPLETE Airport record into target
        set successorLink to successor.right
        delete successor
    ELSE:
        child = target.left if present, otherwise target.right
        child is null -> D1 (leaf)
        child is non-null -> D2 (left-only or right-only)
        set link to child; delete target
    decrement size; return case and replacement code

FOR each real-data case (leaf, left-only, right-only, two children, root):
    rebuild the original tree; print top three levels
    remove selected code; print top three levels; VALIDATE
INSERT QQQ = Xiaoxing Tong Airport, Shanghai, China
SEARCH QQQ; REMOVE QQQ; VALIDATE after each mutation
```

Using the address of the incoming pointer handles root and non-root deletion by
the same algorithm. D3 must reconnect a successor's possible right child.

## Task 4: traversal, persistence, validation

### Inorder (list)

```text
node = root; stack = []
WHILE node exists OR stack not empty:
    WHILE node exists: push node; node = node.left
    node = pop stack; print/store node.airport
    node = node.right
Check every adjacent output pair: previous.code < current.code
```

### Preorder (save) and load

```text
stack = [root] if root exists, otherwise []
WHILE stack not empty:
    node = pop stack; write node.code on its own line
    push node.right if present
    push node.left if present (left will be popped first)
Write complete Airport records to FILE.records.csv as a companion file

LOAD saved FILE:
    read matching records from sidecar (or current records/original catalog)
    replacement = empty tree
    FOR each code in FILE, in saved preorder:
        INSERT its Airport record into replacement
    reject unknown or repeated codes; VALIDATE replacement
    compare height, full level order, and preorder INCLUDING null markers
    replace active tree only after all loading succeeds
```

Why reconstruction works: the first preorder key is the root, all left-subtree
keys precede all right-subtree keys, and the same property holds recursively.
Repeated BST insertion in that order therefore restores all child positions.
The code file satisfies the requested code-only format. Its sidecar allows a
new process to recover names, coordinates, and fictional airports too.

### Validate by the strict (lo, hi) range method

```text
stack = [(root, no lower bound, no upper bound)] if root exists
count = 0
WHILE stack not empty:
    (node, lo, hi) = pop stack
    IF lo exists AND node.code <= lo: return false
    IF hi exists AND node.code >= hi: return false
    count++
    push (node.right, node.code, hi) if right exists
    push (node.left, lo, node.code) if left exists
return count == stored size
```

The bounds come from all ancestors. Checking only immediate children would miss
a node that belongs on the other side of an earlier ancestor.

### Level order

```text
queue = [root]
FOR depth = 0 TO requestedLevels - 1 while queue is not empty:
    width = queue.size
    FOR i = 0 TO width - 1:
        node = dequeue; print node.code (or '-' for a missing child)
        enqueue left and right positions for each non-null node
```

## Task 5: inclusive range with pruning

```text
node = root; stack = []; answer = []; visited = 0
IF FROM > TO: return empty answer, 0
WHILE node exists OR stack not empty:
    WHILE node exists:
        visited++  (each examined non-null node is counted once)
        IF node.code < FROM: node = node.right (skip its entire left subtree)
        ELSE IF node.code > TO: node = node.left (skip its entire right subtree)
        ELSE:
            push node
            node = node.left if node.code > FROM, otherwise null
    IF stack not empty:
        node = pop stack; append node.airport to answer
        node = node.right if node.code < TO, otherwise null
return answer in sorted order, visited
```

## Task 6: balanced build and shape experiment

### Balanced middle rule (rebalance)

```text
sorted = INORDER(tree)
BUILD_MIDDLE(sorted, lo, hi):  // half-open interval [lo, hi)
    IF lo >= hi: return null
    mid = lo + floor((hi - lo) / 2)
    node = new Node(sorted[mid])
    node.left = BUILD_MIDDLE(sorted, lo, mid)
    node.right = BUILD_MIDDLE(sorted, mid + 1, hi)
    return node
Replace root with BUILD_MIDDLE(sorted, 0, sorted.size)
```

This is the only recursive tree operation: its call depth is O(log n), even when
the input tree was a chain. The inorder extraction of that input is iterative.

### Experiment

```text
Generate 10,000 random existing codes with replacement, using seed 2026
Use the EXACT same query vector for every build
FOR each build: file order, sorted insertion, middle-rule balanced, shuffled:
    build a fresh tree from the same unique airports; VALIDATE
    measure edge height and mean node depth
    warm up with 1,000 searches
    FOR repetition = 0 TO 6:
        start steady_clock
        FOR i = 0 TO 9999: perform iterative lookup and accumulate checksum
        stop clock; store elapsed milliseconds
    report median of the seven 10,000-search batches
    separately count mean key comparisons for those same queries
Explain country/code grouping and the early almost-sorted Canadian block
```

## Operation costs

| Operation | Time | Extra space, excluding output |
|---|---|---|
| Insert / search / delete / min / max | O(h) | O(1), or O(h) to retain search path |
| Height / validate / inorder / preorder | O(n) | O(h) |
| Level order | O(n), or nodes in requested levels | O(maximum width) |
| Range reporting k matches | O(h + k), worst O(n) | O(h) |
| Balanced build from sorted array | O(n) | O(log n) recursive frames |
| Rebalance, including inorder array | O(n) | O(n) array |
| Preorder reload by repeated insertion | O(nh), worst O(n^2) | O(n) records |
| Iterative tree destruction | O(n) | O(1) |
