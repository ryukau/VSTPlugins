import subprocess
import os
import time
from pathlib import Path

def get_last_modified(md):
    result = subprocess.run(
        ["git", "log", "-1", '--date=format:%Y-%m-%d', '--format="%cd"', "--", str(md)],
        stdout=subprocess.PIPE,
        encoding="utf-8",
    )
    if len(result.stdout) > 0:
        return result.stdout.rstrip()[1:-1]  # remove double quotation (").

    # if there are no commits yet, get it from system.
    epoch = Path(md).stat().st_mtime
    return time.strftime("%F", time.localtime(epoch))

index_path = Path("index.html").resolve()
css_path = Path("style.css").resolve()
template_path = Path("template.html").resolve()
if os == "nt":
    css_path = css_path.as_uri()
    template_path = template_path.as_uri()

for md in Path(".").glob("**/*.md"):
    index_relpath = os.path.relpath(str(index_path), str(md.parent.resolve()))
    if index_relpath == "index.html":
        index_relpath = ""

    last_modified = get_last_modified(md)

    subprocess.run([
        "pandoc",
        "-s",
        "--toc",
        "--toc-depth=4",
        "--metadata",
        f"title={md.stem}",
        "--metadata",
        f"date-meta={last_modified}",
        "--metadata",
        f"index-relative-path={index_relpath}",
        f"--template={str(template_path)}",
        f"-H",
        str(css_path),
        "-o",
        f"{md.with_suffix('')}.html",
        str(md),
    ])
