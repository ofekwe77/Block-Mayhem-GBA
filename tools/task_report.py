"""Counts task statuses in TASKS.md (evidence-backed progress numbers, PART 59.5).

Usage: python3 tools/task_report.py            -> prints a markdown summary
       python3 tools/task_report.py --update   -> rewrites the generated block in PROGRESS_REPORT.md
"""
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
STATUSES = ["NOT STARTED", "IN PROGRESS", "IMPLEMENTED", "TESTED", "POLISHED", "BLOCKED"]


def parse():
    sections = []
    current = None
    with open(os.path.join(ROOT, "TASKS.md"), encoding="utf-8") as f:
        for line in f:
            if line.startswith("## "):
                current = {"name": line[3:].strip(), "counts": {s: 0 for s in STATUSES}, "total": 0}
                sections.append(current)
                continue
            if not line.startswith("|") or current is None:
                continue
            cols = [c.strip() for c in line.strip().strip("|").split("|")]
            if len(cols) < 3 or cols[1] not in STATUSES:
                continue
            current["counts"][cols[1]] += 1
            current["total"] += 1
    return sections


def table(sections):
    out = ["| Area | Total | Not started | In progress | Implemented | Tested | Polished | Blocked |",
           "|------|------:|------:|------:|------:|------:|------:|------:|"]
    totals = {s: 0 for s in STATUSES}
    grand = 0
    for s in sections:
        c = s["counts"]
        out.append("| %s | %d | %d | %d | %d | %d | %d | %d |" % (
            s["name"], s["total"], c["NOT STARTED"], c["IN PROGRESS"], c["IMPLEMENTED"], c["TESTED"],
            c["POLISHED"], c["BLOCKED"]))
        for k in STATUSES:
            totals[k] += c[k]
        grand += s["total"]
    out.append("| **All** | **%d** | %d | %d | %d | %d | %d | %d |" % (
        grand, totals["NOT STARTED"], totals["IN PROGRESS"], totals["IMPLEMENTED"], totals["TESTED"],
        totals["POLISHED"], totals["BLOCKED"]))
    return "\n".join(out)


def main():
    t = table(parse())
    if "--update" in sys.argv:
        path = os.path.join(ROOT, "PROGRESS_REPORT.md")
        text = open(path, encoding="utf-8").read()
        text = re.sub(r"<!-- TASKS:BEGIN -->.*<!-- TASKS:END -->",
                      "<!-- TASKS:BEGIN -->\n" + t + "\n<!-- TASKS:END -->", text, flags=re.S)
        with open(path, "w", encoding="utf-8", newline="\n") as f:
            f.write(text)
    print(t)


if __name__ == "__main__":
    main()
