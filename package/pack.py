import re
import json
import shutil
from pathlib import Path

def copy_resource(src, dst, error_msg):
    dst.mkdir(parents=True, exist_ok=True)
    if src.exists():
        shutil.copytree(src, dst, dirs_exist_ok=True)
    else:
        print(error_msg)

def check_file(file_path, missing_on_mac_os=None):
    """Check existence of a file."""
    if not file_path.is_file():
        print(f"{file_path} does not exist")
        if missing_on_mac_os is not None:
            if file_path.stem in missing_on_mac_os:
                return
        exit(1)

def get_missing_on_mac_os():
    with open("../CMakeLists.txt", "r", encoding="utf-8") as fi:
        root_cmake_text = fi.read()

    mt = re.findall(r"if\(NOT APPLE\)(.*?)endif\(\)",
                    root_cmake_text,
                    flags=re.MULTILINE | re.DOTALL)
    return re.findall(r"add_subdirectory\((.+?)\)", mt[0])

def create_archive_from_github_actions_artifact():
    with open("manual.json", "r", encoding="utf-8") as fi:
        manual_dict = json.load(fi)

    pack_dir = Path("pack")
    pack_dir.mkdir(exist_ok=True)

    # For manual packaging.
    for zip_file in Path().glob("*.zip"):
        shutil.unpack_archive(zip_file, pack_dir, format="zip")

    # For GitHub Actions.
    for artifact_dir in Path().glob("vst_*"):
        if not artifact_dir.is_dir():
            continue
        shutil.copytree(artifact_dir, pack_dir, dirs_exist_ok=True)

    for dsym in pack_dir.glob("*.dSYM"):
        shutil.rmtree(dsym)
    for desktop_ini in pack_dir.glob("**/desktop.ini"):
        desktop_ini.unlink(desktop_ini)
    for plugin_ico in pack_dir.glob("**/PlugIn.ico"):
        plugin_ico.unlink(plugin_ico)

    missing_on_mac_os = get_missing_on_mac_os()

    for vst3_dir in pack_dir.glob("*.vst3"):
        plugin_name = vst3_dir.stem

        manual_name = (plugin_name
                       if not plugin_name in manual_dict else manual_dict[plugin_name])

        # Do not make package if a plugin doesn't have manual.
        if not Path(f"../docs/manual/{manual_name}").exists():
            print(f"Skipping {plugin_name}: manual not found")
            continue

        copy_resource(
            Path(f"../docs/manual/{manual_name}"),
            vst3_dir / Path("Contents/Resources/Documentation"),
            f"{plugin_name} manual was not found",
        )
        copy_resource(
            Path("../License"),
            vst3_dir / Path("Contents/Resources/License"),
            "License directory was not found",
        )

        # Ensuring that no binary is missing.
        check_file(vst3_dir / Path(f"Contents/x86_64-win/{plugin_name}.vst3"))
        check_file(vst3_dir / Path(f"Contents/x86_64-linux/{plugin_name}.so"))
        check_file(vst3_dir / Path(f"Contents/MacOS/{plugin_name}"), missing_on_mac_os)

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
        version_hpp = Path("../") / Path(path.stem) / Path("source/version.hpp")
        if not version_hpp.exists():
            print(f"{version_hpp} was not found")
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
