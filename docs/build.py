import argparse
import json
import os
import subprocess
import time
import jinja2
import re
import yaml
from pathlib import Path

def loadCommonSection(language):
    def load(name, lang):
        with open(f"manual/common/{name}_{lang}.md", "r", encoding="utf-8") as fi:
            text = fi.read().strip()
        return text

    names = [
        "macos_warning",
        "contact_installation_guiconfig",
        "gui_common",
        "gui_barbox",
        "gui_knob",
    ]
    return {name: load(name, language) for name in names}

def getLanguages(ref_filename="manual/common/gui_common"):
    languages = []
    for path in Path(".").glob(ref_filename + "*"):
        languages.append(path.stem.split("_")[-1])
    return languages

def extractVersion(url):
    mt = re.search(r"(\d+\.\d+\.\d+)\.zip", url)
    return mt.groups()[0]

def loadManualJson(manual_dir, dest):
    manual = manual_dir.name
    with open(manual_dir / Path(f"{manual}.json"), "r", encoding="utf-8") as fi:
        try:
            full_data = json.load(fi)
        except:
            print(f"Failed to load {manual}.json\n")
            raise

    dest[manual] = {
        "latest_version": {},
        "latest_download_url": {},
        "preset_download_url": {},
        "old_download_link": {},
        "changelog": {},
    }
    for plugin, src in full_data.items():
        dest[manual]["latest_version"][plugin] = src["latest_version"]
        dest[manual]["latest_download_url"][plugin] = src["urls"]["plugin_url"][0]
        dest[manual]["preset_download_url"][plugin] = src["urls"]["preset_url"]

        dest[manual]["old_download_link"][plugin] = [{
            "url": url,
            "version": extractVersion(url)
        } for url in src["urls"]["plugin_url"][1:]]

        dest[manual]["changelog"][plugin] = src["changelog"]

def composeMarkdown(markdown, manual_data, section):
    env = jinja2.Environment(loader=jinja2.FileSystemLoader("."))
    data = manual_data[markdown.parent.name]
    language = markdown.stem.split("_")[-1]
    return env.get_template(markdown.as_posix()).render(
        section=section[language],
        latest_version=data["latest_version"],
        latest_download_url=data["latest_download_url"],
        preset_download_url=data["preset_download_url"],
        old_download_link=data["old_download_link"],
        changelog=data["changelog"],
    )

def read_json_mtime():
    info = {}
    for manual_dir in Path("manual").glob("*"):
        if manual_dir.name == "common":
            continue
        json_path = manual_dir / Path(f"{manual_dir.name}.json")
        info[str(json_path)] = {"mtime": os.path.getmtime(json_path)}
    return info

def is_source_modified(md, html):
    if not html.exists():
        return True

    html_mtime = os.path.getmtime(html)

    if "manual" in md.parts and "common" not in md.parts:
        json_path = md.parent / Path(f"{md.parent.name}.json")
        json_mtime = os.path.getmtime(json_path)
        if json_mtime > html_mtime:
            return True

    md_mtime = os.path.getmtime(md)
    return md_mtime > html_mtime

def dump_config_yml():
    md_list = sorted([str(md.as_posix()) for md in Path(".").glob("**/*.md")])
    with open("_config.yml", "w") as outfile:
        yaml.dump({"exclude": md_list}, outfile, default_flow_style=False)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-r", "--rebuild", action='store_true', help="rebuild all file")
    parser.add_argument("-t",
                        "--today",
                        action='store_true',
                        help="use timestamp of today")
    args = parser.parse_args()

    dump_config_yml()

    index_path = Path("index.html").resolve()
    css_path = Path("style.css").resolve()
    template_path = Path("template.html").resolve()
    if os == "nt":
        css_path = css_path.as_uri()
        template_path = template_path.as_uri()

    section = {lang: loadCommonSection(lang) for lang in getLanguages()}

    manual_data = {}
    for manual_dir in Path("manual").glob("*"):
        if manual_dir.name == "common":
            continue
        loadManualJson(manual_dir, manual_data)

    for md in Path(".").glob("**/*.md"):
        if md.stem == "README" or md.parts[0] == "rewrite" or "common" in md.parts:
            continue

        html = md.parent / Path(md.stem + ".html")
        mtime = os.path.getmtime(md)
        if not is_source_modified(md, html) and not args.rebuild:
            continue

        print(f"Processing {md}")

        index_relpath = os.path.relpath(str(index_path), str(md.parent.resolve()))
        if index_relpath == "index.html":
            index_relpath = ""

        pandoc_command = [
            "pandoc",
            "--standalone",
            "--toc",
            "--toc-depth=4",
            "--metadata",
            f"title={md.stem}",
            "--metadata",
            f"date-meta={time.strftime('%F')}",
            "--metadata",
            f"index-relative-path={index_relpath}",
            f"--template={str(template_path)}",
            f"--include-in-header={str(css_path)}",
            f"--output={html}",
        ]

        if md.parts[0] == "manual":
            composed = composeMarkdown(md, manual_data, section)
            subprocess.run(pandoc_command, input=composed, encoding="utf-8")
        else:
            subprocess.run(pandoc_command + [str(md)])
