/*
 * SPDX-FileCopyrightText: 2026 Ronny Eia <3652665+eiaro@users.noreply.github.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "txe81xx.h"

#define TXE81XX_VIRTUAL_BLINK_MASK 0x01

static const char *TAG = "txe81xx_virtual_blinky";

typedef struct {
    uint8_t regs[0x40][3];
} txe81xx_virtual_dev_t;

static esp_err_t txe81xx_virtual_xfer24(void *user_ctx, const uint8_t tx[3], uint8_t rx[3])
{
    if (!user_ctx || !tx || !rx) {
        return ESP_ERR_INVALID_ARG;
    }

    txe81xx_virtual_dev_t *virt = (txe81xx_virtual_dev_t *)user_ctx;
    const bool read = (tx[0] & 0x80u) != 0;
    const uint8_t feature = tx[0] & 0x3Fu;
    const uint8_t port = (uint8_t)((tx[1] >> 4) & 0x07u);

    if (feature >= 0x40u || port >= 3u) {
        return ESP_ERR_INVALID_ARG;
    }

    if (!read) {
        virt->regs[feature][port] = tx[2];
    }

    rx[0] = 0xC0;
    rx[1] = 0x00;
    if (read && feature == 0x01u) {
        rx[2] = 0x16;
    } else {
        rx[2] = virt->regs[feature][port];
    }
    return ESP_OK;
}

void app_main(void)
{
    txe81xx_virtual_dev_t virtual_dev = { 0 };
    txe81xx_handle_config_t txe_cfg = {
        .spi = NULL,
        .use_polling = true,
        .chip = TXE_CHIP_8116,
        .xfer24 = txe81xx_virtual_xfer24,
        .xfer24_ctx = &virtual_dev,
    };

    txe81xx_handle_t dev = NULL;
    ESP_ERROR_CHECK(txe81xx_init_handle(&txe_cfg, &dev));

    ESP_LOGI(TAG, "TXE81XX virtual mode enabled");

    uint8_t dev_id = 0;
    ESP_ERROR_CHECK(txe81xx_read_device_id(dev, &dev_id));
    ESP_LOGI(TAG, "TXE81XX device id: 0x%02" PRIX8, dev_id);

    ESP_ERROR_CHECK(txe81xx_set_direction(dev, TXE_PORT0, TXE81XX_VIRTUAL_BLINK_MASK));

    bool output_enabled = false;
    while (true) {
        output_enabled = !output_enabled;
        const uint8_t out = output_enabled ? TXE81XX_VIRTUAL_BLINK_MASK : 0x00;
        ESP_ERROR_CHECK(txe81xx_write_outputs(dev, TXE_PORT0, out));
        ESP_LOGI(TAG, "PORT0 output=0x%02" PRIX8, out);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
