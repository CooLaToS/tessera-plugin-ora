# Changelog

## 1.1.1
- The README's pictures and links point at GitHub, so they show on the Tessera Plugins page too (it does not resolve
  relative paths).
- "Tried on" says the real screens ran Ora 1.0 on Tessera 0.4.99.

## 1.1.0
- Large and Stacked draw the time in Tessera's own clock digits (plugin API 0.29): the flip clock's and the bedside
  clock's, the largest that fits the glass at its own size. The digits are sharp now instead of a smaller font scaled
  up, they fill the 7 and 10-inch screens instead of stopping at three times, and there is no scaled layer to redraw
  each minute (which on a board without PSRAM, the CYD, came out of the small internal memory). Where no font fits
  well, the next larger one is drawn a little smaller. The seconds dot, the hinge and the outline grow with the digits.
- Before the screen's clock is set the boxes stay empty: the clock digits hold no dash.
- Needs a screen with plugin API 0.29 or newer (Tessera 0.4.102, firmware 0.64.0), which also hands a clock face the
  row's icons (the weather icon, MaxGramser/homeassistant_espscreen#448).

## 1.0.1
- Colours only from the plugin API's theme roles: the Tessera mark's four colours, the alert tone, the ink and the
  instrument's dark ground, alone or mixed. Ora no longer borrows the roles of Tessera's own cards (Bauhaus, energy,
  bedside, camera), so a change to one of those cards no longer changes Ora or breaks its build. Gold, orange, red,
  pink and teal are mixes now and look a shade different; red is a little brighter, teal a little bluer.
- The log of each item of the clock's row is verbose now, not debug: a normal log stays quiet at every update.
- Needs a screen with plugin API 0.27 or newer (the instrument's ground).

## 1.0.0
- First version: a face of the screensaver's clock. Hours and minutes in two boxes (flip cards, outline or plain),
  Large, Stacked or Normal; fifteen colours (wood tones and mint among them) with two tones, a box colour of their
  own and a night colour (or dimmer at night); the date in six forms (with or without the weekday), the weather's
  icon and the clock's row of entities, the temperature, leading zero and seconds dot.
- Coloured digits on a coloured box are kept readable: the box darkens until they stand out.
- Needs a screen with plugin API 0.14 or newer. The weather icon needs a Tessera release with the fix for
  MaxGramser/homeassistant_espscreen#448.
