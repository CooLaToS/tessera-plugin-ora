#!/usr/bin/env python3
"""Ora's layout combinations as a preview.yaml for the bench. By default every style x size x date (54), with the row,
the boxes, the colours, night and the toggles walked through alongside so each of their values shows up several times;
ORA_FULL=1 takes style x size x date x row x boxes (486, about half an hour on the bench). Prints YAML."""
import itertools
import json
import os

STYLES = ['cards', 'outline', 'plain']
SIZES = ['large', 'stacked', 'normal']
DATES = ['short', 'numeric', 'long', 'weekday_long', 'weekday', 'none']
ROWS = ['icons', 'full', 'off']
BOXES = ['graphite', 'tint', 'blue']
COLOURS = ['gold', 'champagne', 'amber', 'orange', 'red', 'pink', 'purple', 'blue', 'teal', 'green', 'oak', 'walnut',
           'mint', 'white', 'grey']
# A rainy weather icon and a humidity reading, as the clock's row sends them (codepoints of Tessera's set).
ITEMS = [{'icon': 0xF0597, 'text': 'Rainy'}, {'icon': 0xF058E, 'text': '54%'}]

savers = []
if os.environ.get('ORA_FULL'):
    combos = list(itertools.product(STYLES, SIZES, DATES, ROWS, BOXES))
else:
    combos = [(st, si, d, ROWS[n % 3], BOXES[(n // 3) % 3])
              for n, (st, si, d) in enumerate(itertools.product(STYLES, SIZES, DATES))]
for n, (style, size, date, row, boxes) in enumerate(combos):
    options = {'style': style, 'size': size, 'date_format': date, 'row': row, 'boxes': boxes,
               'digits': COLOURS[n % 15], 'minutes': ['same', COLOURS[(n + 5) % 15]][n % 2],
               'date': ['soft', 'same', COLOURS[(n + 3) % 15]][n % 3],
               'temperature': ['same', 'soft', COLOURS[(n + 7) % 15]][n % 3],
               'night': ['off', 'dim', 'red'][n % 3],
               'leading_zero': n % 2 == 0, 'seconds': n % 4 != 3}
    savers.append({'saver': 'ora', 'name': f'{style} {size} {date} {row} {boxes}', 'options': options,
                   'data': {'temperature': '' if n % 7 == 6 else '21°', 'items': ITEMS[: n % 3]}})
print('# Made by tests/combos.py: every layout combination, for tests/e2e.sh.')
print(json.dumps({'savers': savers}, ensure_ascii=False, indent=1))  # JSON is YAML
