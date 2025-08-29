// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)
// Copyright Konstantin Voinov

/**
This source is splitted because nlohmann/json.hpp is slow to compile.
*/
#include "style.hpp"
#include "../../lib/ghc/fs_std.hpp"

#define JSON_DIAGNOSTICS 1
#include "../../lib/json.hpp"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

/**
Specification of $XDG_CONFIG_HOME:
https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html
*/
inline fs::path getConfigHome()
{
#ifdef _WIN32
  const char *appdataDir = std::getenv("AppData");
  if (appdataDir != nullptr) return fs::path(appdataDir);

  std::cerr << "%AppData% is empty.\n";
#elif __APPLE__
  const char *home = std::getenv("HOME");
  if (home != nullptr) return fs::path(home) / "Library/Preferences";

  std::cerr << "$HOME is empty.\n";
#else
  const char *configDir = std::getenv("XDG_CONFIG_HOME");
  if (configDir != nullptr) return fs::path(configDir);

  const char *home = std::getenv("HOME");
  if (home != nullptr) return fs::path(home) / ".config";

  std::cerr << "$XDG_CONFIG_HOME and $HOME is empty.\n";

#endif
  return fs::path("");
}

/**
Load style config from `$XDG_CONFIG_HOME/UhhyouPlugins/style/style.json`.
Returns empty json on failure.
*/
inline nlohmann::json loadStyleJson()
{
  nlohmann::json data;

  auto styleJsonPath = getConfigHome() / fs::path("UhhyouPlugins/style/style.json");

  if (!fs::is_regular_file(styleJsonPath)) {
    std::cerr << styleJsonPath << " is not regular file or doesn't exist.\n";
    return data;
  }

  std::ifstream ifs(styleJsonPath);
  if (!ifs.is_open()) {
    std::cerr << "Failed to open " << styleJsonPath << "\n";
    return data;
  }

  try {
    data = nlohmann::json::parse(ifs);
  } catch (const nlohmann::json::parse_error &e) {
    // output exception information
    std::cerr << "Failed to parse `style.json`. Diagnostic message is below.\n"
              << e.what() << std::endl;
  }
  return data;
}

inline int strHexToUInt8(std::string str)
{
  return std::clamp<int>(std::stoi(str, 0, 16), 0, 255);
}

/**
data[key] must come in string of hex color code. "#123456", "#aabbccdd" etc.
Color will be only loaded if the size of string is either 7 or 9 (RGB or RGBA).
First character is ignored. So "!303030", " 0000ff88" are valid.
*/
inline void loadColor(nlohmann::json &data, std::string key, VSTGUI::CColor &color)
{
  if (!data.contains(key)) return;
  if (!data[key].is_string()) return;

  std::string hex = data[key];

  if (hex.size() != 7 && hex.size() != 9) return;

  color = VSTGUI::CColor(
    strHexToUInt8(hex.substr(1, 2)), strHexToUInt8(hex.substr(3, 2)),
    strHexToUInt8(hex.substr(5, 2)),
    hex.size() != 9 ? 255 : strHexToUInt8(hex.substr(7, 2)));
}

inline void loadString(nlohmann::json &data, std::string key, VSTGUI::UTF8String &value)
{
  if (!data.contains(key)) return;
  if (!data[key].is_string()) return;

  std::string loaded = data[key];
  if (loaded.empty()) return;

  value = loaded;
}

inline void loadFontFace(nlohmann::json &data, std::string key, int targetMask, int &face)
{
  if (!data.contains(key)) return;
  if (!data[key].is_boolean()) return;

  face = data[key] ? (face | targetMask) : (face & (~targetMask));
}

inline void loadFloat(nlohmann::json &data, std::string key, float &value)
{
  if (!data.contains(key)) return;
  if (!data[key].is_number()) return;

  value = data[key].get<float>();
}

void Uhhyou::Palette::load()
{
  auto data = loadStyleJson();
  if (data.is_null()) return;

  loadFloat(data, "guiScale", _guiScale);
  loadString(data, "fontFamily", _fontName);
  loadFontFace(data, "fontBold", VSTGUI::CTxtFace::kBoldFace, _fontFace);
  loadFontFace(data, "fontItalic", VSTGUI::CTxtFace::kItalicFace, _fontFace);
  loadColor(data, "foreground", _foreground);
  loadColor(data, "foregroundButtonOn", _foregroundButtonOn);
  loadColor(data, "foregroundInactive", _foregroundInactive);
  loadColor(data, "background", _background);
  loadColor(data, "boxBackground", _boxBackground);
  loadColor(data, "border", _border);
  loadColor(data, "borderCheckbox", _borderCheckbox);
  loadColor(data, "borderLabel", _borderLabel);
  loadColor(data, "unfocused", _unfocused);
  loadColor(data, "highlightMain", _highlightMain);
  loadColor(data, "highlightAccent", _highlightAccent);
  loadColor(data, "highlightButton", _highlightButton);
  loadColor(data, "highlightWarning", _highlightWarning);
  loadColor(data, "overlay", _overlay);
  loadColor(data, "overlayHighlight", _overlayHighlight);
  loadColor(data, "overlayFaint", _overlayFaint);
}
