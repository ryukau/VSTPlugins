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

def get_source_version(plugin_name):
    version_hpp = Path("../../") / Path(plugin_name) / Path("source/version.hpp")

    if not version_hpp.exists():
        raise Exception(f"{version_hpp} was not found")

    with open(version_hpp, "r", encoding="utf-8") as fi:
        text = fi.read()

    re_major_version = re.compile(r"^#define MAJOR_VERSION_INT ([0-9]+)",
                                  flags=re.MULTILINE | re.DOTALL)
    re_minor_version = re.compile(r"^#define SUB_VERSION_INT ([0-9]+)",
                                  flags=re.MULTILINE | re.DOTALL)
    re_patch_version = re.compile(r"^#define RELEASE_NUMBER_INT ([0-9]+)",
                                  flags=re.MULTILINE | re.DOTALL)

    major = re_major_version.search(text).groups()[0]
    minor = re_minor_version.search(text).groups()[0]
    patch = re_patch_version.search(text).groups()[0]

    return (major, minor, patch)

def process(
    path: Path,
    locale: str,
    re_download_link: re.Pattern,
    re_old_versions: re.Pattern,
    re_no_old_versions: re.Pattern,
    re_change_log: re.Pattern,
    change_log: str,
):
    # print(f"Processing {path}") # debug

    with open(path, "r", encoding="utf-8") as fi:
        text = fi.read()

    text = re_no_old_versions.sub("", text)

    matches = list(re_download_link.finditer(text))

    if len(matches) == 0:
        print(f"Download link not found in: {path}")
        return

    hasSinglePlugin = len(matches) == 1
    for mt in matches:
        plugin_name = mt.groups()[0]
        major_version = mt.groups()[1]
        minor_version = mt.groups()[2]
        patch_version = mt.groups()[3]
        download_url = mt.groups()[4]

        source_version = get_source_version(plugin_name)
        if (major_version != source_version[0] or minor_version != source_version[1] or
                int(patch_version) + 1 != int(source_version[2])):
            src_ver = ".".join(source_version)
            man_ver = ".".join([major_version, minor_version, patch_version])
            print(
                f"Warning: {plugin_name} version mismatch. source {src_ver} manual {man_ver}"
            )

        # Update download link.
        new_version = f"{major_version}.{minor_version}.{int(patch_version) + 1}"
        new_downlaod_url = f"https://github.com/ryukau/VSTPlugins/releases/download/{release_name}/{plugin_name}_{new_version}.zip"
        new_link = compose_download_link(locale, plugin_name, new_version,
                                         new_downlaod_url)
        if new_link is None:
            continue

        pos = mt.start()
        text = text[:pos] + re_download_link.sub(new_link, text[pos:], count=1)

        # Add change log.
        if hasSinglePlugin:
            text = re_change_log.sub(
                lambda exp: f"{exp.group()}\n- {new_version}{change_log}", text, count=1)
        else:
            pos = re_change_log.search(text).end()
            text = text[:pos] + re.sub(f"### {plugin_name}",
                                       f"### {plugin_name}\n- {new_version}{change_log}",
                                       text[pos:],
                                       count=1)

        # Add old download link to Old Versions section.
        old_version = f"{major_version}.{minor_version}.{patch_version}"
        old_version_link = f"- [{plugin_name} {old_version} - VST 3 (github.com)]({download_url})"

        if hasSinglePlugin:
            text = re_old_versions.sub(lambda exp: f"{exp.group()}\n{old_version_link}",
                                       text,
                                       count=1)
        else:
            pos = re_old_versions.search(text).end()
            text = text[:pos] + re.sub(f"### {plugin_name}",
                                       f"### {plugin_name}\n{old_version_link}",
                                       text[pos:],
                                       count=1)

    out_dir = Path("out") / Path(path.parts[-2])
    out_dir.mkdir(parents=True, exist_ok=True)
    with open(out_dir / Path(path.name), "w", encoding="utf-8") as fi:
        fi.write(text)

if __name__ == "__main__":
    release_name = "UhhyouPlugins0.34.0"
    en_change_log = """
  - Removed dependency to x86_64 specific SIMD instructions."""
    ja_change_log = """
  - x86_64 固有の SIMD 命令への依存を除去。"""

    re_en_download_link = re.compile(
        r"^- \[Download (\S+?) ([0-9]+)\.([0-9]+)\.([0-9]+) - VST® 3.+?\]\((.*?)\)",
        flags=re.MULTILINE | re.DOTALL,
    )
    re_en_old_versions = re.compile(r"## Old Versions", flags=re.MULTILINE | re.DOTALL)
    re_en_no_old_version = re.compile(r"There is no old versions.\n",
                                      flags=re.MULTILINE | re.DOTALL)
    re_en_change_log = re.compile(r"^## Change Log", flags=re.MULTILINE | re.DOTALL)

    re_ja_download_link = re.compile(
        r"^- \[(\S+?) ([0-9]+)\.([0-9]+)\.([0-9]+) をダウンロード - VST® 3.+?\]\((.*?)\)",
        flags=re.MULTILINE | re.DOTALL,
    )
    re_ja_old_versions = re.compile(r"## 旧バージョン", flags=re.MULTILINE | re.DOTALL)
    re_ja_no_old_version = re.compile(r"現在、旧バージョンはありません。\n",
                                      flags=re.MULTILINE | re.DOTALL)
    re_ja_change_log = re.compile(r"^## チェンジログ", flags=re.MULTILINE | re.DOTALL)

    for path in Path("manual").glob("**/*.md"):
        if path.name.find("_en.md") != -1:
            process(
                path,
                "en",
                re_en_download_link,
                re_en_old_versions,
                re_en_no_old_version,
                re_en_change_log,
                en_change_log,
            )
        elif path.name.find("_ja.md") != -1:
            process(
                path,
                "ja",
                re_ja_download_link,
                re_ja_old_versions,
                re_ja_no_old_version,
                re_ja_change_log,
                ja_change_log,
            )
        else:
            print(f"File name doesn't contain locale: {path}")
            continue
