# Changelog

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
