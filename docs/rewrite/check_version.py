import argparse
import re
import json
from pathlib import Path

def checkVersion(
    target: str,
    alias: list[str],
    update: bool = False,
) -> str:
    # Read version in manual.
    alias_name = alias[target] if target in alias else target
    json_path = Path(f"../manual/{alias_name}/{alias_name}.json")
    if not json_path.exists():
        return f"{json_path} does not exist."
    with open(json_path, "r", encoding="utf-8") as fi:
        data = json.load(fi)
    manual_version = data[target]["latest_version"]

    # Read version in C++.
    version_hpp = Path(f"../../{target}/source/version.hpp")
    if not version_hpp.exists():
        return f"{version_hpp} does not exist."
    with open(version_hpp, "r", encoding="utf-8") as fi:
        text = fi.read()

    re_flag = re.MULTILINE | re.DOTALL
    major = re.search(r"^#define MAJOR_VERSION_INT ([0-9]+)", text, re_flag).groups()[0]
    minor = re.search(r"^#define SUB_VERSION_INT ([0-9]+)", text, re_flag).groups()[0]
    patch = re.search(r"^#define RELEASE_NUMBER_INT ([0-9]+)", text, re_flag).groups()[0]
    cpp_version = f"{major}.{minor}.{patch}"

    if manual_version == cpp_version:
        return None

    print(f"""{target} version mismatch:
  Manual: {manual_version}
  C++   : {cpp_version}
""")

    if not update:
        return None

    # Update C++ `version.hpp`.
    new_patch = str(int(patch) + 1)
    text = re.sub(
        r"#define RELEASE_NUMBER_INT ([0-9]+)",
        f"#define RELEASE_NUMBER_INT {new_patch}",
        text,
        re_flag,
    )
    text = re.sub(
        r"#define RELEASE_NUMBER_STR \"([0-9]+)\"",
        f"#define RELEASE_NUMBER_STR \"{new_patch}\"",
        text,
        re_flag,
    )

    build = re.search(r"^#define BUILD_NUMBER_INT ([0-9]+)", text, re_flag).groups()[0]
    new_build = str(int(build) + 1)
    text = re.sub(
        r"#define BUILD_NUMBER_INT ([0-9]+)",
        f"#define BUILD_NUMBER_INT {new_build}",
        text,
        re_flag,
    )
    text = re.sub(
        r"#define BUILD_NUMBER_STR \"([0-9]+)\"",
        f"#define BUILD_NUMBER_STR \"{new_build}\"",
        text,
        re_flag,
    )

    with open(version_hpp, "w", encoding="utf-8") as fi:
        fi.write(text)

    return None

def checkAllVersion(update: bool):
    with open("rewrite_target_plugins.json", "r", encoding="utf-8") as fi:
        targets = json.load(fi)
    with open("../../package/manual.json", "r", encoding="utf-8") as fi:
        alias = json.load(fi)

    errors = []
    for target in targets:
        error = checkVersion(target, alias, update)
        if error is not None:
            errors.append(error)

    for error in errors:
        print(error)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Check version and print/update mismatch between manaul and C++ code.",
    )
    parser.add_argument(
        "update",
        type=bool,
        nargs='?',
        default=False,
        help="When set, update C++ version according to manual json.",
    )
    args = parser.parse_args()

    checkAllVersion(args.update)
