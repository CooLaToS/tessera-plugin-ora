# Contributing to Ora

Thanks for helping. Ora is a hobby project, made in spare time and without warranty, so there is no guaranteed
response time and not every idea gets built.

## Reporting a bug

1. Check it is Ora: choose Tessera's own clock under **Face** in the Screensaver card. If the problem stays, it is
   Tessera's: report it at [MaxGramser/homeassistant_espscreen](https://github.com/MaxGramser/homeassistant_espscreen/issues).
2. Read "Good to know" in the README (a missing weather icon is a known Tessera issue up to 0.4.99).
3. Update to the newest Ora and search the open and closed issues.
4. Open a [bug report](https://github.com/CooLaToS/tessera-plugin-ora/issues/new/choose) with Ora's version, the
   screen, Tessera's and the firmware's version, Ora's options and a photo or log. One problem per issue.

To agree with an existing issue, add a 👍 reaction instead of a "me too" comment.

## Tried it on your screen?

Open a "Tried it on my screen" issue, good or bad. It feeds the "Tried on" list in the README.

## Ideas

Open an "Idea" issue: what you would like and what it is for. A new screen or a change to Tessera itself belongs on
[Tessera's wishlist](https://tessera-maxgramser.on-forge.com/wishlist).

## Pull requests

Bug fixes, translations and small improvements are welcome. For anything larger, open an issue first so we agree on
the way before you write it.

1. Clone [tessera-plugins](https://github.com/MaxGramser/tessera-plugins) beside this repository and read its
   `AGENTS.md`: it holds the rules every Tessera plugin follows.
2. Make one change per pull request, against `main`.
3. Run the checks (see "Tests" in the README):
   - `uv run tests/run.py` (unit, check and bench), or at least `python3 ../tessera-plugins/tools/check.py .`, which
     must print `ora: ok`;
   - `python3 ../tessera-plugins/tools/preview.py . --strict` after any change to the C++, the manifest or
     `preview.yaml`, until it ends without a warning.
4. Put new or changed texts in `translations/en.json`. No need to speak Greek: add them to `translations/el.json` if
   you can, otherwise leave it and the Greek is added before the merge.
5. Say what changed for users in the README and `CHANGELOG.md`. Leave the version in `tessera-plugin.yaml` alone: it is
   raised when a release is made.
6. Say which board you tried it on, and whether on a real screen or only on the bench.

GitHub runs the unit tests, Tessera's check, a secret scan and the firmware builds on every pull request; they must
pass before it is merged. By sending a pull request you agree that your change is released under the MIT license of
this repository.
