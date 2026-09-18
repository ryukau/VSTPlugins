import copy
import json


def insert_parameter(data: list, index: int, new_param: dict) -> list:
    """
    Inserts `new_param` at a specific `index` in the 'parameter' list
    of every top-level element.
    """
    for item in data:
        if "parameter" in item and isinstance(item["parameter"], list):
            item["parameter"].insert(index, copy.deepcopy(new_param))
    return data


def remove_parameter(data: list, param_name: str) -> list:
    """
    Removes elements with `"name": param_name` from the 'parameter' list
    of every top-level element.
    """
    for item in data:
        if "parameter" in item and isinstance(item["parameter"], list):
            item["parameter"] = [
                p for p in item["parameter"] if p.get("name") != param_name
            ]
    return data


if __name__ == "__main__":
    name = "ClangCymbal"

    # Load from `_PluginName` (leading underscore _).
    with open(f"json/_{name}.preset.json", "r", encoding="utf-8") as fp:
        data = json.load(fp)

    new_param = {"name": "overSampling", "type": "I", "value": 0}
    insert_parameter(data, index=1, new_param=new_param)

    remove_parameter(data, param_name="fdnLowpassCutoffSemiOffset63")

    with open(f"json/{name}.preset.json", "w", encoding="utf-8") as fp:
        json.dump(data, fp, indent=2)
