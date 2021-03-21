"""
This script converts *.vstpreset to json. Requires parameter data structure defined in plugin.

VST 3 Preset file specification:
https://steinbergmedia.github.io/vst3_doc/vstinterfaces/vst3loc.html#presetformat

Note: offset is number of bytes from start of vstpreset file. Arrows in the figure at above link are misleading.
"""

import json
import struct
import sys

from pathlib import Path

def print_data(data):
    """
    data = {
        "chunk_info" : { ... },
        "entry0" : data0,
        "entry1" : data1,
        ...
    }
    """
    for key, chunk in data.items():
        print(f"-- {key}")
        if key == "chunk_info":
            for name, value in chunk.items():
                print(f"{name:<24} {value}")
        else:
            print(chunk)
        print()

def read_vstpreset(path, endian):
    """
    Return value for vstpreset version 1.

    data = {
        "chunk_info" : { ... },
        "Comp" : Comp_data_in_byte_sequence,
        "Cont" : Cont_data_in_byte_sequence,
        "Info" : Info_xml_in_utf-8_text,
    }
    """
    data = {}
    with open(path, "rb") as fi:
        info = {}
        info["header_id"] = fi.read(4).decode("ascii")
        if info["header_id"] != "VST3":
            raise RuntimeError(f"{path} is not vstpreset.")
        info["version"] = int.from_bytes(fi.read(4), endian)
        info["class_id"] = fi.read(32).decode("ascii")
        info["chunk_list_offset"] = int.from_bytes(fi.read(8), endian)

        # Skip to CHUNK LIST section.
        fi.seek(info["chunk_list_offset"], 0)

        info["list_id"] = fi.read(4).decode("ascii")
        info["entry_count"] = int.from_bytes(fi.read(4), endian)

        entry = []
        for n in range(info["entry_count"]):
            section = fi.read(4).decode("ascii")
            offset = int.from_bytes(fi.read(8), endian)
            size = int.from_bytes(fi.read(8), endian)

            entry.append({
                "id": section,
                "offset": offset,
                "size": size,
            })

            info[f"{section}_id"] = section
            info[f"{section}_offset"] = offset
            info[f"{section}_size"] = size

        data["chunk_info"] = info

        for ent in entry:
            if ent["id"] == "Comp":
                fi.seek(ent["offset"], 0)
                data[ent["id"]] = fi.read(ent["size"])
            elif ent["id"] == "Info":
                fi.seek(ent["offset"], 0)
                data[ent["id"]] = fi.read(ent["size"]).decode("utf-8")
            else:
                fi.seek(ent["offset"], 0)
                data[ent["id"]] = fi.read(ent["size"])
    return data

def vstpreset_to_json(preset_plugin_dir,
                      endian="little",
                      dump_json=True,
                      debug_print_data=False):
    """
    preset_list["data"] is optional debug info.

    preset_list = [
        { "name": name, "parameter", {"param_name": value, ...} },
        ...
    ]
    """
    print(f"Processing: {preset_plugin_dir}")
    preset_plugin_dir = Path(preset_plugin_dir)
    plugin_name = preset_plugin_dir.stem
    json_dir = Path(__file__).parent / Path("json")

    preset_list = []
    for vstpreset_path in sorted(preset_plugin_dir.glob("**/*.vstpreset")):
        data = read_vstpreset(vstpreset_path, endian)
        if debug_print_data: # For debug.
            print(f"-- filename\n{vstpreset_path}\n")
            print_data(data)

        # type_data = [{"id":, "name":, "type":, "default":, "scale":, "flags":,}, ... ]
        with open(json_dir / Path(f"{plugin_name}.type.json"), "r") as fi:
            type_data = json.load(fi)

        comp_data = data["Comp"]
        idx = 0
        param = []

        for info in type_data:
            # TODO: delete this branch after fixing LPS presets.
            if preset_plugin_dir.stem == "LightPadSynth" and (info["id"] == 1506 or
                                                              info["id"] == 1513):
                param.append({
                    "name": info["name"],
                    "type": info["type"],
                    "value": info["default"],
                })
                continue

            type_char = info["type"]
            n_byte = struct.calcsize(type_char)
            value = struct.unpack(type_char, comp_data[idx:idx + n_byte])[0]
            idx += n_byte
            param.append({
                "name": info["name"],
                "type": info["type"],
                "value": value,
            })

        preset = {}
        preset["name"] = vstpreset_path.stem
        preset["parameter"] = param
        preset_list.append(preset)

    if dump_json:
        with open(json_dir / Path(f"{plugin_name}.preset.json"), "w") as fi:
            json.dump(preset_list, fi, indent=2)
    return preset_list

if __name__ == "__main__":
    for path in Path("Uhhyou").glob("*"):
        if not path.is_dir():
            continue
        vstpreset_to_json(path, "little")
    # vstpreset_to_json(Path("Uhhyou/SoftClipper"), "little", debug_print_data=True)
