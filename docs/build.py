import subprocess
import os
from pathlib import Path

css_path = Path("style.css")
template_path = str(Path("template.html").resolve().as_uri())
for md in Path(".").glob("**/*.md"):
    relative_css_path = os.path.relpath(css_path, md.parent).replace('\\', '/')
    subprocess.run([
        "pandoc",
        "-s",
        "--toc",
        "--toc-depth=4",
        "--metadata",
        f"title={md.stem}",
        f"--template={template_path}",
        f"--css={relative_css_path}",
        "-o",
        f"{md.with_suffix('')}.html",
        str(md),
    ])
