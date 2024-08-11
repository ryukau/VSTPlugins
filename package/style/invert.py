import json


def hexColorToRgba(hexColorStr: str) -> list[int]:
    stripped = valueStr[1:]
    if len(stripped) == 6:
        stripped += "ff"
    return [int(stripped[i : i + 2], 16) for i in (0, 2, 4, 6)]


def rgbaToHexColor(rgba: list[int]) -> str:
    alpha = rgba[-1]
    if alpha == 0xFF:
        return f"#{rgba[0]:x}{rgba[1]:x}{rgba[2]:x}"
    return f"#{rgba[0]:x}{rgba[1]:x}{rgba[2]:x}{rgba[3]:x}"


def invertColor(rgba: list[int], ratio: float) -> list[int]:
    dest = rgba.copy()
    for i in range(3):
        dest[i] = int(dest[i] + ratio * (0xFF - 2 * dest[i]))
    return dest


if __name__ == "__main__":
    with open("themes/Orbit.json", "r", encoding="utf-8") as fp:
        data = json.load(fp)

    for key, valueStr in data.items():
        if key == "fontPath":
            continue

        rgba = hexColorToRgba(valueStr)
        inverted = invertColor(rgba, 1 - 0x20 / 0xFF)
        invStr = rgbaToHexColor(inverted)

        print(valueStr, invStr)

        data[key] = invStr

    with open("style.json", "w", encoding="utf-8") as fp:
        json.dump(data, fp)
