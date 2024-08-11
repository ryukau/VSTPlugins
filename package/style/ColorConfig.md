# UhhyouPlugins Color
If you made a nice color theme, feel free to send a patch to: https://github.com/ryukau/VSTPlugins .

## `style.json`
For details, see GUI Style Configuration section on `docs/manual/common/contact_installation_guiconfig_en.md`.

- [VSTPlugins/contact_installation_guiconfig_en.md at master · ryukau/VSTPlugins](https://github.com/ryukau/VSTPlugins/blob/master/docs/manual/common/contact_installation_guiconfig_en.md#gui-style-configuration)

Default `style.json` is `BlackOnWhite.json`.

## Color Config Source Code
This section describes how to modify source code to add/change colors. Source code is available on following link.

- https://github.com/ryukau/VSTPlugins

Following steps are required to change colors in code.

1. Add color and style to palette.
2. Change GUI component code to add extra style.
3. Change plugin specific GUI code to apply style to individual component.

### Adding Color and Style
Colors and styles are defined in `common/gui/style.hpp` and `common/gui/style.cpp`.

Colors are defined in `Palette` class. To add color, follow the steps below.

1. Add color variable to `Palette`.
2. Add method to read color variable to `Palette`.
3. Add `loadColor()` to `Palette::load()` in `common/gui/style.cpp`.

Styles are defined as `Style` enum class in `common/gui/style.hpp`. It is better to have a context for a style, rather than purely cosmetic one.

### Change GUI Component Code
GUI component codes are placed in `common/gui`.

To add style, look into `onNanoDisplay()` method. There are codes like following:

```c++
// `pal` is defined as `Palette &pal;`.
if constexpr (style == Style::accent) {
  strokeColor(isMouseEntered ? pal.highlightAccent() : pal.unfocused());
} else if (style == Style::warning) {
  strokeColor(isMouseEntered ? pal.highlightWarning() : pal.unfocused());
} else {
  strokeColor(isMouseEntered ? pal.highlightMain() : pal.unfocused());
}
```

When you defined new style, add a branch to these style conditions. Searcing under `common/gui` with a query `if constexpr` may help to find style conditions.

### Change Plugin Specific GUI Code
Each plugin has a specific GUI code at `PluginName/ui.cpp`.

For example, `EsPhaser/ui.cpp` has a code like following:

```c++
public:
  EsPhaserUI() : PluginUIBase(defaultWidth, defaultHeight)
  {
    /* ... */

    using ID = ParameterID::ID;

    const auto top0 = 20.0f;
    const auto left0 = 20.0f;

    // Phaser.
    const auto phaserTop = top0 - margin;
    const auto phaserLeft = left0;

    addKnob(phaserLeft, phaserTop, knobWidth, margin, uiTextSize, "Mix", ID::mix);
    addKnob(
      phaserLeft + knobX, phaserTop, knobWidth, margin, uiTextSize, "Freq",
      ID::frequency);
    addKnob(
      phaserLeft + 2.0f * knobX, phaserTop, knobWidth, margin, uiTextSize, "Spread",
      ID::freqSpread);
    addKnob<Style::warning>(
      phaserLeft + 3.0f * knobX, phaserTop, knobWidth, margin, uiTextSize, "Feedback",
      ID::feedback);

    /* Rest of addSomeComponent codes. */
  }
```

Note that `addKnob<Style::warning>`. `addKnob` and other methods to add components has a template argument for a style. If the style template argument is not set, it defaults to `Style::common`.

To read implementation of `add*()` methods, see `common/uibase.hpp`.
