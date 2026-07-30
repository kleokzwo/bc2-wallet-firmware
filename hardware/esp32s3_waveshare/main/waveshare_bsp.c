#include "waveshare_bsp.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <string.h>
static const char *TAG="bc2_bsp";
typedef struct { nvs_handle_t nvs; } bsp_context_t;
static bsp_context_t g_ctx;
static bc2_hal_result_t display_present(void *ctx, const bc2_display_frame_t *frame) {
    (void)ctx;
    /* SECURITY: Hardware panel transfer is intentionally disabled until V1/V2 is confirmed. */
    ESP_LOGI(TAG,"EPAPER[%s] %s | %s", frame->require_full_refresh?"FULL":"PART", frame->title, frame->body);
    return BC2_HAL_OK;
}
static bc2_hal_result_t button_poll(void *ctx, bc2_button_event_t *event) {
    (void)ctx; if (event==NULL) return BC2_HAL_ERROR_ARGUMENT;
    memset(event,0,sizeof(*event)); return BC2_HAL_ERROR_UNAVAILABLE;
}
static uint64_t time_now_ms(void *ctx) { (void)ctx; return (uint64_t)(esp_timer_get_time()/1000); }
static bc2_hal_result_t random_fill(void *ctx, uint8_t *out, size_t size) {
    (void)ctx; if(out==NULL) return BC2_HAL_ERROR_ARGUMENT;
    esp_fill_random(out,size); return BC2_HAL_OK;
}
static bc2_hal_result_t storage_read(void *ctx,const char *key,uint8_t *out,size_t cap,size_t *out_size){
    bsp_context_t *b=(bsp_context_t*)ctx; if(!b||!key||!out_size) return BC2_HAL_ERROR_ARGUMENT;
    size_t required=0; esp_err_t e=nvs_get_blob(b->nvs,key,NULL,&required);
    if(e==ESP_ERR_NVS_NOT_FOUND) return BC2_HAL_ERROR_NOT_FOUND;
    if(e!=ESP_OK) return BC2_HAL_ERROR_IO; if(required>cap) return BC2_HAL_ERROR_LIMIT;
    e=nvs_get_blob(b->nvs,key,out,&required); if(e!=ESP_OK) return BC2_HAL_ERROR_IO; *out_size=required; return BC2_HAL_OK;
}
static bc2_hal_result_t storage_write(void *ctx,const char *key,const uint8_t *data,size_t size){
    bsp_context_t *b=(bsp_context_t*)ctx; if(!b||!key||(size&& !data)) return BC2_HAL_ERROR_ARGUMENT;
    if(nvs_set_blob(b->nvs,key,data,size)!=ESP_OK||nvs_commit(b->nvs)!=ESP_OK) return BC2_HAL_ERROR_IO; return BC2_HAL_OK;
}
static bc2_hal_result_t storage_remove(void *ctx,const char *key){
    bsp_context_t *b=(bsp_context_t*)ctx; if(!b||!key) return BC2_HAL_ERROR_ARGUMENT;
    esp_err_t e=nvs_erase_key(b->nvs,key); if(e==ESP_ERR_NVS_NOT_FOUND) return BC2_HAL_ERROR_NOT_FOUND;
    if(e!=ESP_OK||nvs_commit(b->nvs)!=ESP_OK) return BC2_HAL_ERROR_IO; return BC2_HAL_OK;
}
static bc2_hal_result_t usb_send(void *ctx,const uint8_t *data,size_t size){(void)ctx;(void)data;(void)size;return BC2_HAL_ERROR_UNAVAILABLE;}
static bc2_hal_result_t usb_receive(void *ctx,uint8_t *out,size_t cap,size_t *size){(void)ctx;(void)out;(void)cap;if(size)*size=0;return BC2_HAL_ERROR_UNAVAILABLE;}
esp_err_t bc2_waveshare_bsp_init(bc2_hal_t *hal){
    if(!hal) return ESP_ERR_INVALID_ARG; esp_err_t e=nvs_flash_init();
    if(e==ESP_ERR_NVS_NO_FREE_PAGES||e==ESP_ERR_NVS_NEW_VERSION_FOUND){ESP_ERROR_CHECK(nvs_flash_erase());e=nvs_flash_init();}
    if(e!=ESP_OK) return e; e=nvs_open("bc2",NVS_READWRITE,&g_ctx.nvs); if(e!=ESP_OK) return e;
    *hal=(bc2_hal_t){&g_ctx,display_present,button_poll,time_now_ms,random_fill,storage_read,storage_write,storage_remove,usb_send,usb_receive};
    ESP_LOGW(TAG,"Board revision must be confirmed before enabling GPIO display/button drivers"); return ESP_OK;
}
bc2_board_revision_t bc2_waveshare_board_revision(void){return BC2_BOARD_REVISION_UNKNOWN;}
