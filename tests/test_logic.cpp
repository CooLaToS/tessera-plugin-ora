// Unit tests of Ora's logic: the choices, the time and the date as text, and where every part stands on every kind of
// screen. No LVGL, no ESPHome: make -C tests.
#include "ora_logic.h"

#include <cstdio>
#include <string>
#include <vector>

using namespace ora_logic;

static int failures = 0, checks = 0;

#define CHECK(cond)                                                                \
  do {                                                                             \
    ++checks;                                                                      \
    if (!(cond)) {                                                                 \
      ++failures;                                                                  \
      std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);                  \
    }                                                                              \
  } while (0)

#define CHECK_EQ(a, b)                                                             \
  do {                                                                             \
    ++checks;                                                                      \
    if (!((a) == (b))) {                                                           \
      ++failures;                                                                  \
      std::printf("FAIL %s:%d: %s == %s\n", __FILE__, __LINE__, #a, #b);           \
    }                                                                              \
  } while (0)

static void colours() {
  const char *names[] = {"gold", "champagne", "amber", "orange", "red", "pink", "purple", "blue",
                         "teal", "green", "oak", "walnut", "mint", "white", "grey"};
  CHECK(sizeof(names) / sizeof(names[0]) == 15);
  for (const char *n : names) CHECK(is_plain(colour_of(n)));
  CHECK(colour_of("gold") == Colour::GOLD);
  CHECK(colour_of("grey") == Colour::GREY);
  CHECK(colour_of("same") == Colour::SAME);
  CHECK(colour_of("tint") == Colour::TINT);
  CHECK(!is_plain(colour_of("same")));
  CHECK(!is_plain(colour_of("soft")));
  CHECK(colour_of("Gold") == Colour::UNKNOWN);  // the values are lowercase, as the manifest has them
  CHECK(colour_of("") == Colour::UNKNOWN);
  CHECK(colour_of(nullptr) == Colour::UNKNOWN);
}

static void ink() {
  // By day: the person's colours; minutes SAME take the hours'.
  Ink i = ink_for(Colour::GOLD, Colour::SAME, Colour::RED, false);
  CHECK(i.hours == Colour::GOLD && i.minutes == Colour::GOLD && !i.dimmed);
  i = ink_for(Colour::GOLD, Colour::CHAMPAGNE, Colour::OFF, false);
  CHECK(i.hours == Colour::GOLD && i.minutes == Colour::CHAMPAGNE);
  // At night: off keeps the day's, dim keeps them and dims, a colour paints both.
  i = ink_for(Colour::GOLD, Colour::CHAMPAGNE, Colour::OFF, true);
  CHECK(i.hours == Colour::GOLD && i.minutes == Colour::CHAMPAGNE && !i.dimmed);
  i = ink_for(Colour::GOLD, Colour::CHAMPAGNE, Colour::DIM, true);
  CHECK(i.hours == Colour::GOLD && i.minutes == Colour::CHAMPAGNE && i.dimmed);
  i = ink_for(Colour::GOLD, Colour::CHAMPAGNE, Colour::RED, true);
  CHECK(i.hours == Colour::RED && i.minutes == Colour::RED && !i.dimmed);
  // Dim by day does nothing.
  CHECK(!ink_for(Colour::GOLD, Colour::SAME, Colour::DIM, false).dimmed);
  // Nonsense falls back to gold.
  i = ink_for(Colour::UNKNOWN, Colour::UNKNOWN, Colour::UNKNOWN, true);
  CHECK(i.hours == Colour::GOLD && i.minutes == Colour::GOLD && !i.dimmed);
  CHECK(ink_for(Colour::SOFT, Colour::SAME, Colour::OFF, false).hours == Colour::GOLD);
}

static void clock_text() {
  Clock c = split_clock("14:05", true);
  CHECK(c.ok && c.hours == "14" && c.minutes == "05" && c.suffix.empty());
  c = split_clock("09:05", true);
  CHECK_EQ(c.hours, std::string("09"));
  c = split_clock("09:05", false);
  CHECK_EQ(c.hours, std::string("9"));
  c = split_clock("00:00", false);  // midnight keeps one zero
  CHECK(c.hours == "0" && c.minutes == "00");
  c = split_clock("10:30", false);  // a ten is no leading zero
  CHECK_EQ(c.hours, std::string("10"));
  c = split_clock("2:05 PM", true);
  CHECK(c.hours == "02" && c.minutes == "05" && c.suffix == "PM");
  c = split_clock("2:05 PM", false);
  CHECK(c.hours == "2" && c.suffix == "PM");
  c = split_clock("12:00 AM", false);
  CHECK(c.hours == "12" && c.minutes == "00" && c.suffix == "AM");
  // What is no time shows dashes.
  for (const char *bad : {"", "--", "14", ":05", "14:5", "abc"}) {
    c = split_clock(bad, true);
    CHECK(!c.ok && c.hours == "--" && c.minutes == "--");
  }
}

static void dates() {
  CHECK(date_format_of("short") == DateFormat::SHORT);
  CHECK(date_format_of("numeric") == DateFormat::NUMERIC);
  CHECK(date_format_of("long") == DateFormat::LONG);
  CHECK(date_format_of("none") == DateFormat::NONE);
  CHECK(date_format_of("weekday_long") == DateFormat::WEEKDAY_LONG);
  CHECK(date_format_of("weekday") == DateFormat::WEEKDAY);
  CHECK(std::string(date_pattern_key(DateFormat::NUMERIC)) == "date_numeric");
  CHECK(std::string(date_pattern_key(DateFormat::WEEKDAY_LONG)) == "date_weekday_long");
  CHECK(std::string(date_pattern_key(DateFormat::SHORT)).empty());
  CHECK(std::string(date_pattern_key(DateFormat::NONE)).empty());
  CHECK_EQ(fill_date("{weekday} {day} {month}", "20", "November", "Friday"), std::string("Friday 20 November"));
  CHECK_EQ(fill_date("{weekday}", "20", "November", "Friday"), std::string("Friday"));
  CHECK(date_format_of("x") == DateFormat::SHORT);
  CHECK(date_format_of(nullptr) == DateFormat::SHORT);
  CHECK_EQ(two(7), std::string("07"));
  CHECK_EQ(two(20), std::string("20"));
  CHECK_EQ(two(0), std::string("00"));
  CHECK_EQ(two(-3), std::string("00"));
  CHECK_EQ(fill_date("{day}/{month}", two(5), two(3)), std::string("05/03"));
  CHECK_EQ(fill_date("{day} {month}", "20", "November"), std::string("20 November"));
  CHECK_EQ(fill_date("{month}/{day}", "20", "11"), std::string("11/20"));  // a language that writes the month first
  CHECK_EQ(fill_date("no keys", "1", "2"), std::string("no keys"));
  CHECK_EQ(weekday_key(1), std::string("day_1"));
  CHECK_EQ(weekday_key(7), std::string("day_7"));
  CHECK_EQ(weekday_key(0), std::string(""));
  CHECK_EQ(weekday_key(8), std::string(""));
  CHECK_EQ(month_key(12), std::string("month_12"));
  CHECK_EQ(month_key(13), std::string(""));
}

static void choices() {
  CHECK(size_of("large") == Size::LARGE);
  CHECK(size_of("stacked") == Size::STACKED);
  CHECK(size_of("normal") == Size::NORMAL);
  CHECK(size_of(nullptr) == Size::LARGE);
  CHECK(style_of("outline") == Style::OUTLINE);
  CHECK(style_of("plain") == Style::PLAIN);
  CHECK(style_of("cards") == Style::CARDS);
  CHECK(style_of("?") == Style::CARDS);
  CHECK(row_of("icons") == Row::ICONS);
  CHECK(row_of("full") == Row::FULL);
  CHECK(row_of("off") == Row::OFF);
  CHECK(row_of(nullptr) == Row::ICONS);
}

static bool inside(const Box &b, int w, int h) { return b.x >= 0 && b.y >= 0 && b.x + b.w <= w && b.y + b.h <= h; }
static bool apart(const Box &a, const Box &b) {
  return a.x + a.w <= b.x || b.x + b.w <= a.x || a.y + a.h <= b.y || b.y + b.h <= a.y;
}

// Screens of Tessera's catalog with fonts of their look (compact on the CYD, standard elsewhere): glass, the largest
// digits' "88" and line, and the line under the clock. The numbers are close to what the bench measures; the layout
// must hold for any of them.
struct Screen { const char *name; int w, h, digits_w, digits_h, row_h, mm; };
static const Screen SCREENS[] = {
    {"cyd 320x240", 320, 240, 48, 46, 22, 5},
    {"cyd portrait 240x320", 240, 320, 48, 46, 22, 5},
    {"guition 480x480", 480, 480, 70, 75, 30, 7},
    {"waveshare43 portrait 480x800", 480, 800, 70, 75, 30, 6},
    {"waveshare43 800x480", 800, 480, 70, 75, 30, 6},
    {"guition 7 1024x600", 1024, 600, 80, 86, 34, 6},
    {"guition 10 1280x800", 1280, 800, 90, 96, 38, 6},
    {"tab5 720x1280", 720, 1280, 90, 96, 38, 11},
};

static void layouts() {
  for (const Screen &s : SCREENS) {
    for (Size size : {Size::NORMAL, Size::LARGE, Size::STACKED}) {
      for (int row_h : {0, s.row_h}) {
        for (int suffix : {0, 30}) {
          Metrics m{s.w, s.h, s.digits_w, s.digits_h, row_h, suffix, 3 * s.mm, 4 * s.mm, 4 * s.mm, 6};
          const Layout l = layout(m, size);
          const std::string what = std::string(s.name) + " size " + std::to_string(static_cast<int>(size)) + " row " +
                                   std::to_string(row_h) + " suffix " + std::to_string(suffix);
          const int before = failures;
          // The scale: the font's own size at least, never more than the size allows.
          CHECK(l.scale >= 256);
          CHECK(l.scale <= (size == Size::NORMAL ? 256 : MAX_SCALE));
          // The cards: on the glass, apart, the same size, in the order of reading.
          CHECK(inside(l.hours, s.w, s.h));
          CHECK(inside(l.minutes, s.w, s.h));
          CHECK(apart(l.hours, l.minutes));
          CHECK(l.hours.w == l.minutes.w && l.hours.h == l.minutes.h);
          if (size == Size::STACKED) CHECK(l.minutes.y > l.hours.y && l.minutes.x == l.hours.x);
          else CHECK(l.minutes.x > l.hours.x && l.minutes.y == l.hours.y);
          // The date's line under the cards, on the glass.
          const int block_bottom = std::max(l.hours.y + l.hours.h, l.minutes.y + l.minutes.h);
          CHECK(l.row_y >= block_bottom);
          CHECK(l.row_y + m.row_h <= s.h);
          // Upright in the middle of the glass, within a pixel or two.
          const int bottom = row_h ? l.row_y + row_h : block_bottom;
          CHECK(std::abs(l.hours.y - (s.h - bottom)) <= 2);
          // AM or PM beside the minutes, on the glass.
          if (suffix) CHECK(l.suffix_x + suffix <= s.w);
          // The colon between the cards.
          if (size == Size::STACKED) CHECK(l.colon_y > l.hours.y + l.hours.h && l.colon_y < l.minutes.y);
          else CHECK(l.colon_x > l.hours.x + l.hours.w && l.colon_x < l.minutes.x);
          // Centred across the glass, within a pixel or two.
          const int right = std::max(l.hours.x + l.hours.w, l.minutes.x + l.minutes.w) + (suffix ? suffix + 6 : 0);
          CHECK(std::abs(l.hours.x - (s.w - right)) <= 2);
          if (failures != before) std::printf("     in %s\n", what.c_str());
        }
      }
    }
  }
  // Large is larger than Normal on every screen, and fills a good part of the glass's width side by side.
  for (const Screen &s : SCREENS) {
    Metrics m{s.w, s.h, s.digits_w, s.digits_h, s.row_h, 0, 3 * s.mm, 4 * s.mm, 4 * s.mm, 6};
    const Layout normal = layout(m, Size::NORMAL), large = layout(m, Size::LARGE);
    CHECK(large.hours.w >= normal.hours.w);
    CHECK(large.scale > 256);
  }
  // Two digits side by side are about twice as wide as tall: Large suits a square or wide glass, Stacked a tall one.
  Metrics square{480, 480, 70, 75, 30, 0, 18, 24, 24, 6};
  CHECK(layout(square, Size::LARGE).scale > layout(square, Size::STACKED).scale);
  Metrics tall{480, 800, 70, 75, 30, 0, 18, 24, 24, 6};
  CHECK(layout(tall, Size::STACKED).scale > layout(tall, Size::LARGE).scale);
  Metrics wide{1280, 800, 90, 96, 38, 0, 18, 24, 24, 6};
  CHECK(layout(wide, Size::LARGE).scale >= layout(wide, Size::STACKED).scale);
  // Fonts that do not measure (0) do not divide by zero.
  Metrics z{480, 480, 0, 0, 0, 0, 21, 28, 28, 6};
  CHECK(layout(z, Size::LARGE).scale >= 256);
}

static void rows() {
  // Everything fits: date, two items, temperature, three gaps between four parts.
  RowPlan p = plan_row(400, 100, {30, 40}, 50, 10);
  CHECK(p.items == 2 && p.width == 100 + 30 + 40 + 50 + 3 * 10);
  // Too wide: items go from the last, date and temperature stay.
  p = plan_row(220, 100, {30, 40}, 50, 10);
  CHECK(p.items == 1 && p.width == 100 + 30 + 50 + 2 * 10);
  p = plan_row(150, 100, {30, 40}, 50, 10);
  CHECK(p.items == 0 && p.width == 100 + 50 + 10);
  // Even date and temperature too wide: no items, the line as it is (the screen centres it).
  p = plan_row(50, 100, {30}, 50, 10);
  CHECK(p.items == 0 && p.width == 160);
  // Parts that are not there take no room and no gap.
  p = plan_row(400, 0, {30}, 0, 10);
  CHECK(p.items == 1 && p.width == 30);
  p = plan_row(400, 100, {0, 30}, 0, 10);  // an item without icon or words counts as empty
  CHECK(p.items == 2 && p.width == 100 + 30 + 10);
  p = plan_row(400, 0, {}, 0, 10);
  CHECK(p.items == 0 && p.width == 0);
  // Four items, the row's most.
  p = plan_row(1000, 100, {20, 20, 20, 20}, 40, 8);
  CHECK(p.items == 4 && p.width == 100 + 80 + 40 + 5 * 8);
}

static void contrasts() {
  const Rgb black{0, 0, 0}, white{255, 255, 255};
  // WCAG's own end points.
  CHECK(std::abs(contrast(black, white) - 21.0) < 0.01);
  CHECK(std::abs(contrast(white, white) - 1.0) < 0.001);
  CHECK(std::abs(contrast(black, white) - contrast(white, black)) < 1e-9);
  // As LVGL mixes.
  const Rgb m = mix(white, black, 255);
  CHECK(m.r == 255 && m.g == 255 && m.b == 255);
  CHECK(mix(white, black, 0).r == 0);
  CHECK(std::abs(mix(white, black, 128).r - 128) <= 1);
  // The colours of the face (the dark values of their roles) and the boxes they make.
  const Rgb red{0xB2, 0x3A, 0x26}, pink{0xB5, 0x4C, 0x9D}, blue{0x00, 0x9F, 0xE3}, gold{0xC9, 0x93, 0x2F},
      card{0x1A, 0x1A, 0x1A}, champagne = mix(white, gold, 115);
  // The wood tones and the mint, as ora.cpp mixes them from the theme.
  const Rgb oak = mix({0xFF, 0x98, 0x00}, {0x88, 0x88, 0x88}, 100), walnut = mix(oak, black, 170),
            mint = mix({0xDA, 0xDA, 0xDA}, {0x5B, 0xD0, 0xCC}, 170);
  CHECK(oak.r == 0xB7 && oak.g == 0x8E && oak.b == 0x53);
  CHECK(walnut.r == 0x7A && walnut.g == 0x5F && walnut.b == 0x37);
  // Every colour of the face reads on the black glass by itself (plain digits, the date, the row).
  for (const Rgb &c : {oak, walnut, mint, red, pink, gold, champagne}) CHECK(contrast(c, black) >= MIN_CONTRAST);
  const Rgb blue_box = mix(blue, black, 110);
  // The two the bench found too faint: red and pink on a blue box. Darkened, they read.
  CHECK(contrast(blue_box, red) < MIN_CONTRAST);
  const Rgb fixed = readable_fill(blue_box, red, red, black);
  CHECK(contrast(fixed, red) >= MIN_CONTRAST);
  const Rgb fixed2 = readable_fill(blue_box, pink, white, black);
  CHECK(contrast(fixed2, pink) >= MIN_CONTRAST && contrast(fixed2, white) >= MIN_CONTRAST);
  // What reads already stays as it is: gold and champagne on a tinted card, white on a blue box.
  const Rgb tint = mix(gold, card, 52);
  CHECK(readable_fill(tint, gold, champagne, black).r == tint.r);
  CHECK(readable_fill(blue_box, white, white, black).b == blue_box.b);
  // Every colour on every box (coloured, graphite, tinted) reads after the guard, both tones.
  const Rgb all[] = {gold, champagne, {0xFF, 0xC1, 0x07}, {0xFF, 0x98, 0x00}, red, pink, {0x92, 0x6B, 0xC7}, blue,
                     {0x5B, 0xD0, 0xCC}, {0x4C, 0xAF, 0x50}, oak, walnut, mint, white, {0x88, 0x88, 0x88}};
  for (const Rgb &box : all)
    for (const Rgb &a : all)
      for (const Rgb &b : all)
        for (const Rgb &start : {mix(box, black, 110), card, mix(a, card, 52)}) {
        const Rgb f = readable_fill(start, a, b, black);
        CHECK(contrast(f, a) >= MIN_CONTRAST);
        CHECK(contrast(f, b) >= MIN_CONTRAST);
        CHECK(contrast(f, a) >= 2.5 && contrast(f, b) >= 2.5);  // never what the bench calls too faint
      }
}

static void dots() {
  CHECK(dot_size(14, 256, 28, true) == 14);
  CHECK(dot_size(14, 768, 28, true) <= 28 * 6 / 10);  // between cards it stays in the gap
  CHECK(dot_size(14, 768, 28, false) == 42);           // a plain colon grows with the digits
  CHECK(dot_size(0, 256, 28, true) >= 2);
}

int main() {
  colours();
  ink();
  clock_text();
  dates();
  choices();
  layouts();
  rows();
  contrasts();
  dots();
  std::printf("%d checks, %d failed\n", checks, failures);
  return failures ? 1 : 0;
}
