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


def create_archive_from_github_actions_artifact(version, pack_dir: Path, isMacOS: bool):
    with open("manual.json", "r", encoding="utf-8") as fi:
        manual_dict = json.load(fi)

    pack_dir.mkdir(exist_ok=True)

    # For manual packaging.
    for zip_file in Path().glob("vst_macOS.zip" if isMacOS else "vst_*.zip"):
        shutil.unpack_archive(zip_file, pack_dir, format="zip")

    # For GitHub Actions.
    for artifact_dir in Path().glob("vst_macOS" if isMacOS else "vst_*"):
        if not artifact_dir.is_dir():
            continue
        shutil.copytree(artifact_dir, pack_dir, dirs_exist_ok=True)

    for dsym in pack_dir.glob("*.dSYM"):
        shutil.rmtree(dsym)
    for desktop_ini in pack_dir.glob("**/desktop.ini"):
        desktop_ini.unlink(desktop_ini)
    for plugin_ico in pack_dir.glob("**/PlugIn.ico"):
        plugin_ico.unlink(plugin_ico)

    archive_postfix = "_macOS" if isMacOS else ""
    for vst3_dir in pack_dir.glob("*.vst3"):
        plugin_name = vst3_dir.stem

        manual_name = (
            plugin_name if not plugin_name in manual_dict else manual_dict[plugin_name]
        )

        # Do not make package if a plugin doesn't have manual.
        plugin_manual_dir = Path("../docs/manual/") / Path(f"{manual_name}")
        if not plugin_manual_dir.exists():
            print(f"Skipping {plugin_name}: manual not found")
            continue

        if isMacOS:
            # No manual to not mess up codesign.
            check_file(vst3_dir / Path(f"Contents/MacOS/{plugin_name}"))
        else:
            copy_resource(
                plugin_manual_dir,
                vst3_dir / Path("Contents/Resources/Documentation"),
                f"{plugin_name} manual was not found",
            )

            check_file(vst3_dir / Path(f"Contents/x86_64-win/{plugin_name}.vst3"))
            check_file(vst3_dir / Path(f"Contents/x86_64-linux/{plugin_name}.so"))
            check_file(vst3_dir / Path(f"Contents/MacOS/{plugin_name}"))

        create_zip_archive(
            vst3_dir.parent
            / Path(f"{plugin_name}_{version[plugin_name]}{archive_postfix}"),
            plugin_name,
            vst3_dir,
            Path(f"../presets/Uhhyou/{plugin_name}"),
        )

    # Create all-in-one archive.
    project_version = version["UhhyouPlugins"]
    all_dir = pack_dir / Path(f"UhhyouPlugins_{project_version}{archive_postfix}")
    all_dir.mkdir(parents=True, exist_ok=True)

    all_plugin_dir = all_dir / Path("plugins")
    all_plugin_dir.mkdir(parents=True, exist_ok=True)

    # Without `list()`, it recursively globs into `all_plugin_dir`.
    globbed = list(pack_dir.glob("**/*.vst3"))

    for plugin_dir in globbed:
        if not plugin_dir.is_dir():
            continue
        if plugin_dir.parent == pack_dir:
            continue
        shutil.move(plugin_dir, all_plugin_dir)

    all_preset_dir = all_dir / Path("presets")
    all_preset_dir.mkdir(parents=True, exist_ok=True)

    src_preset_dir = Path("../presets/Uhhyou")
    shutil.copytree(
        src_preset_dir, all_preset_dir / src_preset_dir.parts[-1], dirs_exist_ok=True
    )

    shutil.make_archive(all_dir, "zip", all_dir)


def create_zip_archive(archive_dir, plugin_name, vst3_dir, presets_dir):
    archive_dir.mkdir(parents=True, exist_ok=True)

    archive_vst3_dir = archive_dir / vst3_dir.name
    if archive_vst3_dir.exists():
        shutil.rmtree(archive_vst3_dir)
    shutil.move(vst3_dir, archive_dir)

    presets_dir = Path(f"../presets/Uhhyou/{plugin_name}")
    if presets_dir.exists():
        target_dir = Path(*presets_dir.parts[1:])  # Remove relative part.
        shutil.copytree(presets_dir, archive_dir / target_dir, dirs_exist_ok=True)

    packed_path_str = shutil.make_archive(
        archive_dir,
        "zip",
        archive_dir,
    )


def get_version():
    re_major_version = re.compile(
        r"^#define MAJOR_VERSION_INT ([0-9]+)", flags=re.MULTILINE | re.DOTALL
    )
    re_minor_version = re.compile(
        r"^#define SUB_VERSION_INT ([0-9]+)", flags=re.MULTILINE | re.DOTALL
    )
    re_patch_version = re.compile(
        r"^#define RELEASE_NUMBER_INT ([0-9]+)", flags=re.MULTILINE | re.DOTALL
    )

    with open("../CMakeLists.txt", "r", encoding="utf-8") as fi:
        top_level_cmake_text = fi.read()
    plugins = re.findall(r"add_subdirectory\(([A-Z].+?)\)", top_level_cmake_text)

    version = {}

    version["UhhyouPlugins"] = re.search(
        r"project\(UhhyouPlugins\s+VERSION\s+([0-9.]+)\)",
        top_level_cmake_text,
        flags=re.MULTILINE | re.DOTALL,
    ).groups()[0]

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
    create_archive_from_github_actions_artifact(version, Path("pack"), False)
    create_archive_from_github_actions_artifact(version, Path("pack_macOS"), True)
