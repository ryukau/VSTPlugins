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

            entry.append(
                {
                    "id": section,
                    "offset": offset,
                    "size": size,
                }
            )

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


def vstpreset_to_json(
    preset_plugin_dir, endian="little", dump_json=True, debug_print_data=False
):
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
        print(vstpreset_path)
        data = read_vstpreset(vstpreset_path, endian)
        if debug_print_data:  # For debug.
            print(f"-- filename\n{vstpreset_path}\n")
            print_data(data)

        # type_data = [{"id":, "name":, "type":, "default":, "scale":, "flags":,}, ... ]
        with open(
            json_dir / Path(f"{plugin_name}.type.json"), "r", encoding="utf-8"
        ) as fi:
            type_data = json.load(fi)

        comp_data = data["Comp"]
        idx = 0
        param = []

        for info in type_data:
            type_char = info["type"]
            n_byte = struct.calcsize(type_char)
            value = struct.unpack(type_char, comp_data[idx : idx + n_byte])[0]
            idx += n_byte
            param.append(
                {
                    "name": info["name"],
                    "type": info["type"],
                    "value": value,
                }
            )

        preset = {}
        preset["name"] = vstpreset_path.stem
        preset["parameter"] = param
        preset_list.append(preset)

    if dump_json:
        with open(
            json_dir / Path(f"{plugin_name}.preset.json"), "w", encoding="utf-8"
        ) as fi:
            json.dump(preset_list, fi, indent=2)
    return preset_list


def write_vstpreset(path, data, endian="little"):
    """
    Write a data dictionary back to a .vstpreset binary file.

    data = {
        "chunk_info": {
            "version": 1,
            "class_id": "32-character ASCII Class ID",
        },
        "Comp": comp_data_bytes,
        "Info": info_xml_string_or_bytes,  # optional
        "Cont": cont_data_bytes,            # optional
    }
    """
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)

    info = data.get("chunk_info", {})
    version = info.get("version", 1)
    class_id = info.get("class_id", "0" * 32)

    header_id = b"VST3"
    version_bytes = version.to_bytes(4, endian)

    # Class ID must be exactly 32 bytes ASCII
    if isinstance(class_id, str):
        class_id_bytes = class_id.encode("ascii").ljust(32, b"\x00")[:32]
    else:
        class_id_bytes = bytes(class_id).ljust(32, b"\x00")[:32]

    payload = b""
    entries = []

    # Offset 0..47 is reserved for Header (4 + 4 + 32 + 8 = 48 bytes)
    current_offset = 48

    # Process binary chunks (Comp, Info, Cont, etc.)
    chunks = {k: v for k, v in data.items() if k != "chunk_info"}
    for section_id, content in chunks.items():
        if isinstance(content, str):
            content_bytes = content.encode("utf-8")
        else:
            content_bytes = bytes(content)

        size = len(content_bytes)
        entries.append((section_id, current_offset, size))
        payload += content_bytes
        current_offset += size

    # Chunk List comes immediately after payload chunks
    chunk_list_offset = current_offset
    chunk_list_offset_bytes = chunk_list_offset.to_bytes(8, endian)

    # Construct Chunk List section
    chunk_list_bytes = b"List" + len(entries).to_bytes(4, endian)
    for section_id, offset, size in entries:
        sec_bytes = section_id.encode("ascii").ljust(4, b"\x00")[:4]
        chunk_list_bytes += sec_bytes
        chunk_list_bytes += offset.to_bytes(8, endian)
        chunk_list_bytes += size.to_bytes(8, endian)

    header = header_id + version_bytes + class_id_bytes + chunk_list_offset_bytes
    file_bytes = header + payload + chunk_list_bytes

    with open(path, "wb") as fo:
        fo.write(file_bytes)


def json_to_vstpreset(
    plugin_name,
    endian="little",
    write_files=True,
    debug_print_data=False,
    json_dir="json",
):
    """
    Inverse of vstpreset_to_json.

    Reads:
      - json/<plugin_name>.preset.json
      - json/<plugin_name>.type.json

    Writes:
      - <preset_plugin_dir>/<preset_name>.vstpreset
    """
    print(f"Processing JSON to VST Preset: {plugin_name}")
    # preset_plugin_dir = Path(preset_plugin_dir)
    # plugin_name = preset_plugin_dir.stem
    # json_dir = Path(__file__).parent / Path("json")

    json_path = json_dir / Path(f"{plugin_name}.preset.json")
    type_path = json_dir / Path(f"{plugin_name}.type.json")

    # Load parameter structure definition
    with open(type_path, "r", encoding="utf-8") as fi:
        type_data = json.load(fi)

    # Load JSON preset list
    with open(json_path, "r", encoding="utf-8") as fi:
        preset_list = json.load(fi)

    created_files = []

    for preset in preset_list:
        preset_name = preset.get("name")
        param_data = preset.get("parameter", [])

        # Build lookup for parameter values (supports list of dicts or name-value dict)
        if isinstance(param_data, list):
            param_map = {
                p["name"]: p["value"]
                for p in param_data
                if isinstance(p, dict) and "name" in p and "value" in p
            }
            param_list = param_data
        elif isinstance(param_data, dict):
            param_map = param_data
            param_list = []
        else:
            param_map = {}
            param_list = []

        # Pack Comp chunk payload according to type specification
        comp_bytes = bytearray()
        for idx, info in enumerate(type_data):
            p_name = info["name"]
            p_type = info["type"]

            val = None
            # Lookup value by index if names match, or by name map
            if (
                idx < len(param_list)
                and isinstance(param_list[idx], dict)
                and param_list[idx].get("name") == p_name
            ):
                val = param_list[idx].get("value")
                if "type" in param_list[idx]:
                    p_type = param_list[idx]["type"]
            elif p_name in param_map:
                val = param_map[p_name]

            if val is None:
                val = 0  # info.get("default", 0)

            # Determine struct format string
            if not p_type.startswith(("<", ">", "=", "@", "!")):
                prefix = "<" if endian == "little" else ">"
                fmt = prefix + p_type
            else:
                fmt = p_type

            # Type conversion before packing
            base_type = p_type.lstrip("<>=@!")
            if base_type in ("f", "d"):
                val = float(val)
            elif base_type in ("i", "I", "h", "H", "b", "B", "q", "Q", "l", "L"):
                val = int(val)

            comp_bytes += struct.pack(fmt, val)

        class_id_path = json_dir / Path("GUID.json")
        with open(class_id_path, "r", encoding="utf-8") as fi:
            data = json.load(fi)
            class_id = data[plugin_name]["processor_guid"]

        # Assemble preset payload chunks
        vstpreset_data = {
            "chunk_info": {
                "version": preset.get("version", 1),
                "class_id": class_id,
            },
            "Comp": bytes(comp_bytes),
        }

        # Include optional metadata sections if available
        for key in ("Info", "info", "Cont", "cont"):
            if key in preset:
                section_name = key.capitalize()
                vstpreset_data[section_name] = preset[key]

        preset_plugin_dir = Path(".")
        vstpreset_path = preset_plugin_dir / f"{preset_name}.vstpreset"

        if write_files:
            write_vstpreset(vstpreset_path, vstpreset_data, endian=endian)
            created_files.append(vstpreset_path)

            if debug_print_data:
                print(f"-- filename\n{vstpreset_path}\n")
                read_data = read_vstpreset(vstpreset_path, endian)
                print_data(read_data)

    return created_files


if __name__ == "__main__":
    # for path in Path("Uhhyou").glob("*"):
    #     if not path.is_dir():
    #         continue
    #     vstpreset_to_json(path, "little")
    # vstpreset_to_json(Path("Uhhyou/SpectralPhaser"), "little", debug_print_data=True)
    json_to_vstpreset("ClangCymbal", endian="little", debug_print_data=True)
