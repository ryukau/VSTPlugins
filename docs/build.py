import subprocess
import os
from pathlib import Path

css_path = Path("style.css").resolve()
template_path = Path("template.html").resolve()
if os == "nt":
    css_path = css_path.as_uri()
    template_path = template_path.as_uri()

for md in Path(".").glob("**/*.md"):
    subprocess.run([
        "pandoc",
        "-s",
        "--toc",
        "--toc-depth=4",
        "--metadata",
        f"title={md.stem}",
        f"--template={str(template_path)}",
        f"-H",
        css_path,
        "-o",
        f"{md.with_suffix('')}.html",
        str(md),
    ])
