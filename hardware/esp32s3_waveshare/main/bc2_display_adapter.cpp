#include "bc2_display_adapter.h"
#include <cstdint>
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

struct Glyph { char character; uint8_t rows[7]; };
constexpr Glyph kGlyphs[] = {
 {' ',{0,0,0,0,0,0,0}}, {'-',{0,0,0,31,0,0,0}}, {'.',{0,0,0,0,0,12,12}}, {':',{0,12,12,0,12,12,0}}, {'/',{1,2,4,8,16,0,0}}, {'?',{14,17,1,2,4,0,4}},
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
 (void)full_refresh;
 if (g_display == nullptr) return ESP_ERR_INVALID_STATE;

 g_display->EPD_Clear();
 draw_line(title != nullptr ? title : "BC2 COLD WALLET", 12, 2, true);
 draw_multiline(body, 56, 1, 9);
 draw_line(footer != nullptr ? footer : "BOOT: BESTAETIGEN", 181, 1, true);

 g_display->EPD_DisplayPart();
 ESP_LOGI(kTag, "BC2 frame displayed through official Waveshare partial path");
 return ESP_OK;
}
