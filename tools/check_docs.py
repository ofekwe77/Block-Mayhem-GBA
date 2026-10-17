"""Documentation consistency (PART 59.3, 63.2 Documentation): every host test cited by name in the design and
verification documents must exist in tests/*.cpp, so the documents cannot claim evidence that is not there.

A citation is a `suite.name` token whose suite is a real test suite (e.g. bomb.armor_absorbs_and_ice_absorbs,
boss4.electric_emperor_full_verification). Wildcards such as `abilities.*` or `chain.disabled*` are not checked.

Usage: python3 tools/check_docs.py        (exit 0 = every citation resolves)
"""
import glob
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DOCS = ["INTERACTIONS.md", "WORLD_PRODUCTION_MATRIX.md", "BOSS_PRODUCTION_MATRIX.md", "TESTING.md", "BALANCE.md",
        "STAGE_PASSPORTS.md", "RELEASE_VERIFICATION.md", "TASKS.md"]


def main():
    tests = set()
    for path in glob.glob(os.path.join(ROOT, "tests", "*.cpp")):
        for suite, name in re.findall(r"BM_TEST\((\w+),\s*(\w+)\)", open(path, encoding="utf-8").read()):
            tests.add((suite, name))
    suites = {s for s, _ in tests}
    missing = []
    cited = 0
    for doc in DOCS:
        path = os.path.join(ROOT, doc)
        if not os.path.exists(path):
            missing.append("%s: document missing" % doc)
            continue
        text = open(path, encoding="utf-8").read()
        for suite, name in re.findall(r"\b([a-z][a-z0-9]*)\.([a-z][a-z0-9_]*[a-z0-9])(?![\w*])", text):
            if suite not in suites or name in ("cpp", "h", "md", "txt", "png"):
                continue
            cited += 1
            if (suite, name) not in tests:
                missing.append("%s cites %s.%s, which is not a test" % (doc, suite, name))
    for m in sorted(set(missing)):
        print("  " + m)
    print("docs check: %d test citations in %d documents, %s" % (cited, len(DOCS),
                                                                "PASS" if not missing else "%d broken" % len(set(missing))))
    return 0 if not missing else 1


if __name__ == "__main__":
    sys.exit(main())
