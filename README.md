# Mini Project 02: Airport Code Lookup Service with a Binary Search Tree

**Student:** Xiaoxing Tong  
**Language:** C++17  
**Project:** Mini Project 02, Fall 2026  
**Course in the supplied assignment PDF:** EECE 7205

This service looks up airports by their IATA-style code and returns their name,
city, country, ICAO code and coordinates. It uses a manually implemented binary
search tree, with `Airport` as the record class and `AirportBST` as the tree class.
The root and node type are private. There is no standard-library tree or map in
the application. The test suite uses `std::map` only as an independent oracle.

The service supports insertion, duplicate rejection, search paths, all deletion
cases, traversals, preorder save/load, strict BST validation, inclusive range
queries, rebalancing, and a four-way shape experiment.

## Build and run

From this repository's root, using a C++17 compiler:

```sh
g++ -std=c++17 -O2 src/*.cpp -o airports
./airports
```

On Windows, a compiled 64-bit executable is included in the downloadable project
package. Open PowerShell in this folder and run:

```powershell
.\bin\airports.exe
```

To rebuild on Windows with g++ on PATH:

```powershell
.\scripts\build.ps1
.\scripts\test.ps1
```

Both scripts also accept `-Compiler "C:\path\to\g++.exe"`. On Linux/macOS, run
`sh scripts/build.sh` to build both binaries and run the tests. Alternatively,
use `cmake -S . -B build`, `cmake --build build`, then
`ctest --test-dir build --output-on-failure`.

The application itself uses only the C++ standard library. Python is optional
for the independent data check; Pillow is optional for regenerating the GIF.
Data paths are relative to the working directory, so run from this folder.
Use `chcp 65001` in a Windows terminal if non-ASCII airport names display poorly.

## Commands

| Command | Result |
|---|---|
| `load data/airports.dat` | Load records into a new tree in file order |
| `load data/airports.csv` | Also accept the supplied headered CSV |
| `find BOS` | Airport, complete comparison path, comparison count |
| `add CODE "NAME" "CITY" "COUNTRY" ICAO LAT LON` | Insert a record; reject duplicates |
| `remove CODE` | Delete and display the top three levels before/after |
| `list` | Print every full airport record in inorder; check sortedness |
| `min` / `max` | Print the minimum / maximum airport |
| `save docs/tree.preorder` | Save preorder codes plus record sidecar |
| `load docs/tree.preorder` | Rebuild by insertion in preorder |
| `validate` | Check strict ancestor bounds `(lo, hi)` |
| `range BOS BWI` | Inclusive range with subtree pruning and visited count |
| `levels` / `stats` / `cases` | Tree levels, measurements, real case examples |
| `rebalance` | Direct balanced build from the sorted inorder array |
| `experiment docs/experiment.csv` | Compare four builds using the original loaded catalog |
| `help` / `quit` | Command reference / exit |

Quote names and paths containing spaces. Lowercase query codes are converted to
uppercase. `000` and `ZZZ` are useful queries below and above all supplied keys.
The fictional QQQ airport is **Xiaoxing Tong Airport, Shanghai, China**, at
31.2304, 121.4737, with fictional ICAO code ZQQQ.

## Data checks (Task 1)

The exact supplied course files are included unchanged. `airports.dat` is the
primary dataset for the experiment; the headered CSV contains the same retained
code order. The loader handles quoted commas and escaped double quotes, and
skips `\N` or empty IATA values. Malformed rows produce a line-numbered error.
An unsuccessful load leaves the current tree intact.

| Check | Observed |
|---|---:|
| Source rows, excluding optional CSV header | 7,698 |
| Missing IATA rows skipped | 1,626 |
| Inserted unique codes | 6,072 |
| Duplicates | 0 |
| Countries | 235 |
| United States airports | 1,251 |
| Minimum / maximum | AAA / ZZV |

**Data detail:** Dunnville Airport has the code `DU9`. The posted file therefore
has 6,072 nonmissing three-character codes, but only 6,071 are entirely letters.
Accepting uppercase letters and digits preserves the assignment's expected count.
See [independent data evidence](docs/data-check.json). Regenerate it with
`python scripts/inspect_data.py`, which uses Python's standard `csv` parser and
an independent array-based BST implementation to cross-check the C++ results.

## Sample session

```text
> load data/airports.dat
Loaded 6072 airports (1626 lines skipped: no IATA code). Duplicates: 0
Airports: 6072  Height: 177  Average depth: 18.3233
Countries: 235  United States: 1251
> find BOS
Path: GKA -> AEY -> EGS -> BUJ -> BJA -> BSK -> BOY -> BNI -> BOH -> BOO -> BOU -> BOS (comparisons: 12)
BOS  General Edward Lawrence Logan International Airport, Boston, United States | ICAO: KBOS | 42.364300, -71.005203
> range BOS BWI
[113 complete records are printed; see docs/demo-session.txt]
113 airports, 127 nodes visited
> remove AEY
Before deleting AEY:
Level 0: GKA
Level 1: AEY MAG
Level 2: AAE EGS HGU POM
Deleted AEY (case D3: two children; replacement AFA)
After:
Level 0: GKA
Level 1: AFA MAG
Level 2: AAE EGS HGU POM
validate: OK
```

The bracketed range-list line above is an editorial abbreviation. The full
[captured session](docs/demo-session.txt) and [GIF](docs/demo.gif) contain the
actual output. The GIF is a terminal replay rendered from the executable's
captured stdout. It demonstrates find, range, two-child deletion, validation,
QQQ insertion/search/deletion, save/load, and rebalance.

Run it again:

```powershell
.\bin\airports.exe --script docs/demo.commands
```

On Linux/macOS substitute `./airports`. To regenerate the capture and GIF, build
`bin/airports` or `bin/airports.exe`, install Pillow, then run
`python scripts/create_demo.py`. This is an optional documentation step.

## Correctness and tests (Tasks 2-5)

The [test log](docs/test-results.txt) records actual execution. It covers:

- I1 empty-root insertion, I2 left, I3 right, I4 duplicates, preserving the old record.
- Search at root GKA, leaf AAC, deepest node YZR, absent XYZ, below-min 000,
  above-max ZZZ, and min/max. YZR needs 178 comparisons at depth 177; BOS needs 12.
- Leaf deletion AAC, left-only AAQ, right-only AAA, two-child AEY and root GKA.
  Each real-data case starts from the original tree and prints the top three
  levels before/after. All remaining full records are checked for preservation.
- A two-child successor with its own right child, plus root deletion with zero
  or one child. The entire successor record is copied, not just its key.
- Inorder sortedness, exact preorder round-trip, and full metadata recovery in
  a fresh tree, including the personalized QQQ airport.
- A deliberately invalid ancestor bound that immediate-child checks would miss.
- Inclusive range endpoints, absent and reversed ranges, and full-tree queries.
- A 6,072-node sorted chain, iterative search/traversals/validation/cleanup,
  malformed input and failed-load rollback.
- 3,000 mixed randomized operations compared against `std::map` in tests.

Every explicit insertion/deletion in the mutation tests is followed by the
range-method validator. Current run: **181,370 checks; 45,542 explicit
post-mutation validations; all tests passed**. See
[pseudocode and complexity](docs/pseudocode.md) for all six tasks.

For preorder persistence, retain both `FILE` (one code per line) and
`FILE.records.csv` (complete records). A code-only file can also be loaded when
all its records exist in the active tree or previously loaded data catalog.
Shape verification uses height, full level order, and a stronger preorder
signature containing null markers. Unknown codes or repeated saved keys are
rejected before replacing the current tree.

## Shape experiment (Task 6)

Measured on 2026-11-01 with GCC 16.2.0, C++17, `-O2`, Windows 11 x64, AMD Ryzen 5
7500X3D. Height is the number of edges, root depth is zero, and average depth
includes every node, including the root.

| Build order | Height | Average depth | Time for 10,000 searches (ms) | Explanation |
|---|---:|---:|---:|---|
| File order | 177 | 18.323287 | 1.033000 | Geographic groups contain long runs of increasing codes |
| Sorted order | 6,071 | 3,035.500000 | 99.401000 | Every inserted code becomes the next right child |
| Balanced (middle rule) | 12 | 10.653162 | 0.778000 | Middle keys split remaining records nearly equally |
| Random shuffle (bonus) | 28 | 14.109519 | 0.876000 | Shuffling breaks the long ordered runs |

[Raw measured CSV](docs/experiment.csv) also records mean comparisons, seed and
repeat count. These are measured times, not predicted or copied sample values;
timings vary with hardware and background load.

**Method.** Seed 2026 generates 10,000 uniformly selected existing codes with
replacement. All four trees receive the identical query vector. Each tree gets
1,000 warm-up searches, then seven batches of 10,000 searches; the table reports
the median batch time. Construction, validation, random generation, path
formatting and output are outside the timed region. A volatile checksum makes
lookup results observable, preventing removal of the work by optimization.
This measures successful lookups; missing-key behavior is tested separately.
The shuffle has its own seed-2026 generator.

**Why file order is worse than a shuffle.** The file is geographically grouped,
rather than a random ordering of codes. In the retained airport vector, positions
21-201 are 181 Canadian airports, from YAM to ZFM. That block has just one
adjacent descending pair, YZS followed by YDA; most keys otherwise arrive in
increasing order. Repeated insertion creates a long rightward chain. The deepest
file-order node is YZR, at depth 177. Later insertions cannot shorten the chain
because an ordinary BST performs no rotations. Shuffling separates those keys,
so they no longer arrive as one nearly ordered block. The independent data check
records this evidence in `docs/data-check.json`.

**Connection to O(h).** Search, insert and delete follow at most a root-to-leaf
path, so their worst-case time is O(h). Sorted insertion gives h = n - 1 = 6,071
and average depth (n - 1)/2 = 3,035.5. Middle-rule construction gives
h = floor(log2 6072) = 12. File order's tall Canadian branch affects worst-case
search more strongly than mean depth: average depth 18.32 is only moderately
above the shuffled tree's 14.11, despite heights 177 and 28. A random shuffle
improves expected shape but does not guarantee balance; the middle-rule build
does. The direct balanced build is O(n) once records are sorted.

Re-run from the original dataset:

```powershell
.\bin\airports.exe --experiment data/airports.dat docs/experiment.csv
```

## Repository contents and milestone status

```text
src/                  Airport, AirportBST, CSV/persistence, CLI, experiment
data/                 Exact supplied airports.dat and airports.csv
docs/pseudocode.md    Pseudocode organized before Tasks 1-6; operation costs
docs/demo.gif         34.7-second replay of actual session output
docs/demo.commands    Reproducible demo commands
docs/demo-session.txt Full, unabridged captured output
docs/demo.preorder*   Saved demo tree and matching record sidecar
docs/search-cases*    Reproducible search cases and full paths
docs/test-results.txt Actual C++ test output, including all deletion cases
docs/experiment.csv   Raw measured Task 6 results
docs/data-check.json  Independent data/shape facts and source checksums
docs/verification.json Current Mini Project 02 inputs, identity and verification summary
docs/requirements_checklist.md Task-by-task implementation and evidence mapping
docs/experiment-session.txt Captured output of the fresh timing experiment
docs/guide_zh.md      Chinese running guide and explanation
docs/publishing.md    Steps for the required public GitHub repository
tests/                C++ correctness and randomized differential tests
scripts/              Build, test, independent data check, GIF generation
bin/                  Windows executables in the downloadable package only
```

M1, M2 and M3 are verified locally. The implementation, README, experiment and
demo portions of M4 are complete. The required public repository name is
`airport-bst-cpp`. See [publishing steps](docs/publishing.md) or the
[Chinese web-upload guide](docs/publishing_zh.md).

## Data credit and license

Airport data: **[OpenFlights.org](https://openflights.org/data.php)**, using the
course-provided snapshot. The database is available under the
[Open Database License (ODbL) 1.0](https://opendatacommons.org/licenses/odbl/1-0/)
and individual contents under the
[Database Contents License (DbCL) 1.0](https://opendatacommons.org/licenses/dbcl/1-0/),
as stated on the [OpenFlights licensing page](https://openflights.org/data.php#license).
See [DATA_LICENSE.md](DATA_LICENSE.md). No live dataset download is performed,
so the checked-in course snapshot remains reproducible.
