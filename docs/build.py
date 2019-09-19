import subprocess
from pathlib import Path

css_path = Path("style.css").resolve()
template_path = Path("template.html").resolve()
for md in Path(".").glob("**/*.md"):
    subprocess.run([
        "pandoc",
        "-s",
        "--toc",
        "--toc-depth=4",
        "--metadata",
        f"title={md.stem}",
        f"--template={str(template_path.resolve().as_uri())}",
        f"--css={str(css_path.resolve().as_uri())}",
        "-o",
        f"{md.with_suffix('')}.html",
        str(md),
    ])
