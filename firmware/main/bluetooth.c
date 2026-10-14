#include <bluetooth.h>
#include <stdio.h>
#include "esp_mac.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <exercise.h>

static const char *TAG = "BLE";

void ble_send_data(void *data, size_t size) {
    ble_uart_tx(data, size);
}

static void ble_uart_on_rx(const uint8_t *data, size_t len){
    ESP_LOGI(TAG, "rx len: %u bytes", (unsigned) len);

    if (data == NULL || len == 0)
        return;

    ESP_LOG_BUFFER_HEX(TAG, data, len);
    printf("%s", data);

    char buf[64];     
    size_t copy_len = (len < sizeof(buf) - 1) ? len : sizeof(buf) - 1;
    memcpy(buf, data, copy_len);
    buf[copy_len] = '\0';

    if (strcmp(buf, "start") == 0 || strcmp(buf, "stop") == 0) {         
        toggleExercise();          
        ble_send_data((uint8_t *) buf, strlen(buf));
    }
}

void bluetooth_init() {
    uint8_t mac[6] = {0};
    esp_err_t mac_err = esp_read_mac(mac, ESP_MAC_BT);

    if (mac_err != ESP_OK)
        ESP_LOGW(TAG, "esp_read_mac(BT) failed (%s); device name suffix will be 0000", esp_err_to_name(mac_err));

    char name[24];
    snprintf(name, sizeof(name), "%s-%02X%02X", CONFIG_BLE_UART_DEVICE_NAME_PREFIX, mac[4], mac[5]);

    ESP_ERROR_CHECK(ble_uart_install(&(ble_uart_config_t){
        .encrypted      = true,
        .device_name    = name,
        .ble_uart_on_rx = ble_uart_on_rx,
    }));

    ESP_ERROR_CHECK(ble_uart_open());
}