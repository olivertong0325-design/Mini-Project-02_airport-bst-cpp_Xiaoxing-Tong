# Mini Project 02: requirements and evidence

Student: **Xiaoxing Tong**  
Verification date: **2026-11-01**

This checklist maps the supplied four-page assignment to the implementation and
captured evidence. Source code is under `src/`; all tree algorithms use the
project's own nodes rather than an STL associative container.

| Requirement | Implementation | Evidence |
|---|---|---|
| C++17; Airport and AirportBST classes; private root | `src/Airport.h`, `src/AirportBST.h` | Clean g++ build with `-std=c++17 -O2 -Wall -Wextra -Wpedantic` |
| Counted loops use the requested indexed form | `src/*.cpp` | `for (int i = 0; i < n; i++)` and equivalent counted bounds |
| Pseudocode before each of Tasks 1-6 | `docs/pseudocode.md` | Build, CSV parse, insertion, search, deletion, traversals, validation, range and balanced build |
| Task 1: quote-aware load and vector of records | `src/DataIO.cpp` | 7,698 rows; 1,626 missing codes; 6,072 unique records |
| Task 1: file-order insertion and duplicates | `AirportBST::insert`, `loadTree` | Zero duplicates in course data; explicit I4 duplicate fixture in tests |
| Task 1: height and average depth | `AirportBST::stats` | Height 177; depth sum 111,259; average 18.323287 |
| Task 2: full search path and comparisons | `AirportBST::search`, CLI `find` | `docs/search-cases-session.txt` |
| Task 2: every search case, min/max | `tests/test_airport_bst.cpp` | Root GKA; leaf AAC; deepest YZR; absent XYZ; below-min 000; above-max ZZZ; min AAA; max ZZV |
| Task 3: leaf, left-only, right-only, two-child and root deletion | `AirportBST::remove` | AAC, AAQ, AAA, AEY and GKA respectively; `docs/test-results.txt` |
| Task 3: top three levels before/after each deletion | `AirportBST::printLevels` | All five real-data deletion scenarios in the test log |
| Task 3: fictional personalized QQQ | `docs/demo.commands` | Xiaoxing Tong Airport, Shanghai, China; add/find/remove captured in demo |
| Task 4: full alphabetical listing | `AirportBST::inorder`, CLI `list` | Sortedness checks in tests; full CLI list verified |
| Task 4: preorder save/load with original shape | `src/DataIO.cpp` | Same height, complete level order, and null-marked shape signature checked in tests |
| Task 4: range-method validation after mutations in tests | `AirportBST::validate` | 45,542 explicit post-mutation validations |
| Task 5: inclusive pruned range | `AirportBST::range` | BOS-BWI: 113 records and 127 visited non-null nodes |
| Task 6: file-order, sorted and balanced builds | `src/Experiment.cpp` | Heights 177, 6,071 and 12; measured table in README and CSV |
| Task 6 bonus: random shuffle | `src/Experiment.cpp` | Height 28 for this seed/toolchain; same 6,072 codes |
| Task 6: 10,000 random searches for every build | `runExperiment` | Same pre-generated seed-2026 queries; median of seven timed batches |
| Task 6: explanation of file ordering and O(h) | README; Chinese guide | Early 181-record Canadian block; one adjacent code inversion; long branch ending at YZR |
| Iterative insert/search to avoid stack overflow | `src/AirportBST.cpp` | Actual 6,072-node chain is built, searched, traversed, validated, queried and destroyed in tests |
| Short GIF with find, range, D3 delete and validate | `docs/demo.gif` | Actual captured stdout replay; reproducible with `scripts/create_demo.py` |
| Data source credited | README; `DATA_LICENSE.md` | OpenFlights.org, ODbL and DbCL links |
| Public repository named airport-bst-cpp | User publishes the delivered project | `docs/publishing_zh.md` gives the web-upload steps |

## Reproduce the local evidence

Run from the project root, with g++ on PATH:

```powershell
.\scripts\build.ps1
.\scripts\test.ps1
.\bin\airports.exe --script docs/search-cases.commands
.\bin\airports.exe --script docs/demo.commands
.\bin\airports.exe --experiment data/airports.dat docs/experiment.csv
```

The included Windows x64 executable can be run immediately without rebuilding.
For a non-PATH compiler, the PowerShell build/test scripts accept
`-Compiler "C:\path\to\g++.exe"`. Linux/macOS users can use the g++ commands in
README or `sh scripts/build.sh`.

## Data and persistence details

- Retain `DU9`, the alphanumeric code for Dunnville Airport. Rejecting it would
  incorrectly reduce the dataset to 6,071 records.
- `save FILE` creates the required code-only preorder file and a companion
  `FILE.records.csv`, which preserves full metadata even in a fresh process.
- The demo deletes one original airport before its save/load step, so that saved
  demonstration tree has **6,071** nodes. This is expected; all shape-experiment
  builds start from the complete original **6,072** records.
- Failed file loads are rejected before replacing the active tree.
- The supplied PDF's course header says EECE 2560; the user's folder says
  EECE7205. Algorithmic requirements follow the supplied PDF.

The local code, documentation and evidence are complete. Publication is performed
by the student, as requested; this package does not claim a published repository.
