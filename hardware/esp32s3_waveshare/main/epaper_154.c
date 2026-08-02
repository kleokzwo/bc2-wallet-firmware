#include "epaper_154.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <ctype.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>

/* Official Waveshare ESP32-S3-ePaper-1.54 V2 mapping. */
#define PIN_EPD_DC    GPIO_NUM_10
#define PIN_EPD_CS    GPIO_NUM_11
#define PIN_EPD_SCLK  GPIO_NUM_12
#define PIN_EPD_MOSI  GPIO_NUM_13
#define PIN_EPD_RESET GPIO_NUM_9
#define PIN_EPD_BUSY  GPIO_NUM_8
#define PIN_EPD_POWER GPIO_NUM_6

#define FRAME_BYTES ((BC2_EPAPER_WIDTH * BC2_EPAPER_HEIGHT) / 8)
#define BUSY_TIMEOUT_MS 15000U

static const char *TAG = "bc2_epaper";
static spi_device_handle_t g_spi;
static bool g_ready;
static uint8_t g_frame[FRAME_BYTES];

/* Official Waveshare 1.54-inch full-refresh waveform. */
static const uint8_t WF_FULL_1IN54[159] = {
    0x80, 0x48, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x40, 0x48, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x80, 0x48, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x40, 0x48, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x01, 0x00, 0x08, 0x01,
    0x00, 0x02, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00, 0x22, 0x17, 0x41,
    0x00, 0x32, 0x20
};

static void delay_ms(uint32_t ms){ vTaskDelay(pdMS_TO_TICKS(ms)); }

static esp_err_t spi_tx(const uint8_t *buf,size_t len){
    if(!buf||!len) return ESP_ERR_INVALID_ARG;
    spi_transaction_t t={0}; t.length=len*8U; t.tx_buffer=buf;
    return spi_device_polling_transmit(g_spi,&t);
}
static esp_err_t command(uint8_t v){ gpio_set_level(PIN_EPD_DC,0); gpio_set_level(PIN_EPD_CS,0); esp_err_t r=spi_tx(&v,1); gpio_set_level(PIN_EPD_CS,1); return r; }
static esp_err_t data_buffer(const uint8_t *b,size_t n){ gpio_set_level(PIN_EPD_DC,1); gpio_set_level(PIN_EPD_CS,0); esp_err_t r=spi_tx(b,n); gpio_set_level(PIN_EPD_CS,1); return r; }
static esp_err_t data(uint8_t v){ return data_buffer(&v,1); }

static esp_err_t wait_idle(const char *phase) {
    const int64_t start_us = esp_timer_get_time();
    const int64_t timeout_us = (int64_t)BUSY_TIMEOUT_MS * 1000LL;

    while (gpio_get_level(PIN_EPD_BUSY) == 1) {
        const int64_t elapsed_us = esp_timer_get_time() - start_us;
        if (elapsed_us >= timeout_us) {
            ESP_LOGE(TAG, "BUSY timeout after %u ms (%s)",
                     (unsigned)(elapsed_us / 1000LL), phase);
            return ESP_ERR_TIMEOUT;
        }

        /* pdMS_TO_TICKS(5) becomes zero when FreeRTOS uses a 100 Hz tick.
         * One real tick prevents a tight loop and gives the panel time to refresh. */
        vTaskDelay(1);
    }

    const int64_t elapsed_us = esp_timer_get_time() - start_us;
    ESP_LOGI(TAG, "BUSY idle after %u ms (%s)",
             (unsigned)(elapsed_us / 1000LL), phase);
    return ESP_OK;
}
static void hardware_reset(void){ gpio_set_level(PIN_EPD_RESET,1); delay_ms(50); gpio_set_level(PIN_EPD_RESET,0); delay_ms(20); gpio_set_level(PIN_EPD_RESET,1); delay_ms(50); }

static esp_err_t set_window(uint16_t xs,uint16_t ys,uint16_t xe,uint16_t ye){
    ESP_RETURN_ON_ERROR(command(0x44),TAG,"x window cmd");
    ESP_RETURN_ON_ERROR(data((uint8_t)(xs>>3)),TAG,"x start"); ESP_RETURN_ON_ERROR(data((uint8_t)(xe>>3)),TAG,"x end");
    ESP_RETURN_ON_ERROR(command(0x45),TAG,"y window cmd");
    ESP_RETURN_ON_ERROR(data((uint8_t)ys),TAG,"ys lo"); ESP_RETURN_ON_ERROR(data((uint8_t)(ys>>8)),TAG,"ys hi");
    ESP_RETURN_ON_ERROR(data((uint8_t)ye),TAG,"ye lo"); ESP_RETURN_ON_ERROR(data((uint8_t)(ye>>8)),TAG,"ye hi");
    return ESP_OK;
}
static esp_err_t set_cursor(uint16_t x,uint16_t y){
    ESP_RETURN_ON_ERROR(command(0x4E),TAG,"x cursor cmd"); ESP_RETURN_ON_ERROR(data((uint8_t)x),TAG,"x cursor");
    ESP_RETURN_ON_ERROR(command(0x4F),TAG,"y cursor cmd"); ESP_RETURN_ON_ERROR(data((uint8_t)y),TAG,"y cursor lo"); return data((uint8_t)(y>>8));
}
static esp_err_t set_lut(const uint8_t *lut){
    ESP_RETURN_ON_ERROR(command(0x32),TAG,"lut cmd"); ESP_RETURN_ON_ERROR(data_buffer(lut,153),TAG,"lut body");
    ESP_RETURN_ON_ERROR(wait_idle("lut"),TAG,"lut busy");
    ESP_RETURN_ON_ERROR(command(0x3F),TAG,"gate cmd"); ESP_RETURN_ON_ERROR(data(lut[153]),TAG,"gate data");
    ESP_RETURN_ON_ERROR(command(0x03),TAG,"vgh cmd"); ESP_RETURN_ON_ERROR(data(lut[154]),TAG,"vgh data");
    ESP_RETURN_ON_ERROR(command(0x04),TAG,"vcom cmd"); ESP_RETURN_ON_ERROR(data_buffer(&lut[155],3),TAG,"vcom data");
    ESP_RETURN_ON_ERROR(command(0x2C),TAG,"vcom reg cmd"); return data(lut[158]);
}

static const uint8_t *glyph(char ch){
 static const uint8_t blank[5]={0}; static const uint8_t question[5]={2,1,0x51,9,6};
 static const uint8_t digits[10][5]={{0x3E,0x51,0x49,0x45,0x3E},{0,0x42,0x7F,0x40,0},{0x42,0x61,0x51,0x49,0x46},{0x21,0x41,0x45,0x4B,0x31},{0x18,0x14,0x12,0x7F,0x10},{0x27,0x45,0x45,0x45,0x39},{0x3C,0x4A,0x49,0x49,0x30},{1,0x71,9,5,3},{0x36,0x49,0x49,0x49,0x36},{6,0x49,0x49,0x29,0x1E}};
 static const uint8_t letters[26][5]={{0x7E,0x11,0x11,0x11,0x7E},{0x7F,0x49,0x49,0x49,0x36},{0x3E,0x41,0x41,0x41,0x22},{0x7F,0x41,0x41,0x22,0x1C},{0x7F,0x49,0x49,0x49,0x41},{0x7F,9,9,9,1},{0x3E,0x41,0x49,0x49,0x7A},{0x7F,8,8,8,0x7F},{0,0x41,0x7F,0x41,0},{0x20,0x40,0x41,0x3F,1},{0x7F,8,0x14,0x22,0x41},{0x7F,0x40,0x40,0x40,0x40},{0x7F,2,0x0C,2,0x7F},{0x7F,4,8,0x10,0x7F},{0x3E,0x41,0x41,0x41,0x3E},{0x7F,9,9,9,6},{0x3E,0x41,0x51,0x21,0x5E},{0x7F,9,0x19,0x29,0x46},{0x46,0x49,0x49,0x49,0x31},{1,1,0x7F,1,1},{0x3F,0x40,0x40,0x40,0x3F},{0x1F,0x20,0x40,0x20,0x1F},{0x3F,0x40,0x38,0x40,0x3F},{0x63,0x14,8,0x14,0x63},{7,8,0x70,8,7},{0x61,0x51,0x49,0x45,0x43}};
 static const uint8_t dash[5]={8,8,8,8,8},dot[5]={0,0x60,0x60,0,0},colon[5]={0,0x36,0x36,0,0},slash[5]={0x20,0x10,8,4,2};
    if (ch == ' ') {
        return blank;
    }
    if (ch >= '0' && ch <= '9') {
        return digits[ch - '0'];
    }

    ch = (char)toupper((unsigned char)ch);
    if (ch >= 'A' && ch <= 'Z') {
        return letters[ch - 'A'];
    }
    if (ch == '-') {
        return dash;
    }
    if (ch == '.') {
        return dot;
    }
    if (ch == ':') {
        return colon;
    }
    if (ch == '/') {
        return slash;
    }
    return question;
}

static void pixel(int x, int y, bool black) {
    if (x < 0 || y < 0 || x >= 200 || y >= 200) {
        return;
    }

    const size_t index = (size_t)y * 25U + (size_t)x / 8U;
    const uint8_t mask = (uint8_t)(0x80U >> (x & 7));
    if (black) {
        g_frame[index] &= (uint8_t)~mask;
    } else {
        g_frame[index] |= mask;
    }
}

static void draw_char(int x, int y, char character, int scale) {
    const uint8_t *character_glyph = glyph(character);
    for (int column = 0; column < 5; ++column) {
        for (int row = 0; row < 7; ++row) {
            if ((character_glyph[column] & (1U << row)) == 0U) {
                continue;
            }
            for (int dx = 0; dx < scale; ++dx) {
                for (int dy = 0; dy < scale; ++dy) {
                    pixel(x + column * scale + dx, y + row * scale + dy, true);
                }
            }
        }
    }
}

static void draw_line(int x, int y, const char *text, int scale, size_t max_chars) {
    if (text == NULL) {
        return;
    }
    for (size_t index = 0; text[index] != '\0' && index < max_chars; ++index) {
        draw_char(x + (int)index * 6 * scale, y, text[index], scale);
    }
}

static void draw_wrapped(int x, int y, const char *text, int scale, int line_height,
                         size_t max_chars, int max_lines) {
    if (text == NULL) {
        return;
    }

    char line[40];
    size_t length = 0;
    int line_index = 0;
    for (const char *cursor = text;; ++cursor) {
        const char character = *cursor;
        const bool line_break = character == '\n' || character == '\0' || length == max_chars;
        if (line_break) {
            line[length] = '\0';
            draw_line(x, y + line_index * line_height, line, scale, max_chars);
            ++line_index;
            length = 0;
            if (character == '\0' || line_index >= max_lines) {
                break;
            }
            if (character == '\n') {
                continue;
            }
        }
        if (character != '\n' && length < sizeof(line) - 1U) {
            line[length++] = character;
        }
    }
}

static uint32_t frame_checksum(void) {
    uint32_t checksum = 2166136261U;
    for (size_t index = 0; index < sizeof(g_frame); ++index) {
        checksum ^= g_frame[index];
        checksum *= 16777619U;
    }
    return checksum;
}

static size_t frame_black_pixel_count(void) {
    size_t black_pixels = 0U;
    for (size_t index = 0; index < sizeof(g_frame); ++index) {
        uint8_t value = (uint8_t)~g_frame[index];
        while (value != 0U) {
            black_pixels += (size_t)(value & 1U);
            value >>= 1U;
        }
    }
    return black_pixels;
}

static esp_err_t refresh(void) {
    const uint32_t checksum = frame_checksum();
    const size_t black_pixels = frame_black_pixel_count();
    ESP_LOGI(TAG,
             "Starting official Waveshare full-frame refresh: bytes=%u checksum=%08" PRIX32
             " black_pixels=%u",
             (unsigned)sizeof(g_frame), checksum, (unsigned)black_pixels);

    /* Keep this path byte-for-byte equivalent to Waveshare's proven
     * EPD_Display() implementation. The controller window and cursor are
     * configured once during initialization. A full frame is then written to
     * current-image RAM (0x24), followed immediately by a full refresh.
     *
     * Do not rewrite window/cursor registers here and do not write 0x26 during
     * the initial full refresh. Both changes were absent from the official
     * dynamic demo that was verified on the physical board. */
    ESP_RETURN_ON_ERROR(command(0x24), TAG, "current RAM command");
    ESP_RETURN_ON_ERROR(data_buffer(g_frame, sizeof(g_frame)),
                        TAG, "current RAM transfer");
    ESP_LOGI(TAG, "Wrote official current-image RAM plane (command 0x24)");

    ESP_RETURN_ON_ERROR(command(0x22), TAG, "update command");
    ESP_RETURN_ON_ERROR(data(0xC7), TAG, "full update mode");
    ESP_RETURN_ON_ERROR(command(0x20), TAG, "display activation");
    ESP_RETURN_ON_ERROR(wait_idle("full refresh"), TAG, "refresh timeout");

    ESP_LOGI(TAG, "Official Waveshare full-frame refresh completed");
    return ESP_OK;
}

esp_err_t bc2_epaper_init(void){
    gpio_config_t out={.pin_bit_mask=(1ULL<<PIN_EPD_RESET)|(1ULL<<PIN_EPD_DC)|(1ULL<<PIN_EPD_CS)|(1ULL<<PIN_EPD_POWER),.mode=GPIO_MODE_OUTPUT,.pull_up_en=GPIO_PULLUP_ENABLE};
    gpio_config_t in={.pin_bit_mask=(1ULL<<PIN_EPD_BUSY),.mode=GPIO_MODE_INPUT};
    ESP_RETURN_ON_ERROR(gpio_config(&out),TAG,"output GPIO init"); ESP_RETURN_ON_ERROR(gpio_config(&in),TAG,"busy GPIO init");
    gpio_set_level(PIN_EPD_POWER,1); gpio_set_level(PIN_EPD_CS,1); gpio_set_level(PIN_EPD_RESET,1); delay_ms(100);
    spi_bus_config_t bus={.mosi_io_num=PIN_EPD_MOSI,.miso_io_num=-1,.sclk_io_num=PIN_EPD_SCLK,.quadwp_io_num=-1,.quadhd_io_num=-1,.max_transfer_sz=40000};
    ESP_RETURN_ON_ERROR(spi_bus_initialize(SPI2_HOST,&bus,SPI_DMA_CH_AUTO),TAG,"SPI bus init");
    spi_device_interface_config_t dev={.clock_speed_hz=40*1000*1000,.mode=0,.spics_io_num=-1,.queue_size=7};
    ESP_RETURN_ON_ERROR(spi_bus_add_device(SPI2_HOST,&dev,&g_spi),TAG,"SPI device init");
    hardware_reset(); ESP_RETURN_ON_ERROR(wait_idle("hardware reset"),TAG,"reset busy");
    ESP_RETURN_ON_ERROR(command(0x12),TAG,"software reset"); ESP_RETURN_ON_ERROR(wait_idle("software reset"),TAG,"sw reset busy");
    ESP_RETURN_ON_ERROR(command(0x01),TAG,"driver output"); {const uint8_t d[]={0xC7,0x00,0x01}; ESP_RETURN_ON_ERROR(data_buffer(d,3),TAG,"driver data");}
    ESP_RETURN_ON_ERROR(command(0x11),TAG,"entry mode"); ESP_RETURN_ON_ERROR(data(0x01),TAG,"entry data");
    ESP_RETURN_ON_ERROR(set_window(0,199,199,0),TAG,"window");
    ESP_RETURN_ON_ERROR(command(0x3C),TAG,"border"); ESP_RETURN_ON_ERROR(data(0x01),TAG,"border data");
    ESP_RETURN_ON_ERROR(command(0x18),TAG,"temp"); ESP_RETURN_ON_ERROR(data(0x80),TAG,"temp data");
    ESP_RETURN_ON_ERROR(command(0x22),TAG,"load temp"); ESP_RETURN_ON_ERROR(data(0xB1),TAG,"load temp data"); ESP_RETURN_ON_ERROR(command(0x20),TAG,"load temp activate");
    ESP_RETURN_ON_ERROR(set_cursor(0,199),TAG,"cursor"); ESP_RETURN_ON_ERROR(wait_idle("initial activation"),TAG,"initial busy");
    ESP_RETURN_ON_ERROR(set_lut(WF_FULL_1IN54),TAG,"full LUT");
    g_ready=true;
    ESP_LOGI(TAG,"Official Waveshare V2 BSP active: PWR=%d BUSY=%d RST=%d DC=%d CS=%d SCLK=%d MOSI=%d",PIN_EPD_POWER,PIN_EPD_BUSY,PIN_EPD_RESET,PIN_EPD_DC,PIN_EPD_CS,PIN_EPD_SCLK,PIN_EPD_MOSI);
    return ESP_OK;
}

esp_err_t bc2_epaper_show_text(const char *title,const char *body,const char *footer,bool full_refresh){ (void)full_refresh; if(!g_ready)return ESP_ERR_INVALID_STATE; memset(g_frame,0xFF,sizeof(g_frame)); draw_line(8,8,title,2,15); for(int x=8;x<192;x++)pixel(x,27,true); draw_wrapped(8,38,body,1,10,31,11); for(int x=8;x<192;x++)pixel(x,174,true); draw_wrapped(8,181,footer,1,9,31,2); return refresh(); }
bool bc2_epaper_is_ready(void){ return g_ready; }
