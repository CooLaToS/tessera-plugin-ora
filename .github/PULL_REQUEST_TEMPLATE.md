## What this changes

What someone will see on the screen, or be able to do, after this change. Link the issue it fixes (`Fixes #...`).

## How I checked it

Name the board you tried it on, and say whether it ran on a real screen or only on the bench.

- [ ] `uv run tests/run.py` passes (unit, check and bench), or at least `python3 ../tessera-plugins/tools/check.py .` prints `ora: ok`.
- [ ] `python3 ../tessera-plugins/tools/preview.py . --strict` ends without a warning (when the C++, the manifest or `preview.yaml` changed).
- [ ] New or changed texts are in `translations/en.json` (Greek is optional: the maintainer adds it to `translations/el.json`).
- [ ] One change per pull request; the README and `CHANGELOG.md` say what changed for users.
- [ ] I did not raise the version in `tessera-plugin.yaml`: the maintainer does that when releasing.
