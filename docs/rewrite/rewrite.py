import json
from pathlib import Path

def writePluginList():
    # It's better to use TOML because it has comment, but waiting for python 3.11.
    root = Path("../..")
    plugins = []
    for path in root.glob("*"):
        if not (path.is_dir() and path.stem[0].isupper() and path.stem[0].isascii()):
            continue
        if path.name == "License":
            continue
        plugins.append(path.name)
    with open("rewrite_target_plugins.json", "w", encoding="utf-8") as fi:
        json.dump(plugins, fi, indent=2)

def updateVersion(
    target: str,
    alias: dict[str, str],
    changelog_en: list[str],
    changelog_ja: list[str],
    release_name: str,
):
    # Load JSON.
    alias_name = alias[target] if target in alias else target
    json_path = Path(f"../manual/{alias_name}/{alias_name}.json")

    if not json_path.exists():
        print(f"{json_path} does not exist.")
        return

    with open(json_path, "r", encoding="utf-8") as fi:
        data = json.load(fi)

    # Bump version.
    old_version = data[target]["latest_version"]
    ver = old_version.split(".")
    ver[-1] = str(int(ver[-1]) + 1)
    new_version = ".".join(ver)
    data[target]["latest_version"] = new_version

    # Insert change log. Preserving order of elements.
    new_changelog = {
        new_version: {
            "en": changelog_en,
            "ja": changelog_ja,
        }
    }
    new_changelog.update(data[target]["changelog"])
    data[target]["changelog"] = new_changelog

    # Update plugin URL.
    new_plugin_url = f"https://github.com/ryukau/VSTPlugins/releases/download/{release_name}/{target}_{new_version}.zip"
    data[target]["urls"]["plugin_url"].insert(0, new_plugin_url)

    with open(json_path, "w", encoding="utf-8") as fi:
        json.dump(data, fi, ensure_ascii=False, indent=2)
        fi.write("\n")

def appendChangeLog(
    target: str,
    alias: dict[str, str],
    changelog_en: list[str],
    changelog_ja: list[str],
):
    # Load JSON.
    alias_name = alias[target] if target in alias else target
    json_path = Path(f"../manual/{alias_name}/{alias_name}.json")

    if not json_path.exists():
        print(f"{json_path} does not exist.")
        return

    with open(json_path, "r", encoding="utf-8") as fi:
        data = json.load(fi)

    # Insert change log. Preserving order of elements.
    version = data[target]["latest_version"]

    target_log = data[target]["changelog"][version]
    target_log["en"].extend(changelog_en)
    target_log["ja"].extend(changelog_ja)

    print(target_log)
    exit()

    # with open(json_path, "w", encoding="utf-8") as fi:
    #     json.dump(data, fi, ensure_ascii=False, indent=2)
    #     fi.write("\n")

if __name__ == "__main__":
    ## Use `writePluginList` to re-generate `rewrite_target_plugins.json`.
    # writePluginList()
    # exit()

    with open("rewrite_target_plugins.json", "r", encoding="utf-8") as fi:
        targets = json.load(fi)
    with open("../../package/manual.json", "r", encoding="utf-8") as fi:
        alias = json.load(fi)

    release_name = "UhhyouPlugins0.53.0"
    changelog_en = [
        "Fixed crash on Renoise. Previous code was trying to access freed memory of GUI widgets, and it was causing the crash.",
    ]
    changelog_ja = [
        "Renoise で GUI がクラッシュするバグを修正。 GUI 要素のメモリが解放された後にアクセスを行おうとしていたことがクラッシュの原因。",
    ]
    for target in targets:
        updateVersion(target, alias, changelog_en, changelog_ja, release_name)
