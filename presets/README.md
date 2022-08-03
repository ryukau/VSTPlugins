# Preset
Scripts here convert `*.vstpreset` to `*.json`.

Parameter extraction part is a dirty hack that only works on this repository.

## `extractparameter.py`
`extractparameter.py` extracts parameter information from `PluginName/source/parameter.hpp`, then write the result to `presets/json/PluginName.type.json`.

```bash
cd presets
python3 extractparameter.py
```

**Note**: Current implementation can't read enum value specified with variables. Due to this issue, it skips non-target directory. See `extract_parameter()`.

## `vstpresettojson.py`
`vstpresettojson.py` converts `presets/Uhhyou/PluginName/*.vstpreset` to `presets/json/PluginName.preset.json`.

`*.preset.json` is used for testing.

```bash
cd presets
python3 vstpresettojson.py
```

**Note**: Current implementation skips non-target plugins. See `__main__`.
