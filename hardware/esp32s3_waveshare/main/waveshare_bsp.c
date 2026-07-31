#include "waveshare_bsp.h"

#include "driver/usb_serial_jtag.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "nvs.h"
#include "nvs_flash.h"

#include <string.h>

#define BC2_USB_RX_BUFFER_SIZE 4096U
#define BC2_USB_TX_BUFFER_SIZE 4096U
#define BC2_USB_IO_TIMEOUT_TICKS 0U

static const char *TAG = "bc2_bsp";

typedef struct {
    nvs_handle_t nvs;
    bool usb_ready;
} bsp_context_t;

static bsp_context_t g_context;

static bc2_hal_result_t display_present(void *context,
                                        const bc2_display_frame_t *frame) {
    (void)context;
    if (frame == NULL) return BC2_HAL_ERROR_ARGUMENT;

    /* GPIO panel transfer stays blocked until the physical board revision is known. */
    ESP_LOGI(TAG,
             "EPAPER[%s] %s | %s | %s",
             frame->require_full_refresh ? "FULL" : "PART",
             frame->title,
             frame->body,
             frame->footer);
    return BC2_HAL_OK;
}

static bc2_hal_result_t button_poll(void *context,
                                    bc2_button_event_t *event) {
    (void)context;
    if (event == NULL) return BC2_HAL_ERROR_ARGUMENT;

    memset(event, 0, sizeof(*event));
    return BC2_HAL_ERROR_UNAVAILABLE;
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
                                                    BC2_USB_IO_TIMEOUT_TICKS);
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
                                                    BC2_USB_IO_TIMEOUT_TICKS);
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
    const usb_serial_jtag_driver_config_t config = {
        .tx_buffer_size = BC2_USB_TX_BUFFER_SIZE,
        .rx_buffer_size = BC2_USB_RX_BUFFER_SIZE,
    };

    const esp_err_t result = usb_serial_jtag_driver_install(&config);
    if (result == ESP_OK) g_context.usb_ready = true;
    return result;
}

esp_err_t bc2_waveshare_bsp_init(bc2_hal_t *hal) {
    if (hal == NULL) return ESP_ERR_INVALID_ARG;

    memset(&g_context, 0, sizeof(g_context));
    ESP_RETURN_ON_ERROR(initialize_storage(), TAG, "Storage unavailable");
    ESP_RETURN_ON_ERROR(initialize_usb(), TAG, "USB Serial/JTAG unavailable");

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

    ESP_LOGW(TAG, "Physical display and buttons remain disabled until board revision confirmation");
    return ESP_OK;
}

bc2_board_revision_t bc2_waveshare_board_revision(void) {
    return BC2_BOARD_REVISION_UNKNOWN;
}

bool bc2_waveshare_display_ready(void) {
    return false;
}

bool bc2_waveshare_buttons_ready(void) {
    return false;
}
