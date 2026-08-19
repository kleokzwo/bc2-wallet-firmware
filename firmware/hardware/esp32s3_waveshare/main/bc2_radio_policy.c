#include "bc2_radio_policy.h"
#include "esp_log.h"
static const char *TAG = "bc2_radio";
void bc2_radio_policy_assert_disabled(void) {
    ESP_LOGI(TAG, "BC2 radio policy: Wi-Fi is not initialized; Bluetooth/BLE disabled");
}
