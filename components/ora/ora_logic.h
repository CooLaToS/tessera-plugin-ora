#pragma once
// Ora's logic without LVGL or ESPHome: the choices of the options, the time and date as text, where every part of the
// face stands, and how much of the line under the clock fits. ora.cpp draws from it; tests/test_logic.cpp checks it on
// a computer (make -C tests).
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

namespace ora_logic {

// ---- Colours ----
// The fifteen colours a person picks from (oak, walnut and mint: wood tones and a pale aqua), and the words that
// stand for something else: "same" (as the hours), "soft" (the soft white of the date), "off" and "dim" (the night
// colour), "graphite" and "tint" (the boxes).
enum class Colour {
  GOLD, CHAMPAGNE, AMBER, ORANGE, RED, PINK, PURPLE, BLUE, TEAL, GREEN, OAK, WALNUT, MINT, WHITE, GREY,
  SAME, SOFT, OFF, DIM, GRAPHITE, TINT, UNKNOWN
};

inline Colour colour_of(const char *name) {
  struct Name { const char *word; Colour colour; };
  static const Name NAMES[] = {
      {"gold", Colour::GOLD},     {"champagne", Colour::CHAMPAGNE}, {"amber", Colour::AMBER},
      {"orange", Colour::ORANGE}, {"red", Colour::RED},             {"pink", Colour::PINK},
      {"purple", Colour::PURPLE}, {"blue", Colour::BLUE},           {"teal", Colour::TEAL},
      {"green", Colour::GREEN},   {"oak", Colour::OAK},             {"walnut", Colour::WALNUT},
      {"mint", Colour::MINT},     {"white", Colour::WHITE},         {"grey", Colour::GREY},
      {"same", Colour::SAME},     {"soft", Colour::SOFT},           {"off", Colour::OFF},
      {"dim", Colour::DIM},       {"graphite", Colour::GRAPHITE},   {"tint", Colour::TINT},
  };
  if (!name) return Colour::UNKNOWN;
  for (const Name &n : NAMES)
    if (std::strcmp(name, n.word) == 0) return n.colour;
  return Colour::UNKNOWN;
}

// A colour a person can see as itself (not a word for another one).
inline bool is_plain(Colour c) { return c <= Colour::GREY; }

// What the hours and the minutes are painted in, and whether they are dimmed, by day or by night.
struct Ink {
  Colour hours, minutes;
  bool dimmed;
};

// `digits` and `minutes` as the person chose them (minutes may be SAME), `night_choice` OFF, DIM or a colour, and
// whether the night levels hold now. Anything unknown falls back to gold, the face's own colour.
inline Ink ink_for(Colour digits, Colour minutes, Colour night_choice, bool night) {
  if (!is_plain(digits)) digits = Colour::GOLD;
  if (!is_plain(minutes)) minutes = digits;
  if (night && night_choice == Colour::DIM) return {digits, minutes, true};
  if (night && is_plain(night_choice)) return {night_choice, night_choice, false};
  return {digits, minutes, false};
}

// ---- The time ----
// ---- Contrast ----
// Digits in a colour on a box of another (red on blue) can be too faint to read. The box then goes darker, step by step,
// until the digits stand out from it by at least MIN_CONTRAST, WCAG's ratio for large text. Every colour of the face
// reaches it on black (the darkest, red, has 3.5 there), so the guard always ends readable. The bench warns below 2.5.
struct Rgb { int r, g, b; };

// As LVGL's lv_color_mix: `mix` of 255 is all `a`, 0 all `b`.
inline Rgb mix(Rgb a, Rgb b, int mix) {
  auto one = [&](int x, int y) { return (x * mix + y * (255 - mix) + 127) / 255; };
  return {one(a.r, b.r), one(a.g, b.g), one(a.b, b.b)};
}

// WCAG's relative luminance and contrast ratio.
inline double luminance(Rgb c) {
  auto lin = [](int v) {
    const double s = v / 255.0;
    return s <= 0.03928 ? s / 12.92 : std::pow((s + 0.055) / 1.055, 2.4);
  };
  return 0.2126 * lin(c.r) + 0.7152 * lin(c.g) + 0.0722 * lin(c.b);
}
inline double contrast(Rgb a, Rgb b) {
  const double la = luminance(a), lb = luminance(b);
  return (std::max(la, lb) + 0.05) / (std::min(la, lb) + 0.05);
}

constexpr double MIN_CONTRAST = 3.0;

// The box's fill, darkened toward `black` until both inks reach MIN_CONTRAST on it (or it is nearly black).
inline Rgb readable_fill(Rgb fill, Rgb ink1, Rgb ink2, Rgb black) {
  for (int step = 0; step < 8; ++step) {
    if (contrast(fill, ink1) >= MIN_CONTRAST && contrast(fill, ink2) >= MIN_CONTRAST) break;
    fill = mix(fill, black, 170);
  }
  return fill;
}

// The time as the screen writes it ("14:05", "2:05 PM", "--") cut into its hours, its minutes and what follows.
struct Clock {
  std::string hours, minutes, suffix;
  bool ok;
};

inline Clock split_clock(const std::string &text, bool leading_zero) {
  const size_t colon = text.find(':');
  if (colon == std::string::npos || colon == 0 || colon + 3 > text.size()) return {"--", "--", "", false};
  if (!std::isdigit(static_cast<unsigned char>(text[colon + 1])) || !std::isdigit(static_cast<unsigned char>(text[colon + 2])))
    return {"--", "--", "", false};
  std::string hours = text.substr(0, colon);
  if (hours.size() == 1 && leading_zero) hours = "0" + hours;
  if (hours.size() == 2 && hours[0] == '0' && !leading_zero) hours = hours.substr(1);
  const std::string minutes = text.substr(colon + 1, 2);
  const size_t space = text.find(' ', colon);
  const std::string suffix = space == std::string::npos ? "" : text.substr(space + 1);
  return {hours, minutes, suffix, true};
}

// ---- The date ----
// "Fri 20 Nov" (the screen's own), "20/11", "20 November", "Friday 20 November", "Friday", or nothing.
enum class DateFormat { SHORT, NUMERIC, LONG, WEEKDAY_LONG, WEEKDAY, NONE };

inline DateFormat date_format_of(const char *name) {
  struct Name { const char *word; DateFormat format; };
  static const Name NAMES[] = {{"numeric", DateFormat::NUMERIC}, {"long", DateFormat::LONG},
                               {"weekday_long", DateFormat::WEEKDAY_LONG}, {"weekday", DateFormat::WEEKDAY},
                               {"none", DateFormat::NONE}};
  if (name)
    for (const Name &n : NAMES)
      if (std::strcmp(name, n.word) == 0) return n.format;
  return DateFormat::SHORT;
}

// The translation key of the pattern a format is written with; "" for the screen's own (SHORT) and for none.
inline const char *date_pattern_key(DateFormat f) {
  switch (f) {
    case DateFormat::NUMERIC: return "date_numeric";
    case DateFormat::LONG: return "date_long";
    case DateFormat::WEEKDAY_LONG: return "date_weekday_long";
    case DateFormat::WEEKDAY: return "date_weekday";
    default: return "";
  }
}

// Two digits: 7 is "07".
inline std::string two(int n) {
  if (n < 0) n = 0;
  n %= 100;
  return std::string(1, static_cast<char>('0' + n / 10)) + static_cast<char>('0' + n % 10);
}

// "{day}", "{month}" and "{weekday}" of a pattern of the screen's language replaced, wherever the language put them:
// "{day}/{month}" with "20" and "11" is "20/11".
inline std::string fill_date(const std::string &pattern, const std::string &day, const std::string &month,
                             const std::string &weekday = "") {
  std::string out = pattern;
  const std::pair<const char *, const std::string *> keys[] = {{"{day}", &day}, {"{month}", &month}, {"{weekday}", &weekday}};
  for (const auto &[key, value] : keys) {
    const size_t at = out.find(key);
    if (at != std::string::npos) out.replace(at, std::strlen(key), *value);
  }
  return out;
}

// The translation key of a weekday (1 Sunday to 7 Saturday) and of a month (1 to 12); "" outside them.
inline std::string weekday_key(int weekday) { return weekday >= 1 && weekday <= 7 ? "day_" + std::to_string(weekday) : ""; }
inline std::string month_key(int month) { return month >= 1 && month <= 12 ? "month_" + std::to_string(month) : ""; }

// ---- The line under the clock: the date, the clock's row of entities, the temperature ----
// The row of entities is the one the person gives the Clock step in Tessera (a weather entity shows its condition's
// icon): off, their icons only, or their icons and words.
enum class Row { OFF, ICONS, FULL };

inline Row row_of(const char *name) {
  if (name && std::strcmp(name, "off") == 0) return Row::OFF;
  if (name && std::strcmp(name, "full") == 0) return Row::FULL;
  return Row::ICONS;
}

// How many of the row's items fit beside the date and the temperature in `room` pixels, and how wide the line is then.
// Parts stand `between` apart; a part of width 0 is not there. The date and the temperature always stay; the items go
// from the last when the line is too wide.
struct RowPlan {
  int items;
  int width;
};

inline RowPlan plan_row(int room, int date_w, const std::vector<int> &items, int temp_w, int between) {
  for (int n = static_cast<int>(items.size()); n >= 0; --n) {
    int width = 0, parts = 0;
    auto add = [&](int w) {
      if (w <= 0) return;
      width += w;
      ++parts;
    };
    add(date_w);
    for (int i = 0; i < n; ++i) add(items[i]);
    add(temp_w);
    if (parts > 1) width += between * (parts - 1);
    if (width <= room || n == 0) return {n, width};
  }
  return {0, 0};
}

// ---- Where everything stands ----
enum class Size { NORMAL, LARGE, STACKED };
enum class Style { CARDS, OUTLINE, PLAIN };

inline Size size_of(const char *name) {
  if (name && std::strcmp(name, "normal") == 0) return Size::NORMAL;
  if (name && std::strcmp(name, "stacked") == 0) return Size::STACKED;
  return Size::LARGE;
}
inline Style style_of(const char *name) {
  if (name && std::strcmp(name, "outline") == 0) return Style::OUTLINE;
  if (name && std::strcmp(name, "plain") == 0) return Style::PLAIN;
  return Style::CARDS;
}

// What the glass and its fonts measure, in pixels.
struct Digits {
  int w;  // "88" in the font
  int h;  // its line height
};
constexpr int MAX_FONTS = 3;  // the screen's digit fonts, largest first; the last is the one Normal takes

struct Metrics {
  int width, height;          // the glass
  Digits fonts[MAX_FONTS];    // the digit fonts, largest first (two may measure the same: a board without a larger one)
  int font_count;
  int row_h;                  // the line under the clock (date, row, temperature); 0 when it shows nothing
  int suffix_w;               // AM or PM beside the minutes; 0 on a 24-hour screen
  int pad, gap, margin, space;
};

struct Box { int x, y, w, h; };

struct Layout {
  int font;             // which of Metrics::fonts the digits take
  int scale;            // the font drawn at scale/256 of its size; 256 (no transform) wherever a font fits
  int gap;              // between the boxes, and above the line under them
  int grow;             // how large the digits are against the last font (Normal's) at its size, 256 is the same
  Box hours, minutes;   // the two boxes
  int colon_x, colon_y; // the middle of the dot (or the colon's two dots)
  bool colon_across;    // the colon's two dots side by side (stacked) instead of one above the other
  int suffix_x, suffix_y;
  int row_y;            // top of the line under the clock
};

inline int clamp(int v, int lo, int hi) { return std::max(lo, std::min(v, hi)); }

// The digits grow at most three times their font's size (256 is 1x): at four the 10-inch screens showed their pixels.
// Only when even the largest font is too small for the room, which the page's own digits hardly ever are.
constexpr int MAX_SCALE = 768;
// A font at its own size is taken when it fills at least four fifths of the room (the room is at most 5/4 of it).
constexpr int FILLS = 320;

struct Pick { int font, scale; };

// How far a font fits a room of avail_w x avail_h for "88", 256 is exactly.
inline int fit_of(const Digits &f, int avail_w, int avail_h) {
  return std::min(std::max(avail_w, 1) * 256 / std::max(1, f.w), std::max(avail_h, 1) * 256 / std::max(1, f.h));
}

// The font, from how far each fits (fit[], largest font first): the largest that fits at its own size and fills the
// room well (drawn without a transform: sharp, and no layer of its own on every redraw); else the smallest that is too
// large, drawn smaller (a font made smaller stays sharp); else, when every font is too small, the largest, larger.
inline Pick pick_fit(const int *fit, int n) {
  n = clamp(n, 1, MAX_FONTS);
  for (int i = 0; i < n; ++i)
    if (fit[i] >= 256 && fit[i] <= FILLS) return {i, 256};
  for (int i = n - 1; i >= 0; --i)
    if (fit[i] < 256) return {i, std::max(fit[i], 1)};
  return {0, std::min(fit[0], MAX_SCALE)};
}

// The same for one room that does not depend on the font.
inline Pick pick(const Digits *fonts, int n, int avail_w, int avail_h) {
  n = clamp(n, 1, MAX_FONTS);
  int fit[MAX_FONTS];
  for (int i = 0; i < n; ++i) fit[i] = fit_of(fonts[i], avail_w, avail_h);
  return pick_fit(fit, n);
}

// The gap between the two boxes (and around the line under them) grows with the digits, as on a flip clock: an eighth
// of their height, the screen's own gap at least. Room for the seconds dot, and for a plain colon clear of the digits.
inline int gap_for(const Metrics &m, int digits_h) { return std::max(m.gap, digits_h / 8); }

inline Layout layout(const Metrics &m, Size size) {
  Layout l{};
  const int n = clamp(m.font_count, 1, MAX_FONTS);
  const int suffix = m.suffix_w ? m.suffix_w + m.space : 0;

  // Each font in the room it leaves itself: its gap is part of what it takes.
  Pick p{n - 1, 256};
  if (size != Size::NORMAL) {
    int fit[MAX_FONTS];
    for (int i = 0; i < n; ++i) {
      const int gap = gap_for(m, m.fonts[i].h), row = m.row_h ? m.row_h + gap : 0;
      if (size == Size::LARGE)
        fit[i] = fit_of(m.fonts[i], (m.width - 2 * m.margin - gap - suffix) / 2 - 2 * m.pad,
                        m.height - 2 * m.margin - row - 2 * m.pad);
      else
        fit[i] = fit_of(m.fonts[i], m.width - 2 * m.margin - suffix - 2 * m.pad,
                        (m.height - 2 * m.margin - row - gap) / 2 - 2 * m.pad);
    }
    p = pick_fit(fit, n);
  }
  l.font = p.font;
  l.scale = p.scale;
  const int dw = std::max(1, m.fonts[p.font].w), dh = std::max(1, m.fonts[p.font].h);
  l.grow = std::max(1, dh * l.scale / std::max(1, m.fonts[n - 1].h));
  // The gap the font was measured with (Normal: the screen's own); a font drawn smaller keeps it, which only adds room.
  l.gap = size == Size::NORMAL ? m.gap : gap_for(m, dh);
  const int gap = l.gap, row = m.row_h ? m.row_h + gap : 0;
  const int cw = dw * l.scale / 256 + 2 * m.pad, ch = dh * l.scale / 256 + 2 * m.pad;
  const bool stacked = size == Size::STACKED;
  const int block_w = stacked ? cw : 2 * cw + gap;
  const int block_h = stacked ? 2 * ch + gap : ch;

  const int y = (m.height - block_h - row) / 2;
  const int left = (m.width - block_w - suffix) / 2;
  l.hours = {left, y, cw, ch};
  l.minutes = stacked ? Box{left, y + ch + gap, cw, ch} : Box{left + cw + gap, y, cw, ch};
  l.colon_across = stacked;
  l.colon_x = stacked ? left + cw / 2 : left + cw + gap / 2;
  l.colon_y = stacked ? y + ch + gap / 2 : y + ch / 2;
  l.suffix_x = l.minutes.x + l.minutes.w + m.space;
  l.suffix_y = l.minutes.y + l.minutes.h - m.pad;
  l.row_y = y + block_h + (m.row_h ? gap : 0);
  return l;
}

// The size of the seconds dot: it grows with the digits (grow, Layout), and stays within the gap: between two boxes
// with room around it, as a colon of two dots without boxes clear of the digits beside it.
inline int dot_size(int base, int grow, int gap, bool boxes) {
  int dot = base * grow / 256;
  dot = std::min(dot, boxes ? gap * 6 / 10 : gap * 9 / 10);
  return std::max(dot, 2);
}

}  // namespace ora_logic
