/*
 * SPDX-FileCopyrightText: 2026 Ronny Eia <3652665+eiaro@users.noreply.github.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

#include "driver/spi_common.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "txe81xx.h"

#ifndef TXE81XX_EXAMPLE_SPI_HOST
#define TXE81XX_EXAMPLE_SPI_HOST SPI2_HOST
#endif

#ifndef TXE81XX_EXAMPLE_PIN_SCLK
#define TXE81XX_EXAMPLE_PIN_SCLK GPIO_NUM_6
#endif

#ifndef TXE81XX_EXAMPLE_PIN_MOSI
#define TXE81XX_EXAMPLE_PIN_MOSI GPIO_NUM_7
#endif

#ifndef TXE81XX_EXAMPLE_PIN_MISO
#define TXE81XX_EXAMPLE_PIN_MISO GPIO_NUM_2
#endif

#ifndef TXE81XX_EXAMPLE_PIN_CS
#define TXE81XX_EXAMPLE_PIN_CS GPIO_NUM_10
#endif

#ifndef TXE81XX_EXAMPLE_BLINK_MASK
#define TXE81XX_EXAMPLE_BLINK_MASK 0x01
#endif

static const char *TAG = "txe81xx_blinky";

void app_main(void)
{
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = TXE81XX_EXAMPLE_PIN_MOSI,
        .miso_io_num = TXE81XX_EXAMPLE_PIN_MISO,
        .sclk_io_num = TXE81XX_EXAMPLE_PIN_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0,
        .flags = SPICOMMON_BUSFLAG_MASTER,
        .isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO,
        .intr_flags = 0,
    };

    ESP_ERROR_CHECK(spi_bus_initialize(TXE81XX_EXAMPLE_SPI_HOST, &bus_cfg, SPI_DMA_CH_AUTO));

    txe81xx_spi_config_t txe_cfg = {
        .host = TXE81XX_EXAMPLE_SPI_HOST,
        .cs_gpio = TXE81XX_EXAMPLE_PIN_CS,
        .clock_hz = 1000000,
        .mode = 0,
        .queue_size = 1,
        .use_polling = true,
        .chip = TXE_CHIP_8116,
    };

    txe81xx_handle_t dev = NULL;
    ESP_ERROR_CHECK(txe81xx_init_spi(&txe_cfg, &dev));

    uint8_t dev_id = 0;
    ESP_ERROR_CHECK(txe81xx_read_device_id(dev, &dev_id));
    ESP_LOGI(TAG, "TXE81XX device id: 0x%02" PRIX8, dev_id);

    ESP_ERROR_CHECK(txe81xx_set_direction(dev, TXE_PORT0, TXE81XX_EXAMPLE_BLINK_MASK));

    bool on = false;
    while (true) {
        on = !on;
        const uint8_t out = on ? TXE81XX_EXAMPLE_BLINK_MASK : 0x00;
        ESP_ERROR_CHECK(txe81xx_write_outputs(dev, TXE_PORT0, out));
        ESP_LOGI(TAG, "PORT0 output=0x%02" PRIX8, out);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
