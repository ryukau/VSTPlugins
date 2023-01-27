from pathlib import Path
import re

re_release_number_str = re.compile(r"#define RELEASE_NUMBER_STR \"(\d+)\"")
re_release_number_int = re.compile(r"#define RELEASE_NUMBER_INT (\d+)")
re_build_number_str = re.compile(r"#define BUILD_NUMBER_STR \"(\d+)\"")
re_build_number_int = re.compile(r"#define BUILD_NUMBER_INT (\d+)")

for path in Path(".").glob("*"):
    if not path.is_dir():
        continue
    if not path.stem[0].isupper():
        continue
    version_hpp = path / Path("source/version.hpp")
    if not version_hpp.exists():
        continue

    with open(version_hpp, "r", encoding="utf-8") as fi:
        text = fi.read()
    # print(text)

    mt = re_release_number_int.search(text)
    patch = str(int(mt.groups()[0]) + 1)

    mt = re_build_number_int.search(text)
    build = str(int(mt.groups()[0]) + 1)

    text = re_release_number_str.sub(f"#define RELEASE_NUMBER_STR \"{patch}\"",
                                     text,
                                     count=1)
    text = re_release_number_int.sub(f"#define RELEASE_NUMBER_INT {patch}", text, count=1)
    text = re_build_number_str.sub(f"#define BUILD_NUMBER_STR \"{build}\"", text, count=1)
    text = re_build_number_int.sub(f"#define BUILD_NUMBER_INT {build}", text, count=1)

    with open(version_hpp, "w", encoding="utf-8", newline="\n") as fi:
        fi.write(text)
