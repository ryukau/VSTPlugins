import re
from pathlib import Path

def compose_download_link(locale, plugin_name, version, download_url):
    if locale == "en":
        return f"- [Download {plugin_name} {version} - VST® 3 (github.com)]({download_url})"
    elif locale == "ja":
        return f"- [{plugin_name} {version} をダウンロード - VST® 3 (github.com)]({download_url})"
    else:
        print('Locale "{locale}" is not available.')
        return None

def process(
    path: Path,
    locale: str,
    re_download_link: re.Pattern,
    re_old_versions: re.Pattern,
    re_change_log: re.Pattern,
    change_log: str,
):
    print(f"Processing {path}")

    with open(path, "r", encoding="utf-8") as fi:
        text = fi.read()

    mt = re_download_link.search(text)
    if mt is None:
        print(f"Download link not found in: {path}")
        return

    plugin_name = mt.groups()[0]
    major_version = mt.groups()[1]
    minor_version = mt.groups()[2]
    patch_version = mt.groups()[3]
    download_url = mt.groups()[4]

    # Add old download link to Old Versions section.
    old_version = f"{major_version}.{minor_version}.{patch_version}"
    old_version_link = f"- [{plugin_name} {old_version} - VST 3 (github.com)]({download_url})"
    text = re_old_versions.sub(lambda exp: f"{exp.group()}\n{old_version_link}",
                               text,
                               count=1)

    # Update download link.
    new_version = f"{major_version}.{minor_version}.{int(patch_version) + 1}"
    new_downlaod_url = f"https://github.com/ryukau/VSTPlugins/releases/download/{release_name}/{plugin_name}_{new_version}.zip"

    new_link = compose_download_link(locale, plugin_name, new_version, new_downlaod_url)
    if new_link is None:
        return

    text = re_download_link.sub(new_link, text, count=1)

    # Add change log.
    text = re_change_log.sub(lambda exp: f"{exp.group()}\n- {new_version}{change_log}",
                             text,
                             count=1)

    out_dir = Path("out") / Path(path.parts[-2])
    out_dir.mkdir(parents=True, exist_ok=True)
    with open(out_dir / Path(path.name), "w", encoding="utf-8") as fi:
        fi.write(text)

if __name__ == "__main__":
    release_name = "ConformVSTGUI4_11"
    en_change_log = """
  - Updated VSTGUI from 4.10 to 4.11."""
    ja_change_log = """
  - VSTGUI を 4.10 から 4.11 にアップデート。"""

    re_en_download_link = re.compile(
        r"^- \[Download (.+?) ([0-9]+)\.([0-9]+)\.([0-9]+) - VST® 3.+?\]\((.*?)\)",
        flags=re.MULTILINE | re.DOTALL,
    )
    re_en_old_versions = re.compile(r"### Old Versions", flags=re.MULTILINE | re.DOTALL)
    re_en_change_log = re.compile(r"^## Change Log", flags=re.MULTILINE | re.DOTALL)

    re_ja_download_link = re.compile(
        r"^- \[(.+?) ([0-9]+)\.([0-9]+)\.([0-9]+) をダウンロード - VST® 3.+?\]\((.*?)\)",
        flags=re.MULTILINE | re.DOTALL,
    )
    re_ja_old_versions = re.compile(r"### 旧バージョン", flags=re.MULTILINE | re.DOTALL)
    re_ja_change_log = re.compile(r"^## チェンジログ", flags=re.MULTILINE | re.DOTALL)

    for path in Path("manual").glob("**/*.md"):
        if path.name.find("_en.md") != -1:
            process(
                path,
                "en",
                re_en_download_link,
                re_en_old_versions,
                re_en_change_log,
                en_change_log,
            )
        elif path.name.find("_ja.md") != -1:
            process(
                path,
                "ja",
                re_ja_download_link,
                re_ja_old_versions,
                re_ja_change_log,
                ja_change_log,
            )
        else:
            print(f"File name doesn't contain locale: {path}")
            continue
