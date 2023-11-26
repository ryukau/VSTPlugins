import shutil
from pathlib import Path

# `print` are added for debugging GitHub Actions.
print("presets/pack.py: Packing presets.")

if __name__ == "__main__":
    pack_dir = Path("pack")
    pack_dir.mkdir(parents=True, exist_ok=True)

    for preset_dir in Path("Uhhyou").glob("*"):
        if not preset_dir.is_dir():
            continue
        packed_path_str = shutil.make_archive(
            str(pack_dir / Path(f"{preset_dir.name}_Presets")),
            "zip",
            ".",
            preset_dir,
        )
        print(packed_path_str)
