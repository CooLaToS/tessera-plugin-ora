#include "ora.h"
#include "ora_logic.h"
#include "esphome/core/log.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace esphome::ora {

using tessera::Font;
namespace ui = tessera::ui;
namespace L = ora_logic;

// Ora: a face of the screensaver's clock. The hours and the minutes in two boxes (flip cards, outlines or nothing),
// as large as the glass allows, in colours the person picks: two tones, a colour of the boxes of their own, and a
// colour for the night. Under them one line: the date in one of six forms, the icons (and words) of the clock's row of
// entities (a weather entity shows its condition: sun, cloud, rain), and the outside temperature. The logic (choices,
// texts, places) is in ora_logic.h, tested on a computer; this file only draws.
//
// Colours are theme roles, as the plugin rules ask (never a colour as a number). A saver is drawn in the dark look,
// so each colour is the dark value of its role: gold #C9932F, amber #FFC107, orange #FF9800, red #B23A26,
// pink #B54C9D, purple #926BC7, blue #009FE3, teal #5BD0CC, green #4CAF50, white, grey #888888; champagne, oak,
// walnut and mint are mixes of those (#D1B37C, #B78E53, #7A5F37, #B0D7D5). The screen moves the whole saver a
// millimetre every minute, so it stands still itself.

static const char *const TAG = "ora";

namespace {

constexpr int MAX_ITEMS = 4;  // the clock's row holds at most four entities (Tessera's ITEMS_MAX)

lv_color_t black() { return ui::color(theme::CAMERA_PAGE); }

// Light wood: the orange of solar power softened with the bedside grey (#B78E53); walnut is it darker (#7A5F37).
lv_color_t oak() { return lv_color_mix(ui::color(theme::ENERGY_SOLAR), ui::color(theme::BEDSIDE), 100); }

// The colour a person picked, as the glass shows it; `same` for SAME (and anything that is no colour).
lv_color_t paint(L::Colour c, lv_color_t same) {
  switch (c) {
    case L::Colour::GOLD: return ui::color(theme::BAUHAUS_YELLOW);
    case L::Colour::CHAMPAGNE: return lv_color_mix(ui::color(theme::INK), ui::color(theme::BAUHAUS_YELLOW), 115);
    case L::Colour::AMBER: return ui::color(theme::MARK_AMBER);
    case L::Colour::ORANGE: return ui::color(theme::ENERGY_SOLAR);
    case L::Colour::RED: return ui::color(theme::BAUHAUS_RED);
    case L::Colour::PINK: return ui::color(theme::ENERGY_DEVICE_3);
    case L::Colour::PURPLE: return ui::color(theme::MARK_PURPLE);
    case L::Colour::BLUE: return ui::color(theme::MARK_BLUE);
    case L::Colour::TEAL: return ui::color(theme::ENERGY_DEVICE_4);
    case L::Colour::GREEN: return ui::color(theme::MARK_GREEN);
    case L::Colour::OAK: return oak();
    case L::Colour::WALNUT: return lv_color_mix(oak(), black(), 170);
    case L::Colour::MINT: return lv_color_mix(ui::color(theme::INK), ui::color(theme::ENERGY_DEVICE_4), 170);
    case L::Colour::WHITE: return ui::color(theme::INK);
    case L::Colour::GREY: return ui::color(theme::BEDSIDE);
    case L::Colour::SOFT: return ui::color(theme::INK_SOFT);
    default: return same;
  }
}

// A colour at night with "Dimmer": the same colour, deeper.
lv_color_t deeper(lv_color_t c) { return lv_color_mix(c, black(), 150); }

L::Rgb rgb(lv_color_t c) { return {c.red, c.green, c.blue}; }
lv_color_t lv(L::Rgb c) { return lv_color_make(c.r, c.g, c.b); }

}  // namespace

class OraSaver : public tessera::Saver {
 public:
  explicit OraSaver(const tessera::Plugin *plugin) : plugin_(plugin) {}

  void show(const tessera::SaverContext &c) override {
    // An option the person never touched may not be in the word: read each with its default.
    const L::Colour digits = L::colour_of(c.options["digits"] | "gold");
    const L::Colour minutes = L::colour_of(c.options["minutes"] | "same");
    const L::Colour boxes = L::colour_of(c.options["boxes"] | "graphite");
    const L::Colour night = L::colour_of(c.options["night"] | "off");
    style_ = L::style_of(c.options["style"] | "cards");
    const L::Size size = L::size_of(c.options["size"] | "large");
    format_ = L::date_format_of(c.options["date_format"] | "short");
    row_ = L::row_of(c.options["row"] | "icons");
    leading_zero_ = c.options["leading_zero"] | true;
    seconds_ = c.options["seconds"] | true;

    const L::Ink ink = L::ink_for(digits, minutes, night, c.night);
    lv_color_t hours_ink = paint(ink.hours, ui::color(theme::BAUHAUS_YELLOW));
    lv_color_t minutes_ink = paint(ink.minutes, hours_ink);
    if (ink.dimmed) {
      hours_ink = deeper(hours_ink);
      minutes_ink = deeper(minutes_ink);
    }
    const lv_color_t date_ink = paint(L::colour_of(c.options["date"] | "soft"), hours_ink);
    const lv_color_t temp_ink = paint(L::colour_of(c.options["temperature"] | "same"), hours_ink);

    // What the glass and the fonts measure. AM or PM is there when the screen writes 12 hours now.
    const uint32_t now = tessera::epoch();
    const L::Clock clock = L::split_clock(tessera::clock_text(now ? now : 43200), leading_zero_);
    width_ = c.width;
    const bool small = size == L::Size::NORMAL;
    row_font_ = small ? Font::TITLE : Font::HEADLINE;
    // The row's icons in the top bar's icon font, as Tessera's own clock draws them: a screen carries in it the icons
    // of its clock row and top bar. The tile icon font holds only the icons of the screen's tiles (a screen without a
    // weather tile has no weather glyph there: the icon drew as nothing on real glass, while the bench carries all).
    icon_font_ = Font::ICON_SMALL;
    row_h_ = std::max(ui::line_height(row_font_), ui::line_height(icon_font_));
    margin_ = ui::mm(4);
    between_ = ui::mm(3);
    L::Metrics m{};
    m.width = c.width;
    m.height = c.height;
    m.digits_w = ui::text_width("88", Font::DIGITS);
    m.digits_h = ui::line_height(Font::DIGITS);
    m.row_h = row_h_;
    m.suffix_w = clock.suffix.empty() ? 0 : ui::text_width(clock.suffix, Font::BODY);
    m.pad = ui::mm(3);
    m.gap = ui::mm(4);
    m.margin = margin_;
    m.space = ui::px(6);
    layout_ = L::layout(m, size);
    digits_w_ = m.digits_w;
    digits_h_ = m.digits_h;

    // The boxes: graphite flip cards, tinted with the hours' colour, or in a colour of their own; an outline is a line
    // in that colour (the hours' own for graphite and tint) on the black glass.
    lv_color_t fill = ui::color(theme::CARD);
    if (boxes == L::Colour::TINT) fill = lv_color_mix(hours_ink, ui::color(theme::CARD), 52);
    else if (L::is_plain(boxes)) fill = lv_color_mix(paint(boxes, fill), black(), 110);
    // Coloured digits on a coloured box (red on blue) go darker until they read (ora_logic.h, "Contrast").
    fill = lv(L::readable_fill(rgb(fill), rgb(hours_ink), rgb(minutes_ink), rgb(black())));
    const lv_color_t line = L::is_plain(boxes) ? paint(boxes, hours_ink) : hours_ink;
    hours_ = box(c.parent, layout_.hours, fill, line, hours_ink);
    minutes_ = box(c.parent, layout_.minutes, fill, line, minutes_ink);

    // The seconds: one dot between two boxes, a colon of two dots without boxes.
    const int dot = L::dot_size(ui::mm(2), layout_.scale, m.gap, style_ != L::Style::PLAIN);
    if (style_ == L::Style::PLAIN) {
      const int apart = layout_.colon_across ? layout_.hours.w / 8 : layout_.hours.h / 6;
      const int dx = layout_.colon_across ? apart : 0, dy = layout_.colon_across ? 0 : apart;
      dot_a_ = make_dot(c.parent, dot, layout_.colon_x - dx, layout_.colon_y - dy, hours_ink);
      dot_b_ = make_dot(c.parent, dot, layout_.colon_x + dx, layout_.colon_y + dy, hours_ink);
    } else {
      dot_a_ = make_dot(c.parent, dot, layout_.colon_x, layout_.colon_y, hours_ink);
    }

    suffix_ = ui::label(c.parent, Font::BODY, theme::MUTED);
    lv_obj_set_style_text_color(suffix_, temp_ink, 0);
    lv_obj_set_pos(suffix_, layout_.suffix_x, layout_.suffix_y - ui::line_height(Font::BODY));

    // The line under the clock: the date, the row's icons (and words), the temperature. Made once, hidden until used.
    date_ = row_label(c.parent, row_font_, date_ink);
    for (int i = 0; i < MAX_ITEMS; ++i) {
      icons_[i] = row_label(c.parent, icon_font_, temp_ink);
      words_[i] = row_label(c.parent, row_font_, temp_ink);
    }
    temperature_ = row_label(c.parent, row_font_, temp_ink);
  }

  // The clock step's contract: `temperature` ("21°", "" for none) and `items` (the clock's row of entities, each with
  // the codepoint of its icon in Tessera's set and its words).
  void on_state(JsonObjectConst data) override {
    bool changed = false;
    const char *degrees = data["temperature"] | "";
    if (std::strncmp(degrees, degrees_, sizeof(degrees_)) != 0) {
      std::snprintf(degrees_, sizeof(degrees_), "%s", degrees);
      changed = true;
    }
    Item next[MAX_ITEMS] = {};
    int count = 0;
    for (JsonObjectConst item : data["items"].as<JsonArrayConst>()) {
      if (count == MAX_ITEMS) break;
      next[count].icon = item["icon"] | 0u;
      std::snprintf(next[count].text, sizeof(next[count].text), "%s", item["text"] | "");
      ESP_LOGD(TAG, "row item %d: icon %05X, text '%s'", count, static_cast<unsigned>(next[count].icon), next[count].text);
      ++count;
    }
    if (count != count_ || std::memcmp(next, items_, sizeof(items_)) != 0) {
      std::memcpy(items_, next, sizeof(items_));
      count_ = count;
      changed = true;
    }
    if (changed) paint_row();
  }

  void on_tick(uint32_t epoch) override {
    if (!epoch) {  // the screen's clock is not set yet
      ui::set_text(lv_obj_get_child(hours_, 0), "--");
      ui::set_text(lv_obj_get_child(minutes_, 0), "--");
      return;
    }
    if (seconds_ && (epoch % 2 == 0) == lv_obj_has_flag(dot_a_, LV_OBJ_FLAG_HIDDEN)) {
      set_shown(dot_a_, epoch % 2 == 0);
      set_shown(dot_b_, epoch % 2 == 0);
    }
    if (epoch / 60 == minute_) return;
    minute_ = epoch / 60;
    const L::Clock clock = L::split_clock(tessera::clock_text(epoch), leading_zero_);
    ui::set_text(lv_obj_get_child(hours_, 0), clock.hours);
    ui::set_text(lv_obj_get_child(minutes_, 0), clock.minutes);
    const int sw = ui::text_width(clock.suffix, Font::BODY);
    lv_obj_set_width(suffix_, sw > 0 ? sw : 1);
    ui::set_text(suffix_, clock.suffix);
    const tessera::LocalTime now = tessera::local_time(epoch);
    if (now.day != day_) {
      day_ = now.day;
      paint_row();
    }
  }

 private:
  struct Item {
    uint32_t icon;
    char text[24];
  };

  // A box with the digits in its middle, scaled about their own middle when the face is large. Cards: a filled block
  // with the hinge across it; outline: a line around it; plain: nothing but the digits.
  lv_obj_t *box(lv_obj_t *parent, const L::Box &b, lv_color_t fill, lv_color_t line, lv_color_t ink) {
    lv_obj_t *card = ui::block(parent, theme::CARD);
    lv_obj_set_size(card, b.w, b.h);
    lv_obj_set_pos(card, b.x, b.y);
    if (style_ == L::Style::CARDS) {
      lv_obj_set_style_bg_color(card, fill, 0);
    } else {
      lv_obj_set_style_bg_opa(card, LV_OPA_TRANSP, 0);
      if (style_ == L::Style::OUTLINE) {
        lv_obj_set_style_border_color(card, line, 0);
        lv_obj_set_style_border_width(card, std::max(2, ui::px(2) * layout_.scale / 256), 0);
        lv_obj_set_style_border_opa(card, LV_OPA_COVER, 0);
      }
    }
    // The label is as wide as the card once scaled, so the hinge inside it spans the card; scaled about its middle.
    lv_obj_t *digits = ui::label(card, Font::DIGITS, theme::INK);
    lv_obj_set_style_text_color(digits, ink, 0);
    const int lw = b.w * 256 / layout_.scale;
    lv_obj_set_width(digits, lw);
    lv_obj_set_style_text_align(digits, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(digits, (b.w - lw) / 2, (b.h - digits_h_) / 2);
    if (layout_.scale != 256) {
      lv_obj_set_style_transform_pivot_x(digits, lw / 2, 0);
      lv_obj_set_style_transform_pivot_y(digits, digits_h_ / 2, 0);
      lv_obj_set_style_transform_scale(digits, layout_.scale, 0);
    }
    if (style_ == L::Style::CARDS) {
      // The hinge across the middle of the digits, where the card folds: a child of the digits, so it lies over them
      // and scales with them, as in Tessera's Flip clock.
      lv_obj_t *hinge = ui::block(digits, theme::CAMERA_PAGE);
      lv_obj_set_style_radius(hinge, 0, 0);
      const int h = std::max(1, ui::px(2));
      lv_obj_set_size(hinge, lw, h);
      lv_obj_set_pos(hinge, 0, (digits_h_ - h) / 2);
    }
    return card;
  }

  lv_obj_t *make_dot(lv_obj_t *parent, int size, int cx, int cy, lv_color_t ink) {
    lv_obj_t *d = ui::block(parent, theme::INK_SOFT);
    lv_obj_set_style_bg_color(d, ink, 0);
    lv_obj_set_style_radius(d, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_size(d, size, size);
    lv_obj_set_pos(d, cx - size / 2, cy - size / 2);
    // Without the seconds a plain colon stands still and lit; the dot between boxes stays away.
    if (seconds_ || style_ != L::Style::PLAIN) lv_obj_add_flag(d, LV_OBJ_FLAG_HIDDEN);
    return d;
  }

  static lv_obj_t *row_label(lv_obj_t *parent, Font font, lv_color_t ink) {
    lv_obj_t *l = ui::label(parent, font, theme::INK_SOFT);
    lv_obj_set_style_text_color(l, ink, 0);
    lv_obj_add_flag(l, LV_OBJ_FLAG_HIDDEN);
    return l;
  }

  static void set_shown(lv_obj_t *o, bool on) {
    if (!o) return;
    if (on) lv_obj_remove_flag(o, LV_OBJ_FLAG_HIDDEN);
    else lv_obj_add_flag(o, LV_OBJ_FLAG_HIDDEN);
  }

  // One part of the line: its text, its width, shown or hidden, placed at x in the middle of the line's height.
  int place(lv_obj_t *o, const std::string &text, Font font, int x) {
    const int w = text.empty() ? 0 : ui::text_width(text, font);
    ui::set_text(o, text);
    lv_obj_set_width(o, w > 0 ? w : 1);
    lv_obj_set_pos(o, x, layout_.row_y + (row_h_ - ui::line_height(font)) / 2);
    set_shown(o, w > 0);
    return w;
  }

  std::string date_text() const {
    const uint32_t epoch = tessera::epoch();
    if (!epoch || format_ == L::DateFormat::NONE) return "";
    if (format_ == L::DateFormat::SHORT) return tessera::date_text(epoch);
    const tessera::LocalTime t = tessera::local_time(epoch);
    const bool numeric = format_ == L::DateFormat::NUMERIC;
    return L::fill_date(plugin_->text(L::date_pattern_key(format_)), numeric ? L::two(t.day) : std::to_string(t.day),
                        numeric ? L::two(t.month) : std::string(plugin_->text(L::month_key(t.month).c_str())),
                        plugin_->text(L::weekday_key(t.weekday).c_str()));
  }

  // The line under the clock, in the middle of the glass: the date, the row's items that fit, the temperature.
  void paint_row() {
    const std::string date = date_text();
    const std::string degrees = degrees_;
    const int small_gap = ui::px(4);
    std::vector<int> widths;
    std::string icon_text[MAX_ITEMS], word_text[MAX_ITEMS];
    for (int i = 0; i < count_ && row_ != L::Row::OFF; ++i) {
      icon_text[i] = items_[i].icon ? ui::icon(items_[i].icon) : "";
      word_text[i] = row_ == L::Row::FULL ? items_[i].text : "";
      const int iw = icon_text[i].empty() ? 0 : ui::text_width(icon_text[i], icon_font_);
      const int ww = word_text[i].empty() ? 0 : ui::text_width(word_text[i], row_font_);
      widths.push_back(iw + ww + (iw && ww ? small_gap : 0));
    }
    const int dw = date.empty() ? 0 : ui::text_width(date, row_font_);
    const int tw = degrees.empty() ? 0 : ui::text_width(degrees, row_font_);
    const L::RowPlan plan = L::plan_row(width_ - 2 * margin_, dw, widths, tw, between_);

    int x = (width_ - plan.width) / 2;
    auto next = [&](int w) {
      if (w > 0) x += w + between_;
    };
    next(place(date_, date, row_font_, x));
    for (int i = 0; i < MAX_ITEMS; ++i) {
      const bool shown = i < plan.items && widths[i] > 0;
      const int iw = place(icons_[i], shown ? icon_text[i] : "", icon_font_, x);
      const int ww = place(words_[i], shown ? word_text[i] : "", row_font_, x + iw + (iw ? small_gap : 0));
      next(iw + ww + (iw && ww ? small_gap : 0));
    }
    place(temperature_, degrees, row_font_, x);
  }

  const tessera::Plugin *plugin_;
  L::Layout layout_{};
  L::Style style_ = L::Style::CARDS;
  L::DateFormat format_ = L::DateFormat::SHORT;
  L::Row row_ = L::Row::ICONS;
  Font row_font_ = Font::TITLE, icon_font_ = Font::ICON_SMALL;
  bool seconds_ = true, leading_zero_ = true;
  int width_ = 0, digits_w_ = 0, digits_h_ = 0, day_ = 0, row_h_ = 0, margin_ = 0, between_ = 0, count_ = 0;
  uint32_t minute_ = 0;
  char degrees_[16] = "";
  Item items_[MAX_ITEMS] = {};
  lv_obj_t *hours_{}, *minutes_{}, *dot_a_{}, *dot_b_{}, *suffix_{}, *date_{}, *temperature_{};
  lv_obj_t *icons_[MAX_ITEMS] = {}, *words_[MAX_ITEMS] = {};
};

void Ora::setup() {
  // The id is the saver's id in the manifest, and its step must be the manifest's (clock).
  add_saver("ora", tessera::SaverStep::CLOCK, [this]() { return new OraSaver(this); });
}

}  // namespace esphome::ora
