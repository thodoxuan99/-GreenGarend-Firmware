#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_smartconfig.h"
#include "smart_config.h"
#include "protocol_examples_common.h"
#include "mqtts.h"

bool wifi_connected = false;
void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    initialise_smartconfig_wifi();
    // mqtt_app_start();
}