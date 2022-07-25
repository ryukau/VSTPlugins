Control with many blue vertical bars (BarBox) have some keyboard shortcuts. `LFO Wave` on Main tab and `Gain`, `Width`, `Pitch`, `Phase` on Wavetable tab are using BarBox. Shortcuts are enabled after left clicking BarBox and mouse cursor is on the inside of BarBox. Cheat sheet is available on Infomation tab.

| Input                                                      | Control                            |
| ---------------------------------------------------------- | ---------------------------------- |
| <kbd>Left Drag</kbd>                                       | Change Value                       |
| <kbd>Shift</kbd> + <kbd>Left Drag</kbd>                    | Change Value (Snapped)             |
| <kbd>Ctrl</kbd> + <kbd>Left Drag</kbd>                     | Reset to Default                   |
| <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>Left Drag</kbd>  | Change Value (Skip Between Frames) |
| <kbd>Right Drag</kbd>                                      | Draw Line                          |
| <kbd>Shift</kbd> + <kbd>Right Drag</kbd>                   | Edit One Bar                       |
| <kbd>Ctrl</kbd> + <kbd>Right Drag</kbd>                    | Reset to Default                   |
| <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>Right Drag</kbd> | Toggle Lock                        |
| <kbd>a</kbd>                                               | Alternate Sign                     |
| <kbd>d</kbd>                                               | Reset Everything to Default        |
| <kbd>D</kbd>                                               | Toggle Min/Mid/Max                 |
| <kbd>e</kbd>                                               | Emphasize Low                      |
| <kbd>E</kbd>                                               | Emphasize High                     |
| <kbd>f</kbd>                                               | Low-pass Filter                    |
| <kbd>F</kbd>                                               | High-pass Filter                   |
| <kbd>i</kbd>                                               | Invert Value (Preserve minimum)    |
| <kbd>I</kbd>                                               | Invert Value (Minimum to 0)        |
| <kbd>l</kbd>                                               | Toggle Lock under Mouse Cursor     |
| <kbd>L</kbd>                                               | Toggle Lock for All                |
| <kbd>n</kbd>                                               | Normalize (Preserve minimum)       |
| <kbd>N</kbd>                                               | Normalize (Minimum to 0)           |
| <kbd>p</kbd>                                               | Permute                            |
| <kbd>r</kbd>                                               | Randomize                          |
| <kbd>R</kbd>                                               | Sparse Randomize                   |
| <kbd>s</kbd>                                               | Sort Descending Order              |
| <kbd>S</kbd>                                               | Sort Ascending Order               |
| <kbd>t</kbd>                                               | Subtle Randomize (Random walk)     |
| <kbd>T</kbd>                                               | Subtle Randomize (Converge to 0)   |
| <kbd>z</kbd>                                               | Undo                               |
| <kbd>Z</kbd>                                               | Redo                               |
| <kbd>,</kbd> (Comma)                                       | Rotate Back                        |
| <kbd>.</kbd> (Period)                                      | Rotate Forward                     |
| <kbd>1</kbd>                                               | Decrease                           |
| <kbd>2</kbd>-<kbd>9</kbd>                                  | Decrease 2n-9n                     |

Snapping is not available for all BarBox. If you'd like to have snapping for certain BarBox, feel free to open issue to [GitHub repository](https://github.com/ryukau/VSTPlugins).

Edit One Bar with <kbd>Shift</kbd> + <kbd>Right Drag</kbd> holds a bar under the cursor when mouse right button is pressed. Then only changes that one bar while holding down mouse right button.

Toggle Lock with <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>Right Drag</kbd> behaves as line edit. When right mouse button (RMB) is pressed, it holds the opposite state of the bar below mouse cursor, then use the state for the rest of bars. For example, if RMB is pressed on a locked bar, dragging unlocks bars while holding down RMB.

Some BarBox has scroll bar to zoom in. Scroll bar has handles on left end and right end. To control zoom, use <kbd>Left Drag</kbd> on one of the handle. Scroll bar also has following controls:

- <kbd>Right Click</kbd> : Reset zoom.
- <kbd>Mouse Wheel</kbd> : Zoom in/out.
