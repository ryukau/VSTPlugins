# Manual Rewrite
Automation scripts to rewrite manuals.

## `rewrite.py`
`rewrite.py` rewrites json data for manuals as listed below. This is useful when a change affects multiple plugins.

- Bump plugin version.
- Add new download URL.
- Add change log for latest version.

### Usage
Create plugin name list in `rewrite_target_plugins.json` as following. Also see `writePluginList` in `rewrite.py`.

```json
[
  "ClangCymbal",
  "ParallelComb",
]
```

Update following variables in `rewrite.py`:

- `release_name`: Name of release. Tag on git.
- `changelog_en`: Change log text for english manual.
- `changelog_ja`: Change log text for japanese manual.

These variables can be found in `if __name__ == "__main__":`. Example is shown below.

```python
if __name__ == "__main__":
    # ...

    release_name = "UhhyouPlugins1000.0.0"
    changelog_en = [
        "Change log in English.",
        "Multiple log can be written.",
        "Nesting of list can be done.\n    - But not recommended.\n    - Markdown syntax.\n    - 4 spaces indent."
    changelog_ja = [
        "日本語のチェンジログ。",
        "複数のログも書けます。",
        "無理やり入れ子にすることもできます。\n    - ただし非推奨。\n    - Markdown 記法。\n    - インデントはスペース 4 つ。"
    ]
```

Run following command to apply changes to `docs/manual/<PluginName>.json`:

```bash
cd docs/rewrite
python3 rewrite.py
```

## `check_version.py`
`check_version.py` checks version and prints mismatch between manaul and C++ code.

### Usage
```bash
cd docs/rewrite
python3 check_version.py
```

## `verify.py`
`verify.py` verifies urls in all of manual html. Use this to verify download links updated by `rewrite.py`.

### Usage
```bash
cd docs/rewrite
python3 verify.py
```

### Dependencies
- [Beautiful Soup 4](https://www.crummy.com/software/BeautifulSoup/)
- [Requests](https://requests.readthedocs.io/en/master/)
