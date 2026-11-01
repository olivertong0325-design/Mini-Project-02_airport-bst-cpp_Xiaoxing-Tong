"""Independent standard-library CSV/data/BST check (not the C++ implementation)."""
import csv
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
dat_path = ROOT / "data/airports.dat"
csv_path = ROOT / "data/airports.csv"
with dat_path.open(encoding="utf-8", newline="") as stream:
    all_rows = list(csv.reader(stream))
rows = [row for row in all_rows if row[4] != r"\N"]
with csv_path.open(encoding="utf-8", newline="") as stream:
    csv_rows = list(csv.DictReader(stream))
keys = [row[4] for row in rows]
left = [-1] * len(rows)
right = [-1] * len(rows)
parent = [-1] * len(rows)
for i in range(1, len(keys)):
    node = 0
    while True:
        children = left if keys[i] < keys[node] else right
        if children[node] < 0:
            children[node] = i
            parent[i] = node
            break
        node = children[node]
pending = [(0, 0)]
depth_sum = 0
deepest = (0, 0)
while pending:
    node, depth = pending.pop()
    depth_sum += depth
    if depth > deepest[1]:
        deepest = (node, depth)
    if right[node] >= 0:
        pending.append((right[node], depth + 1))
    if left[node] >= 0:
        pending.append((left[node], depth + 1))
canada_block = rows[20:201]
report = {
    "total_rows": len(all_rows),
    "missing_iata": len(all_rows) - len(rows),
    "loaded": len(rows),
    "unique_codes": len(set(keys)),
    "countries": len({row[3] for row in rows}),
    "united_states": sum(row[3] == "United States" for row in rows),
    "alphanumeric_exception": [row[4] for row in rows if not row[4].isalpha()],
    "minimum": min(keys),
    "maximum": max(keys),
    "range_BOS_BWI": sum("BOS" <= code <= "BWI" for code in keys),
    "file_order_height": deepest[1],
    "file_order_depth_sum": depth_sum,
    "file_order_average_depth": depth_sum / len(rows),
    "deepest_code": keys[deepest[0]],
    "csv_same_nonmissing_key_order": keys == [r["iata"] for r in csv_rows if r["iata"]],
    "early_canadian_block": {
        "retained_record_positions_1_based": [21, 201],
        "size": len(canada_block),
        "all_canada": all(row[3] == "Canada" for row in canada_block),
        "adjacent_inversions": [
            [canada_block[i - 1][4], canada_block[i][4]]
            for i in range(1, len(canada_block))
            if canada_block[i - 1][4] >= canada_block[i][4]
        ],
        "first_code": canada_block[0][4],
        "last_code": canada_block[-1][4],
    },
    "sha256": {
        path.name: hashlib.sha256(path.read_bytes()).hexdigest()
        for path in [dat_path, csv_path]
    },
}
# The supplied headered CSV uses empty cells for missing IATA values.
assert report["csv_same_nonmissing_key_order"]
assert (report["loaded"], report["file_order_height"], report["range_BOS_BWI"]) == (6072, 177, 113)
text = json.dumps(report, indent=2, ensure_ascii=False) + "\n"
(ROOT / "docs/data-check.json").write_text(text, encoding="utf-8")
print(text)
