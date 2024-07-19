import os

def add_buildgn():
    build_gn = os.path.join("v8-project","v8","build","config","sanitizers","BUILD.gn")
    coverage_flags_found = False
    added = False

    lines = []

    with open(build_gn) as f:
        for line in f.readlines():
            if "config(\"coverage_flags\")" in line:
                coverage_flags_found = True
            
            if coverage_flags_found:
                if "-fsanitize-coverage=$sanitizer_coverage_flags" in line and not added:
                    lines.append('        "-fsanitize-coverage-allowlist=$sanitizer_coverage_allowlist_flags",')
                    added = True

            lines.append(line)

    with open(build_gn, "w") as f:
        for line in lines:
            print(line, file=f, end="")

def add_sanitizer():
    san_gni = os.path.join("v8-project","v8","build","config","sanitizers","sanitizers.gni")
    declare_args_found = False
    added = False

    lines = []

    with open(san_gni) as f:
        for line in f.readlines():
            if "declare_args()" in line:
                declare_args_found = True

            if declare_args_found:
                if "sanitizer_coverage_flags = \"\"" in line and not added:
                    lines.append("  sanitizer_coverage_allowlist_flags = \"\"\n")
                    added = True
            
            lines.append(line)

    with open(san_gni, "w") as f:
        for line in lines:
            print(line, file=f, end="")

def main():
    add_buildgn()
    add_sanitizer()

if __name__ == "__main__":
    main()