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

def check_file(file_path):
    """Check existence of a file."""
    if not file_path.is_file():
        print(f"{file_path} does not exist")
        exit(1)

def create_full_archive_from_github_actions_artifact(version):
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

        # Ensuring that no binary is missing.
        check_file(vst3_dir / Path(f"Contents/x86_64-win/{plugin_name}.vst3"))
        check_file(vst3_dir / Path(f"Contents/x86_64-linux/{plugin_name}.so"))
        check_file(vst3_dir / Path(f"Contents/MacOS/{plugin_name}"))

        packed_path_str = shutil.make_archive(
            str(vst3_dir.parent / Path(f"{plugin_name}_{version[plugin_name]}")),
            "zip",
            pack_dir,
            vst3_dir.name,
        )

def create_macos_archive_from_github_actions_artifact(version):
    """
    Ad-hoc method. Merge this to `create_full_archive_from_github_actions_artifact`
    in future. Also remove `pack_macOS` line from `.github/workflows/build.yml`.
    """
    with open("manual.json", "r", encoding="utf-8") as fi:
        manual_dict = json.load(fi)

    pack_dir = Path("pack_macOS")
    pack_dir.mkdir(exist_ok=True)

    # For manual packaging.
    for zip_file in Path().glob("vst_macOS.zip"):
        shutil.unpack_archive(zip_file, pack_dir, format="zip")

    # For GitHub Actions.
    for artifact_dir in Path().glob("vst_macOS"):
        if not artifact_dir.is_dir():
            continue
        shutil.copytree(artifact_dir, pack_dir, dirs_exist_ok=True)

    for dsym in pack_dir.glob("*.dSYM"):
        shutil.rmtree(dsym)

    for vst3_dir in pack_dir.glob("*.vst3"):
        plugin_name = vst3_dir.stem

        manual_name = (plugin_name
                       if not plugin_name in manual_dict else manual_dict[plugin_name])

        # Do not make package if a plugin doesn't have manual.
        if not Path(f"../docs/manual/{manual_name}").exists():
            print(f"Skipping {plugin_name}: manual not found")
            continue

        # Ensuring that no binary is missing.
        check_file(vst3_dir / Path(f"Contents/MacOS/{plugin_name}"))

        packed_path_str = shutil.make_archive(
            str(vst3_dir.parent / Path(f"{plugin_name}_{version[plugin_name]}_macOS")),
            "zip",
            pack_dir,
            vst3_dir.name,
        )

def get_version():
    re_major_version = re.compile(r"^#define MAJOR_VERSION_INT ([0-9]+)",
                                  flags=re.MULTILINE | re.DOTALL)
    re_minor_version = re.compile(r"^#define SUB_VERSION_INT ([0-9]+)",
                                  flags=re.MULTILINE | re.DOTALL)
    re_patch_version = re.compile(r"^#define RELEASE_NUMBER_INT ([0-9]+)",
                                  flags=re.MULTILINE | re.DOTALL)

    with open("../CMakeLists.txt", "r", encoding="utf-8") as fi:
        top_level_cmake_text = fi.read()
    plugins = re.findall(r"add_subdirectory\(([A-Z].+?)\)", top_level_cmake_text)

    version = {}
    for plugin_name in plugins:
        version_hpp = Path("../") / Path(plugin_name) / Path("source/version.hpp")
        if not version_hpp.exists():
            print(f"{version_hpp} was not found")
            continue

        with open(version_hpp, "r", encoding="utf-8") as fi:
            text = fi.read()

        major = re_major_version.search(text).groups()[0]
        minor = re_minor_version.search(text).groups()[0]
        patch = re_patch_version.search(text).groups()[0]

        version[plugin_name] = f"{major}.{minor}.{patch}"
    return version

if __name__ == "__main__":
    version = get_version()
    create_macos_archive_from_github_actions_artifact(version)
    create_full_archive_from_github_actions_artifact(version)
