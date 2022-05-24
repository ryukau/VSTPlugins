import datetime
import json
import re
import shutil
from pathlib import Path

studiorack_dir = Path("pack/studiorack")

def create_studiorack_archives(platforms):
    """
    This function creates studiorack package as zip archive.
    Returns list of packaged plugin names.
    """
    plugins = []
    studiorack_dir.mkdir(parents=True, exist_ok=True)
    for path in Path("pack").glob("*.zip"):
        stem = str(path.stem).split("_")
        name = stem[0]
        version = stem[1]
        for platform in platforms:
            target_path = studiorack_dir / Path(f"{name}-{platform}{path.suffix}")
            shutil.copyfile(path, target_path)
        plugins.append({"name": name, "version": version})
    return plugins

def format_id(name):
    sp = re.sub(r"([A-Z])", r" \1", name).split()
    return "-".join(sp).lower()

def get_size(path: Path):
    if not path.exists():
        print(f"{path} does not exist")
        exit(1)
    return path.stat().st_size

def extract_description_from_manual(manual_path):
    with open(manual_path, "r", encoding="utf-8") as fi:
        text = fi.read()
    return re.search(r"^# .+?\n!\[\]\(.+?\)\n\n(.+?)\n",
                     text,
                     flags=re.MULTILINE | re.DOTALL).groups()[0]

def create_studiorack_data(plugins, platforms):
    common = {}
    common["author"] = "Takamitsu Endo"
    common["homepage"] = "https://ryukau.github.io/VSTPlugins/"
    common["date"] = f"{datetime.datetime.utcnow().isoformat(timespec='milliseconds')}Z"

    with open("manual.json", "r", encoding="utf-8") as fi:
        manual_dict = json.load(fi)

    re_plugtype = re.compile(
        r"#define\s+stringSubCategory\s+Steinberg::Vst::PlugType::(.*?)$",
        re.DOTALL | re.MULTILINE)

    plugindata = {"plugins": []}
    for plugin_name_splitted in plugins:
        name = plugin_name_splitted["name"]
        plugin_dir = Path(f"../../{name}")

        manual_name = (name if not name in manual_dict else manual_dict[name])

        #
        # On `screenshot_png`, `resource` directory for each plugin only contains 1 png
        # file which is plugin screenshot. So current implementation is just globbing it.
        # Otherwise, `ProcessorUID` should be retrieved from `source/fuid.hpp`.
        #
        plugfactory_cpp = plugin_dir / Path("source/plugfactory.cpp")
        screenshot_png = list((plugin_dir / Path("resource")).glob("*.png"))[0]
        audiosample_wav = Path(f"../audiosample/{name}.wav")
        manual_en_md = Path(f"../../docs/manual/{manual_name}/{manual_name}_en.md")

        if not (plugfactory_cpp.exists() and screenshot_png.exists() and
                audiosample_wav.exists() and manual_en_md.exists()):
            continue

        with open(plugfactory_cpp, "r", encoding="utf-8") as fi:
            factory_text = fi.read()
        mt = re_plugtype.findall(factory_text)
        plugtype = mt[0][1:]
        tags = re.findall(r"[A-Z][^A-Z]*", plugtype)

        data = common.copy()
        data["name"] = name
        data["tags"] = tags
        data["version"] = plugin_name_splitted["version"]
        data["description"] = extract_description_from_manual(manual_en_md)
        data["id"] = format_id(name)
        data["files"] = {} # Will be filled in following sections.

        audiosample_path = studiorack_dir / Path(audiosample_wav.name)
        shutil.copyfile(audiosample_wav, audiosample_path)
        data["files"]["audio"] = {
            "name": audiosample_wav.name,
            "size": get_size(audiosample_path),
        }

        screenshot_name = f"{name}.png"
        screenshot_path = studiorack_dir / Path(screenshot_name)
        shutil.copyfile(screenshot_png, screenshot_path)
        data["files"]["image"] = {
            "name": screenshot_name,
            "size": get_size(screenshot_path)
        }

        for platform in platforms:
            archive_name = f"{name}-{platform}.zip"
            data["files"][platform] = {
                "name": archive_name,
                "size": get_size(studiorack_dir / Path(archive_name))
            }

        plugindata["plugins"].append(data)

    with open(f"{studiorack_dir}/plugins.json", "w", encoding="utf-8") as fi:
        json.dump(plugindata, fi)

if __name__ == "__main__":
    platforms = ["linux", "win"]

    plugins = create_studiorack_archives(platforms)
    create_studiorack_data(plugins, platforms)
