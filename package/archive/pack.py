import re
import shutil
from pathlib import Path

def copy_resource(src, dst, error_msg):
    dst.mkdir(parents=True, exist_ok=True)
    if src.exists():
        shutil.copytree(src, dst, dirs_exist_ok=True)
    else:
        print(error_msg)

def check_file(file_path):
    """Check existence of a file."""
    if not file_path.is_file():
        print(f"{file_path} does not exist")
        exit(1)

def create_archive_from_github_actions_artifact():
    pack_dir = Path("pack")
    pack_dir.mkdir(exist_ok=True)

    for zip_file in Path().glob("*.zip"):
        shutil.unpack_archive(zip_file, pack_dir, format="zip")

    for dsym in pack_dir.glob("*.dSYM"):
        shutil.rmtree(dsym)

    for vst3_dir in pack_dir.glob("*.vst3"):
        plugin_name = vst3_dir.stem

        # Do not make package if a plugin doesn't have manual.
        if not Path(f"../../docs/manual/{plugin_name}").exists():
            print(f"Skipping {plugin_name}: manual not found")
            continue

        copy_resource(
            Path(f"../../docs/manual/{plugin_name}"),
            vst3_dir / Path("Contents/Resources/Documentation"),
            f"{plugin_name} manual did not found",
        )
        copy_resource(
            Path("../../License"),
            vst3_dir / Path("Contents/Resources/License"),
            "License directory did not found",
        )

        # Ensuring that no binary is missing.
        check_file(vst3_dir / Path(f"Contents/x86_64-win/{plugin_name}.vst3"))
        check_file(vst3_dir / Path(f"Contents/x86_64-linux/{plugin_name}.so"))
        check_file(vst3_dir / Path(f"Contents/MacOS/{plugin_name}"))

        packed_path_str = shutil.make_archive(
            str(vst3_dir.parent / Path(f"{plugin_name}")),
            "zip",
            pack_dir,
            vst3_dir.name,
        )
        packed_path = Path(packed_path_str)

def add_version_to_archive_name():
    re_major_version = re.compile(r"^#define MAJOR_VERSION_INT ([0-9]+)",
                                  flags=re.MULTILINE | re.DOTALL)
    re_minor_version = re.compile(r"^#define SUB_VERSION_INT ([0-9]+)",
                                  flags=re.MULTILINE | re.DOTALL)
    re_patch_version = re.compile(r"^#define RELEASE_NUMBER_INT ([0-9]+)",
                                  flags=re.MULTILINE | re.DOTALL)

    current = list(Path("pack").glob("*.zip"))

    for path in current:
        version_hpp = Path("../../") / Path(path.stem) / Path("source/version.hpp")
        if not version_hpp.exists():
            print(f"{version_hpp} did not found")
            continue

        with open(version_hpp, "r", encoding="utf-8") as fi:
            text = fi.read()

        major = re_major_version.search(text).groups()[0]
        minor = re_minor_version.search(text).groups()[0]
        patch = re_patch_version.search(text).groups()[0]

        new_path = path.parent / Path(f"{path.stem}_{major}.{minor}.{patch}.zip")
        if new_path.exists():
            print(f"Error: {new_path} already exists.")
            exit()
        path.rename(new_path)

if __name__ == "__main__":
    create_archive_from_github_actions_artifact()
    add_version_to_archive_name()
