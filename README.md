# Ora

A big clock for Tessera's screensaver, in the colours you choose. Ora (Greek for "hour") shows the hours and the
minutes in two boxes as large as the screen allows, with the date and the outside temperature under them.

## Set up

1. In Tessera, open **Plugins**, choose **Add with a link** and paste `https://github.com/CooLaToS/tessera-plugin-ora`
   (or find **Ora** in the list once it is listed there). Add it to a screen; the screen builds once.
2. Open the screen's **Settings** and the **Screensaver** card. Turn on **Show in standby** (the screen needs
   **Auto standby** on too).
3. Click the **Clock** step and choose **Ora** under **Face**.
4. Pick the colours, the style and the size in the same drawer.

## Options

- **Hours colour** and **Minutes colour**: Gold (default), Champagne, Amber, Orange, Red, Pink, Purple, Blue, Teal,
  Green, Oak, Walnut, Mint, White or Night grey. Give the minutes their own colour for two tones.
- **Box colour**: Graphite flip cards, cards Tinted with the hours' colour, or any colour above. With **Outline** it is
  the colour of the line.
- **Style**: Flip cards, Outline, or Plain digits with a colon.
- **Size**: Large (side by side, as wide as the screen; best on a square or wide screen), Stacked (hours above
  minutes; best on a tall screen) or Normal (the screen's own digit size).
- **Date**: Fri 20 Nov, 20/11, 20 November, Friday 20 November, Friday, or no date.
- **Weather and entities**: the entities you add to the Clock step's row (the **+** beside the clock in the Clock
  drawer). Add your weather entity with **icon only** and Ora shows its condition, sun, cloud or rain, before the
  temperature. Icons (default), icons and words, or off. Up to four; those that do not fit the width are left out.
- **Date colour** and **Temperature colour** (also the colour of the row's icons).
- **Night colour**: during the screen's night hours the same colours, the same but dimmer, or one colour for the whole
  clock (deep red is easy on the eyes).
- **Leading zero** (09:05 or 9:05) and **Seconds dot**.

## Good to know

- Any colour on any box stays readable: when the digits would not stand out enough from their box (red on blue, say),
  Ora darkens the box until they do.
- The weather icon needs a Tessera release with the fix for
  [MaxGramser/homeassistant_espscreen#448](https://github.com/MaxGramser/homeassistant_espscreen/issues/448): up to
  Tessera 0.4.99 a plugin's clock face receives the row's icons as 0, so Ora shows the date and the temperature without
  the icon. Nothing to change in Ora: the icon appears once Tessera hands it over.

- Large and Stacked scale the screen's largest digits up (a plugin brings no font of its own), so they are a little
  softer than the font's own size.
- The night colour is chosen when the screensaver comes on. If the night hours begin while the clock is already showing,
  it changes the next time the screen goes to standby.
- The screen moves the whole clock a millimetre every minute, so nothing stands on the same pixels for hours.
- A screen without the plugin shows Tessera's own clock.
- Tried on: Guition 4-inch (480 x 480); on the bench also the CYD 2.8-inch, the Waveshare 4.3-inch standing
  up and the Guition 10.1-inch.

## Tests

```sh
uv run tests/run.py              # unit, check and bench
uv run tests/run.py unit         # only the unit tests of the face's logic (seconds)
```

- **unit**: `components/ora/ora_logic.h` (the choices, the time and date texts, the line under the clock and where every
  part stands on every kind of screen) compiled for your computer with the sanitizers, `tests/test_logic.cpp`.
- **check**: Tessera's `tools/check.py`.
- **bench**: Tessera's bench in strict mode, the real firmware with Ora built for your computer: the showcase on four
  kinds of screen, then the layout combinations of `tests/combos.py` on the Guition 4-inch. Any finding fails.
  `ORA_FULL=1` renders all 486 combinations.

The check and the bench need a clone of [tessera-plugins](https://github.com/MaxGramser/tessera-plugins) beside this
repository (or `TESSERA_PLUGINS=<path>`), its bench set up once (`python3 tools/preview.py --setup` there).

## Credits

The layout of the flip cards follows Tessera's own Flip clock example (MIT, Max Gramser). Ora is written from
Tessera's plugin template.
