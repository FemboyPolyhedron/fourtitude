#!/usr/bin/env python3
import re
import sys
from pathlib import Path

errors = []

KEYWORDS = r"(if|for|while|switch)"

def strip_strings_and_comments(line):
    line = re.sub(r'//.*', '', line)
    line = re.sub(r'"(\\.|[^"])*"', '""', line)
    line = re.sub(r"'(\\.|[^'])*'", "''", line)
    return line

def check_file(path):
    try:
        raw = Path(path).read_text(encoding="utf-8", errors="ignore")
    except:
        return

    is_header = Path(path).suffix in (".h", ".hpp")

    lines = raw.splitlines()

    for i, orig in enumerate(lines, 1):
        line = strip_strings_and_comments(orig)
        stripped = line.strip()

        if not stripped:
            continue
 
        if not is_header:
            if re.search(r'\b[a-zA-Z_][a-zA-Z0-9_]*\s{2,}[a-zA-Z_]', line):
                errors.append(f"{path}:{i} excessive spacing in declaration")
 
        if re.search(rf'\b{KEYWORDS}\(', line):
            errors.append(f"{path}:{i} missing space after keyword")
 
        if re.search(rf'\b{KEYWORDS}\s*\(.*\)\s*{{\s*$', line):
            errors.append(f"{path}:{i} K&R braces not allowed. must be allman")
 
        if re.match(rf'\s*{KEYWORDS}\s*\(.*\)\s*$', line):
            if i < len(lines):
                nxt = lines[i].strip()
                if not nxt.startswith("{") and not nxt.endswith(";"):
                    errors.append(f"{path}:{i} multiline statement must be braced or not be multiline")

        if re.search(r'\S\s{2,}=\s{2,}\S', line):
            errors.append(f"{path}:{i} excessive spacing around '='")

def main():
    paths = sys.argv[1:] or ["."]
    for p in paths:
        for file in Path(p).rglob("*"):
            if "bazel-" in str(file):
                continue
            if file.suffix in (".c", ".h", ".cpp", ".hpp"):
                check_file(file)

    if errors:
        print("\nstyling violations found...\n")
        for e in errors:
            print(e)
        sys.exit(1)
    else:
        print("stylecheck passed")

if __name__ == "__main__":
    main()