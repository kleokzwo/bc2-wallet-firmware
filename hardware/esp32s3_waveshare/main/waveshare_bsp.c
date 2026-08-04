#include "waveshare_bsp.h"

#include "driver/gpio.h"
#include "driver/usb_serial_jtag.h"
#include "bc2_display_adapter.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "nvs.h"
#include "nvs_flash.h"

#include <string.h>

#define BC2_USB_RX_BUFFER_SIZE 4096U
#define BC2_USB_TX_BUFFER_SIZE 4096U
#define BC2_USB_RX_TIMEOUT_TICKS 0U
#define BC2_USB_TX_TIMEOUT_TICKS pdMS_TO_TICKS(20U)

static const char *TAG = "bc2_bsp";

typedef struct {
    nvs_handle_t nvs;
    bool usb_ready;
    bool display_ready;
    bool button_ready;
    int button_previous_level;
    uint64_t button_last_change_ms;
} bsp_context_t;

static bsp_context_t g_context;

static bc2_hal_result_t display_present(void *context,
                                        const bc2_display_frame_t *frame) {
    bsp_context_t *bsp = (bsp_context_t *)context;
    if (bsp == NULL || frame == NULL) return BC2_HAL_ERROR_ARGUMENT;
    if (!bsp->display_ready) return BC2_HAL_ERROR_UNAVAILABLE;

    const esp_err_t result = bc2_display_adapter_show_text(frame->title,
                                                           frame->body,
                                                           frame->footer,
                                                           frame->require_full_refresh);
    return result == ESP_OK ? BC2_HAL_OK : BC2_HAL_ERROR_IO;
}

static bc2_hal_result_t button_poll(void *context,
                                    bc2_button_event_t *event) {
    bsp_context_t *bsp = (bsp_context_t *)context;
    if (bsp == NULL || event == NULL) return BC2_HAL_ERROR_ARGUMENT;
    if (!bsp->button_ready) return BC2_HAL_ERROR_UNAVAILABLE;

    memset(event, 0, sizeof(*event));
    const int level = gpio_get_level(GPIO_NUM_0);
    const uint64_t now = (uint64_t)(esp_timer_get_time() / 1000);
    if (level == bsp->button_previous_level) return BC2_HAL_ERROR_NOT_FOUND;
    if (now - bsp->button_last_change_ms < 35U) return BC2_HAL_ERROR_NOT_FOUND;

    bsp->button_previous_level = level;
    bsp->button_last_change_ms = now;
    event->button = BC2_BUTTON_CONFIRM;
    event->action = level == 0 ? BC2_BUTTON_PRESSED : BC2_BUTTON_RELEASED;
    event->timestamp_ms = now;
    return BC2_HAL_OK;
}

static uint64_t time_now_ms(void *context) {
    (void)context;
    return (uint64_t)(esp_timer_get_time() / 1000);
}

static bc2_hal_result_t random_fill(void *context,
                                    uint8_t *output,
                                    size_t output_size) {
    (void)context;
    if (output == NULL || output_size == 0U) return BC2_HAL_ERROR_ARGUMENT;

    esp_fill_random(output, output_size);
    return BC2_HAL_OK;
}

static bc2_hal_result_t storage_read(void *context,
                                     const char *key,
                                     uint8_t *output,
                                     size_t output_capacity,
                                     size_t *output_size) {
    bsp_context_t *bsp = (bsp_context_t *)context;
    if (bsp == NULL || key == NULL || output == NULL || output_size == NULL)
        return BC2_HAL_ERROR_ARGUMENT;

    size_t required_size = 0U;
    esp_err_t result = nvs_get_blob(bsp->nvs, key, NULL, &required_size);
    if (result == ESP_ERR_NVS_NOT_FOUND) return BC2_HAL_ERROR_NOT_FOUND;
    if (result != ESP_OK) return BC2_HAL_ERROR_IO;
    if (required_size > output_capacity) return BC2_HAL_ERROR_LIMIT;

    result = nvs_get_blob(bsp->nvs, key, output, &required_size);
    if (result != ESP_OK) return BC2_HAL_ERROR_IO;

    *output_size = required_size;
    return BC2_HAL_OK;
}

static bc2_hal_result_t storage_write(void *context,
                                      const char *key,
                                      const uint8_t *data,
                                      size_t data_size) {
    bsp_context_t *bsp = (bsp_context_t *)context;
    if (bsp == NULL || key == NULL || data == NULL || data_size == 0U)
        return BC2_HAL_ERROR_ARGUMENT;

    if (nvs_set_blob(bsp->nvs, key, data, data_size) != ESP_OK)
        return BC2_HAL_ERROR_IO;
    if (nvs_commit(bsp->nvs) != ESP_OK) return BC2_HAL_ERROR_IO;
    return BC2_HAL_OK;
}

static bc2_hal_result_t storage_remove(void *context,
                                       const char *key) {
    bsp_context_t *bsp = (bsp_context_t *)context;
    if (bsp == NULL || key == NULL) return BC2_HAL_ERROR_ARGUMENT;

    const esp_err_t erase_result = nvs_erase_key(bsp->nvs, key);
    if (erase_result == ESP_ERR_NVS_NOT_FOUND) return BC2_HAL_ERROR_NOT_FOUND;
    if (erase_result != ESP_OK) return BC2_HAL_ERROR_IO;
    if (nvs_commit(bsp->nvs) != ESP_OK) return BC2_HAL_ERROR_IO;
    return BC2_HAL_OK;
}

static bc2_hal_result_t usb_send(void *context,
                                 const uint8_t *data,
                                 size_t data_size) {
    bsp_context_t *bsp = (bsp_context_t *)context;
    if (bsp == NULL || data == NULL || data_size == 0U)
        return BC2_HAL_ERROR_ARGUMENT;
    if (!bsp->usb_ready) return BC2_HAL_ERROR_UNAVAILABLE;

    const int written = usb_serial_jtag_write_bytes(data,
                                                    data_size,
                                                    BC2_USB_TX_TIMEOUT_TICKS);
    return written == (int)data_size ? BC2_HAL_OK : BC2_HAL_ERROR_IO;
}

static bc2_hal_result_t usb_receive(void *context,
                                    uint8_t *output,
                                    size_t output_capacity,
                                    size_t *output_size) {
    bsp_context_t *bsp = (bsp_context_t *)context;
    if (bsp == NULL || output == NULL || output_size == NULL || output_capacity == 0U)
        return BC2_HAL_ERROR_ARGUMENT;
    if (!bsp->usb_ready) return BC2_HAL_ERROR_UNAVAILABLE;

    const int received = usb_serial_jtag_read_bytes(output,
                                                    output_capacity,
                                                    BC2_USB_RX_TIMEOUT_TICKS);
    if (received < 0) return BC2_HAL_ERROR_IO;

    *output_size = (size_t)received;
    return received == 0 ? BC2_HAL_ERROR_NOT_FOUND : BC2_HAL_OK;
}

static esp_err_t initialize_storage(void) {
    esp_err_t result = nvs_flash_init();
    if (result == ESP_ERR_NVS_NO_FREE_PAGES ||
        result == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_RETURN_ON_ERROR(nvs_flash_erase(), TAG, "NVS erase failed");
        result = nvs_flash_init();
    }
    ESP_RETURN_ON_ERROR(result, TAG, "NVS initialization failed");
    return nvs_open("bc2", NVS_READWRITE, &g_context.nvs);
}

static esp_err_t initialize_usb(void) {
    usb_serial_jtag_driver_config_t config = {
        .tx_buffer_size = BC2_USB_TX_BUFFER_SIZE,
        .rx_buffer_size = BC2_USB_RX_BUFFER_SIZE,
    };

    const esp_err_t result = usb_serial_jtag_driver_install(&config);
    if (result == ESP_OK) g_context.usb_ready = true;
    return result;
}


static esp_err_t initialize_button(void) {
    const gpio_config_t config = {
        .pin_bit_mask = 1ULL << GPIO_NUM_0,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_RETURN_ON_ERROR(gpio_config(&config), TAG, "BOOT button init failed");
    g_context.button_previous_level = gpio_get_level(GPIO_NUM_0);
    g_context.button_last_change_ms = 0U;
    g_context.button_ready = true;
    return ESP_OK;
}

static esp_err_t initialize_display(void) {
    const esp_err_t result = bc2_display_adapter_init();
    if (result == ESP_OK) g_context.display_ready = true;
    return result;
}
esp_err_t bc2_waveshare_bsp_init(bc2_hal_t *hal) {
    if (hal == NULL) return ESP_ERR_INVALID_ARG;

    memset(&g_context, 0, sizeof(g_context));
    ESP_RETURN_ON_ERROR(initialize_storage(), TAG, "Storage unavailable");
    ESP_RETURN_ON_ERROR(initialize_usb(), TAG, "USB Serial/JTAG unavailable");
    ESP_RETURN_ON_ERROR(initialize_button(), TAG, "BOOT button unavailable");
    ESP_RETURN_ON_ERROR(initialize_display(), TAG, "E-paper unavailable");

    *hal = (bc2_hal_t){
        &g_context,
        display_present,
        button_poll,
        time_now_ms,
        random_fill,
        storage_read,
        storage_write,
        storage_remove,
        usb_send,
        usb_receive,
    };

    ESP_LOGI(TAG, "Waveshare V2 e-paper and BOOT confirm button enabled");
    return ESP_OK;
}

bc2_board_revision_t bc2_waveshare_board_revision(void) {
    return BC2_BOARD_REVISION_V2;
}

bool bc2_waveshare_display_ready(void) {
    return g_context.display_ready;
}

bool bc2_waveshare_buttons_ready(void) {
    return g_context.button_ready;
}
