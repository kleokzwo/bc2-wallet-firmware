#include "bc2_display_adapter.h"
#include <cstdint>
#include <cstdio>
#include "board_power_bsp.h"
#include "epaper_driver_bsp.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <algorithm>
#include <cctype>
#include <cstring>

namespace {
constexpr int kWidth = 200;
constexpr int kHeight = 200;
constexpr int kBufferLength = 5000;
const char *kTag = "bc2_display";
epaper_driver_display *g_display = nullptr;
board_power_bsp_t *g_power = nullptr;
char g_previous_title[32] = {};
unsigned int g_partial_refresh_count = 0U;
constexpr unsigned int kMaxPartialRefreshes = 12U;

struct Glyph { char character; uint8_t rows[7]; };
constexpr Glyph kGlyphs[] = {
 {' ',{0,0,0,0,0,0,0}}, {'-',{0,0,0,31,0,0,0}}, {'.',{0,0,0,0,0,12,12}}, {':',{0,12,12,0,12,12,0}}, {'/',{1,2,4,8,16,0,0}}, {'<',{1,2,4,8,4,2,1}}, {'?',{14,17,1,2,4,0,4}},
 {'0',{14,17,19,21,25,17,14}}, {'1',{4,12,4,4,4,4,14}}, {'2',{14,17,1,2,4,8,31}}, {'3',{30,1,1,14,1,1,30}}, {'4',{2,6,10,18,31,2,2}}, {'5',{31,16,16,30,1,1,30}}, {'6',{14,16,16,30,17,17,14}}, {'7',{31,1,2,4,8,8,8}}, {'8',{14,17,17,14,17,17,14}}, {'9',{14,17,17,15,1,1,14}},
 {'A',{14,17,17,31,17,17,17}}, {'B',{30,17,17,30,17,17,30}}, {'C',{14,17,16,16,16,17,14}}, {'D',{30,17,17,17,17,17,30}}, {'E',{31,16,16,30,16,16,31}}, {'F',{31,16,16,30,16,16,16}}, {'G',{14,17,16,23,17,17,15}}, {'H',{17,17,17,31,17,17,17}}, {'I',{14,4,4,4,4,4,14}}, {'J',{7,2,2,2,18,18,12}}, {'K',{17,18,20,24,20,18,17}}, {'L',{16,16,16,16,16,16,31}}, {'M',{17,27,21,21,17,17,17}}, {'N',{17,25,21,19,17,17,17}}, {'O',{14,17,17,17,17,17,14}}, {'P',{30,17,17,30,16,16,16}}, {'Q',{14,17,17,17,21,18,13}}, {'R',{30,17,17,30,20,18,17}}, {'S',{15,16,16,14,1,1,30}}, {'T',{31,4,4,4,4,4,4}}, {'U',{17,17,17,17,17,17,14}}, {'V',{17,17,17,17,17,10,4}}, {'W',{17,17,17,21,21,21,10}}, {'X',{17,17,10,4,10,17,17}}, {'Y',{17,17,10,4,4,4,4}}, {'Z',{31,1,2,4,8,16,31}},
};
const uint8_t *glyph_for(char value) {
 const char normalized = static_cast<char>(std::toupper(static_cast<unsigned char>(value)));
 for (const auto &glyph : kGlyphs) if (glyph.character == normalized) return glyph.rows;
 for (const auto &glyph : kGlyphs) if (glyph.character == '?') return glyph.rows;
 return kGlyphs[0].rows;
}
void draw_char(int x,int y,char value,int scale) {
 const uint8_t *rows=glyph_for(value);
 for(int row=0;row<7;++row) for(int col=0;col<5;++col) if((rows[row]&(1U<<(4-col)))!=0U)
  for(int dy=0;dy<scale;++dy) for(int dx=0;dx<scale;++dx)
   g_display->EPD_DrawColorPixel(static_cast<uint16_t>(x+col*scale+dx),static_cast<uint16_t>(y+row*scale+dy),DRIVER_COLOR_BLACK);
}
void draw_line(const char *text,int y,int scale,bool centered) {
 if (text == nullptr) return;
 const size_t max_chars = static_cast<size_t>(kWidth / (6 * scale));
 const size_t length = std::min(std::strlen(text), max_chars);
 int x = centered ? std::max(0, (kWidth - static_cast<int>(length) * 6 * scale) / 2) : 8;
 for (size_t i = 0; i < length; ++i) {
  draw_char(x, y, text[i], scale);
  x += 6 * scale;
 }
}
void draw_multiline(const char *text,int start_y,int scale,int max_lines) {
 if (text == nullptr) return;
 const int line_height = 9 * scale;
 const size_t max_chars = static_cast<size_t>((kWidth - 16) / (6 * scale));
 const char *cursor = text;
 for (int line = 0; line < max_lines && *cursor != '\0'; ++line) {
  char buffer[40] = {};
  size_t used = 0;
  while (*cursor != '\0' && *cursor != '\n' && used < max_chars) {
   buffer[used++] = *cursor++;
  }
  if (*cursor == '\n') ++cursor;
  draw_line(buffer, start_y + line * line_height, scale, false);
  while (*cursor != '\0' && *cursor != '\n' && used == max_chars) ++cursor;
  if (*cursor == '\n') ++cursor;
 }
}
void fill_rect(int x, int y, int width, int height) {
 for (int py = y; py < y + height; ++py)
  for (int px = x; px < x + width; ++px)
   g_display->EPD_DrawColorPixel(static_cast<uint16_t>(px), static_cast<uint16_t>(py), DRIVER_COLOR_BLACK);
}
void draw_rect(int x, int y, int width, int height, int thickness = 1) {
 fill_rect(x, y, width, thickness); fill_rect(x, y + height - thickness, width, thickness);
 fill_rect(x, y, thickness, height); fill_rect(x + width - thickness, y, thickness, height);
}
void draw_usb_icon(int x, int y) {
 fill_rect(x + 5, y, 2, 11); fill_rect(x + 2, y + 4, 8, 2);
 fill_rect(x, y + 3, 3, 3); fill_rect(x + 9, y + 3, 3, 3); fill_rect(x + 4, y + 10, 4, 3);
}
void draw_battery_icon(int x, int y) {
 draw_rect(x, y, 17, 9); fill_rect(x + 17, y + 3, 2, 3); fill_rect(x + 3, y + 3, 8, 3);
}
void draw_lock_icon(int center_x, int y) {
 draw_rect(center_x - 13, y + 12, 26, 22, 2);
 fill_rect(center_x - 8, y + 3, 2, 11); fill_rect(center_x + 6, y + 3, 2, 11);
 fill_rect(center_x - 6, y, 12, 2); fill_rect(center_x - 11, y + 5, 3, 7); fill_rect(center_x + 8, y + 5, 3, 7);
 fill_rect(center_x - 1, y + 20, 3, 7);
}
void draw_bc2_mark(void) {
 draw_rect(8, 7, 30, 21, 2); draw_line("BC2", 13, 1, false);
}
void draw_status_bar(void) {
 draw_bc2_mark(); draw_usb_icon(153, 11); draw_battery_icon(174, 11); fill_rect(7, 34, 186, 1);
}
void draw_lock_screen(const char *body, const char *footer) {
 draw_status_bar();
 draw_lock_icon(100, 46);
 draw_line("GERAET", 91, 2, true);
 draw_line("GESPERRT", 110, 2, true);
 draw_rect(47, 137, 106, 18);
 draw_line("STATUS: SICHER", 143, 1, true);
 if (body != nullptr) draw_line(body, 163, 1, true);
 fill_rect(7, 178, 186, 1);
 draw_line(footer != nullptr ? footer : "KURZ: ENTSPERREN", 185, 1, true);
}
void draw_key(int x, int y, char key, bool selected) {
 draw_rect(x, y, 38, 24, selected ? 3 : 1);
 draw_char(x + 16, y + 5, key, 2);
}
void draw_pin_screen(const char *title, unsigned int selected_key, unsigned int digit_count) {
 static constexpr char keys[] = {'1','2','3','4','5','6','7','8','9','<','0'};
 draw_status_bar();
 draw_line(title, 39, 1, true);
 for (unsigned int index = 0; index < 6U; ++index) {
  draw_rect(31 + static_cast<int>(index) * 24, 60, 18, 18);
  if (index < digit_count) fill_rect(37 + static_cast<int>(index) * 24, 66, 6, 6);
 }
 for (unsigned int index = 0; index < 9U; ++index)
  draw_key(33 + static_cast<int>(index % 3U) * 48,
           84 + static_cast<int>(index / 3U) * 28,
           keys[index], index == selected_key);
 draw_key(57, 168, '<', selected_key == 9U);
 draw_key(105, 168, '0', selected_key == 10U);
}
void draw_receive_screen(const char *address) {
 draw_status_bar();
 draw_line("ADRESSE PRUEFEN", 42, 1, true);
 draw_line("BC2 MAINNET", 58, 1, true);
 draw_rect(10, 73, 180, 86, 2);
 if (address != nullptr) {
  const size_t length = std::min(std::strlen(address), static_cast<size_t>(90));
  for (size_t offset = 0U, line = 0U; offset < length && line < 6U; offset += 15U, ++line) {
   char part[16] = {};
   const size_t count = std::min(static_cast<size_t>(15), length - offset);
   std::memcpy(part, address + offset, count);
   draw_line(part, 82 + static_cast<int>(line) * 12, 1, true);
  }
 }
 fill_rect(7, 178, 186, 1);
 draw_line("BEIDE: BESTAETIGEN", 185, 1, true);
}
}
extern "C" esp_err_t bc2_display_adapter_init(void) {
 if (g_display != nullptr) return ESP_OK;

 g_power = new board_power_bsp_t(GPIO_NUM_6, GPIO_NUM_42, GPIO_NUM_17);
 if (g_power == nullptr) return ESP_ERR_NO_MEM;
 g_power->POWEER_EPD_ON();

 custom_lcd_spi_t config = {};
 config.cs = GPIO_NUM_11;
 config.dc = GPIO_NUM_10;
 config.rst = GPIO_NUM_9;
 config.busy = GPIO_NUM_8;
 config.mosi = GPIO_NUM_13;
 config.scl = GPIO_NUM_12;
 config.spi_host = SPI2_HOST;
 config.buffer_len = kBufferLength;

 g_display = new epaper_driver_display(kWidth, kHeight, config);
 if (g_display == nullptr) return ESP_ERR_NO_MEM;

 g_display->EPD_Init();
 g_display->EPD_Clear();
 g_display->EPD_DisplayPartBaseImage();
 g_display->EPD_Init_Partial();

 ESP_LOGI(kTag, "Official Waveshare V2 driver initialized unchanged");
 return ESP_OK;
}

extern "C" esp_err_t bc2_display_adapter_show_text(const char *title,
                                                     const char *body,
                                                     const char *footer,
                                                     bool full_refresh) {
 if (g_display == nullptr) return ESP_ERR_INVALID_STATE;

 const char *current_title = title != nullptr ? title : "";
 const bool screen_changed = std::strncmp(g_previous_title, current_title,
                                          sizeof(g_previous_title)) != 0;
 const bool refresh_limit_reached = g_partial_refresh_count >= kMaxPartialRefreshes;
 const bool use_full_refresh = full_refresh || screen_changed || refresh_limit_reached;

 g_display->EPD_Clear();
 if (title != nullptr && (std::strcmp(title, "PIN EINGEBEN") == 0 ||
                          std::strcmp(title, "PIN ANLEGEN") == 0 ||
                          std::strcmp(title, "PIN WIEDERHOLEN") == 0 ||
                          std::strcmp(title, "EMPFANG FREIGEBEN") == 0 ||
                          std::strcmp(title, "ZAHLUNG FREIGEBEN") == 0)) {
  unsigned int selected_key = 0U;
  unsigned int pin_digit_count = 0U;
  if (body != nullptr) (void)std::sscanf(body, "%u:%u", &selected_key, &pin_digit_count);
  draw_pin_screen(title, selected_key, pin_digit_count);
 } else if (title != nullptr && std::strcmp(title, "GERAET GESPERRT") == 0) {
  draw_lock_screen(body, footer);
 } else if (title != nullptr && std::strcmp(title, "ADRESSE PRUEFEN") == 0) {
  draw_receive_screen(body);
 } else {
  draw_status_bar();
  draw_line(title != nullptr ? title : "BC2 COLD WALLET", 43, 2, true);
  draw_multiline(body, 75, 1, 10);
  fill_rect(7, 178, 186, 1);
  draw_line(footer != nullptr ? footer : "BOOT: BESTAETIGEN", 185, 1, true);
 }

 if (use_full_refresh) {
  g_display->EPD_Init();
  g_display->EPD_DisplayPartBaseImage();
  g_display->EPD_Init_Partial();
  g_partial_refresh_count = 0U;
 } else {
  g_display->EPD_DisplayPart();
  ++g_partial_refresh_count;
 }
 (void)std::snprintf(g_previous_title, sizeof(g_previous_title), "%s", current_title);
 ESP_LOGI(kTag, "BC2 frame displayed through official Waveshare %s path",
          use_full_refresh ? "full" : "partial");
 return ESP_OK;
}
