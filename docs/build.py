import argparse
import json
import os
import subprocess
import time
from pathlib import Path

def read_build_info(rebuild=False):
    if not rebuild and Path("buildinfo.json").exists():
        with open("buildinfo.json", "r") as build_info:
            return json.load(build_info)
    return {}

def get_last_modified(md):
    result = subprocess.run(
        ["git", "log", "-1", '--date=format:%Y-%m-%d', '--format="%cd"', "--",
         str(md)],
        stdout=subprocess.PIPE,
        encoding="utf-8",
    )
    if len(result.stdout) > 0:
        return result.stdout.rstrip()[1:-1] # remove double quotation (").

    # if there are no commits yet, get it from system.
    epoch = Path(md).stat().st_mtime
    return time.strftime("%F", time.localtime(epoch))

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-r", "--rebuild", action='store_true', help="rebuild all file")
    parser.add_argument("-t",
                        "--today",
                        action='store_true',
                        help="use timestamp of today")
    args = parser.parse_args()

    md_info = read_build_info(args.rebuild)

    index_path = Path("index.html").resolve()
    css_path = Path("style.css").resolve()
    template_path = Path("template.html").resolve()
    if os == "nt":
        css_path = css_path.as_uri()
        template_path = template_path.as_uri()

    for md in Path(".").glob("**/*.md"):
        if md.stem == "README" or md.parts[0] == "rewrite":
            continue

        html = md.parent / Path(md.stem + ".html")
        mtime = os.path.getmtime(md)
        if str(md) not in md_info:
            md_info[str(md)] = {"mtime": mtime}
        elif html.exists() and md_info[str(md)]["mtime"] == mtime:
            continue
        md_info[str(md)]["mtime"] = mtime

        print(f"Processing {md}")

        index_relpath = os.path.relpath(str(index_path), str(md.parent.resolve()))
        if index_relpath == "index.html":
            index_relpath = ""

        last_modified = time.strftime("%F") if args.today else get_last_modified(md)

        subprocess.run([
            "pandoc",
            "--standalone",
            "--toc",
            "--toc-depth=4",
            "--metadata",
            f"title={md.stem}",
            "--metadata",
            f"date-meta={last_modified}",
            "--metadata",
            f"index-relative-path={index_relpath}",
            f"--template={str(template_path)}",
            f"--include-in-header={str(css_path)}",
            f"--output={md.with_suffix('')}.html",
            str(md),
        ])

    with open("buildinfo.json", "w") as fi:
        json.dump(md_info, fi)
