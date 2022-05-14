# Manual Rewrite
Automation scripts to rewrite manuals.

## `rewrite.py`
`rewrite.py` rewrites all of manual markdown. This is useful when a change is affected to all plugins.

- Bump plugins version in download link.
- Add old version link to Old Versions section.
- Add change log for latest version.

### Usage
Copy `docs/manual/*.md` under `rewrite`.

```bash
cd docs/rewrite
cp -r ../manual .
```

Update following variables in `rewrite.py`:

- `release_name`: Name of release. Tag on git.
- `en_change_log`: Change log text for english manual.
- `ja_change_log`: Change log text for japanese manual.

Run following command to apply changes:

```bash
python3 rewrite.py
```

Rewrited markdowns will be placed to `docs/rewrite/out` directory.

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
