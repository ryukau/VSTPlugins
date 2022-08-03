import re
import json
import sys
from pathlib import Path

def convert_string(string: str):
    try:
        value = int(string)
        return value
    except ValueError:
        try:
            value = float(string)
            return value
        except ValueError:
            if string == "true":
                return True
            elif string == "false":
                return False
            return string

def get_enum_range(code):
    # Sort parameter by the order defined in enum ParameterID::ID.
    found = re.findall(
        r"namespace ParameterID \{\nenum ID.*\{([\s\S]*)\};\n\} // namespace ParameterID",
        code)

    enum = []
    for line in found[0].split("\n"):
        line = line.strip()
        if line == "":
            continue
        if line[0] == "/":
            continue
        line = re.split(r"//", line)[0]
        enum.append(line.replace(",", ""))

    temp = []
    index = -1
    for line in enum:
        if line.find("=") == -1:
            index += 1
            temp.append({"name": line, "index": index})
            continue
        name, end_str = line.replace(" ", "").split("=")
        index = int(end_str)
        temp.append({"name": name, "index": index})

    index = 0
    for idx in range(len(enum) - 1):
        temp[idx]["range"] = temp[idx + 1]["index"] - temp[idx]["index"]
    temp.pop() # Delete ID_ENUM_LENGTH

    enum = {}
    for elem in temp:
        name = elem.pop("name")
        enum[name] = elem
    return enum

def extract_plugin_parameter(parameter_hpp_path):
    with open(parameter_hpp_path, "r", encoding="utf-8") as fi:
        code = fi.read()

    plugin_name = re.findall(r"This file is part of (.*)\n", code)[0][:-1]
    enum = get_enum_range(code)

    # Using python struct module format character to represent type.
    # "I" is unsigned int with standard size of 4 bytes.
    # "d" is double with standard size of 8 bytes.
    index = 0
    data = []
    for matched in re.findall(
            r"value\[.*ID::(.*)\]\s*=\s*std::make_unique<(.*)>\(\s*([^,]*),\s*([^,]*),\s*[^,]*,\s*([^\)]*)\);",
            code,
    ):
        datatype = None
        if matched[1] == "UIntValue":
            datatype = "I"
        else:
            datatype = "d"

        if matched[0].find(" + i") != -1:                 # TODO: Change this to regex.
            name, _, _ = matched[0].rpartition(" + i")
            basename = name[0:-1]                         # remove '0'.
            for i in range(enum[name]["range"]):
                default = matched[2].replace("idx", str(i))
                data.append({
                    "id": index,
                    "name": f"{basename}{i}",
                    "type": datatype,
                    "default": convert_string(default),
                    "scale": matched[3],
                    "flags": matched[4],
                })
                index += 1
        else:
            data.append({
                "id": index,
                "name": matched[0],
                "type": datatype,
                "default": convert_string(matched[2]),
                "scale": matched[3],
                "flags": matched[4],
            })
            index += 1

    json_dir = Path(__file__).parent / Path("json")
    json_dir.mkdir(parents=True, exist_ok=True)
    with open(json_dir / Path(f"{plugin_name}.type.json"), "w", encoding="utf-8") as fi:
        json.dump(data, fi, indent=2)

def extract_parameter():
    for path in Path("..").glob("**/parameter.hpp"):
        if path.parts[1] == "_dump":
            continue
        if path.parts[1] != "ClangCymbal":
            continue
        print(path)
        extract_plugin_parameter(path)

def extract_guid():
    re_processor_guid = re.compile(
        r"static const FUID ProcessorUID\(0x(.*), 0x(.*), 0x(.*), 0x(.*)\);")

    data = {}
    for path in Path("..").glob("**/fuid.hpp"):
        if path.parts[1] == "_dump":
            continue

        with open(path, "r", encoding="utf-8") as fi:
            code = fi.read()

        found = re_processor_guid.findall(code)
        data[path.parts[-3]] = {
            "processor_guid": "".join(found[0]),
        }

    with open("json/GUID.json", "w", encoding="utf-8") as fi:
        json.dump(data, fi)

if __name__ == "__main__":
    extract_guid()
    extract_parameter()
