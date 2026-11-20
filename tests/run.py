#!/usr/bin/env python3
# /// script
# requires-python = ">=3.11"
# dependencies = ["pyyaml"]
# ///
"""Ora's tests, in the order they fail fastest:

  unit    the face's logic (components/ora/ora_logic.h) compiled for this computer with the address and
          undefined-behaviour sanitizers, and run (tests/test_logic.cpp)
  check   Tessera's own check of the plugin (tools/check.py of tessera-plugins)
  bench   the bench of tessera-plugins in strict mode: the real firmware with Ora, built for this computer, drawing the
          showcase (preview.yaml) on four kinds of screen, then the layout combinations (tests/combos.py) on the
          Guition 4-inch. Any finding fails.

    uv run tests/run.py              # all three
    uv run tests/run.py unit         # one of them (or several: unit check)

The check and the bench need a clone of tessera-plugins beside this repository (or TESSERA_PLUGINS=<path>), its bench
set up once with `python3 <clone>/tools/preview.py --setup`. ORA_FULL=1 renders every combination (486, about half an
hour) instead of 54.
"""
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent
TP = Path(os.environ.get('TESSERA_PLUGINS', ROOT.parent / 'tessera-plugins')).resolve()


def run(*command, **kwargs):
    print('$', ' '.join(str(c) for c in command), flush=True)
    done = subprocess.run([str(c) for c in command], **kwargs)
    if done.returncode:
        sys.exit(f'FAILED ({done.returncode}): {command[0]}')
    return done


def unit():
    cxx = os.environ.get('CXX') or shutil.which('c++') or shutil.which('clang++') or shutil.which('g++')
    if not cxx:
        sys.exit('unit: no C++ compiler (set CXX)')
    with tempfile.TemporaryDirectory() as tmp:
        binary = Path(tmp) / 'test_logic'
        flags = ['-std=c++17', '-Wall', '-Wextra', '-Werror', '-O1']
        if os.environ.get('ORA_NO_SANITIZE') is None:
            flags.append('-fsanitize=address,undefined')
        run(cxx, *flags, f'-I{ROOT / "components" / "ora"}', '-o', binary, HERE / 'test_logic.cpp')
        run(binary)


def need_tessera_plugins():
    if not (TP / 'tools' / 'check.py').is_file():
        sys.exit(f'no tessera-plugins at {TP}: clone https://github.com/MaxGramser/tessera-plugins there '
                 'or set TESSERA_PLUGINS')


def check():
    need_tessera_plugins()
    run(sys.executable, TP / 'tools' / 'check.py', ROOT)


def bench():
    need_tessera_plugins()
    preview = TP / 'tools' / 'preview.py'
    print('== bench: the showcase on four kinds of screen', flush=True)
    run(sys.executable, preview, ROOT, '--strict')
    print('== bench: the layout combinations on the Guition 4-inch', flush=True)
    with tempfile.TemporaryDirectory() as tmp:
        # The plugin as it is, with the combinations as its preview.yaml (the bench reads the plugin's own folder).
        copy = Path(tmp) / 'ora'
        shutil.copytree(ROOT, copy, ignore=shutil.ignore_patterns('.git', '.preview*', '__pycache__'))
        combos = run(sys.executable, HERE / 'combos.py', capture_output=True, text=True).stdout
        (copy / 'preview.yaml').write_text(combos)
        run(sys.executable, preview, copy, '--board', 'guition', '--strict', '--out', ROOT / '.preview-combos')


STEPS = {'unit': unit, 'check': check, 'bench': bench}


def main(argv):
    chosen = argv or list(STEPS)
    unknown = [a for a in chosen if a not in STEPS]
    if unknown:
        sys.exit(f'unknown step {unknown}: choose from {list(STEPS)}')
    for name in chosen:
        print(f'== {name}', flush=True)
        STEPS[name]()
    print('== all green:', ', '.join(chosen))


if __name__ == '__main__':
    main(sys.argv[1:])
