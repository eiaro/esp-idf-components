/*
 * SPDX-FileCopyrightText: 2026 Ronny Eia <3652665+eiaro@users.noreply.github.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "txe81xx.h"

#include <stdlib.h>
#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

static const char *TAG = "txe81xx";

#if CONFIG_TXE81XX_VERBOSE_LOG
#define TXE_LOGV(...) ESP_LOGI(TAG, __VA_ARGS__)
#else
#define TXE_LOGV(...) do { } while (0)
#endif

/* Feature addresses (F[5:0]) */
#define TXE_FEAT_SCRATCH        0x00
#define TXE_FEAT_DEVICE_ID      0x01
#define TXE_FEAT_INPUT_PORT     0x02
#define TXE_FEAT_OUTPUT_PORT    0x03
#define TXE_FEAT_DIRECTION      0x04
#define TXE_FEAT_PP_OD          0x06
#define TXE_FEAT_PULL_EN        0x08
#define TXE_FEAT_PULL_SEL       0x09
#define TXE_FEAT_SMART_INT      0x0B
#define TXE_FEAT_INT_MASK       0x0C
#define TXE_FEAT_GLITCH_EN      0x0D
#define TXE_FEAT_INT_FLAG       0x0E
#define TXE_FEAT_INT_PORT_STAT  0x0F
#define TXE_FEAT_SOFT_RESET     0x1A
#define TXE_FEAT_FAULT_STATUS   0x19

#define TXE_SOFT_RESET_DATA     0x02

struct txe81xx_t {
    spi_device_handle_t spi;
    bool owns_spi_device;
    bool use_polling;
    txe81xx_chip_t chip;
    txe81xx_xfer24_fn xfer24;
    void *xfer24_ctx;

#if CONFIG_TXE81XX_USE_MUTEX
    SemaphoreHandle_t mutex;
#endif

    // IRQ support
    bool irq_installed;
    gpio_num_t int_gpio;
    bool int_active_low;
    TaskHandle_t irq_task;
    txe81xx_irq_cb_t irq_cb;
    void *irq_cb_ctx;

    // which ports we service (bit0=port0, bit1=port1, bit2=port2)
    uint8_t irq_ports_enabled;
};

static inline int txe_max_ports(txe81xx_chip_t chip)
{
    return (chip == TXE_CHIP_8124) ? 3 : 2;
}

static inline bool txe_port_valid(const txe81xx_t *dev, txe81xx_port_t port)
{
    return dev && ((int)port >= 0) && ((int)port < txe_max_ports(dev->chip));
}

static inline bool txe_chip_valid(txe81xx_chip_t chip)
{
    return chip == TXE_CHIP_8116 || chip == TXE_CHIP_8124;
}

static inline bool txe_dev_ready(const txe81xx_t *dev)
{
    return dev && (dev->spi || dev->xfer24);
}

#if CONFIG_TXE81XX_USE_MUTEX
static inline void lock_dev(txe81xx_t *dev)
{
    if (dev && dev->mutex) {
        xSemaphoreTake(dev->mutex, portMAX_DELAY);
    }
}
static inline void unlock_dev(txe81xx_t *dev)
{
    if (dev && dev->mutex) {
        xSemaphoreGive(dev->mutex);
    }
}
#else
static inline void lock_dev(txe81xx_t *dev)
{
    (void)dev;
}
static inline void unlock_dev(txe81xx_t *dev)
{
    (void)dev;
}
#endif

/* Byte0: RW, 0, F[5:0] */
static inline uint8_t txe_cmd_byte(uint8_t feature, bool read)
{
    return (read ? 0x80u : 0x00u) | (feature & 0x3Fu);
}

/* Byte1: X PPP X X X M ; PPP bits6..4, M bit0 */
static inline uint8_t txe_portsel_byte(txe81xx_port_t port, bool multi_port)
{
    uint8_t b = (uint8_t)(((uint8_t)port & 0x07u) << 4);
    if (multi_port) {
        b |= 0x01u;
    }
    return b;
}

static esp_err_t txe_xfer24(txe81xx_t *dev,
                            uint8_t b0, uint8_t b1, uint8_t b2,
                            uint16_t *out_status16, uint8_t *out_data8)
{
    if (!dev || (!dev->spi && !dev->xfer24)) {
        ESP_LOGE(TAG, "xfer24 called with device not ready");
        return ESP_ERR_INVALID_STATE;
    }

    uint8_t tx[3] = { b0, b1, b2 };
    uint8_t rx[3] = { 0 };

    esp_err_t err;
    if (dev->xfer24) {
        err = dev->xfer24(dev->xfer24_ctx, tx, rx);
    } else {
        spi_transaction_t t;
        memset(&t, 0, sizeof(t));
        t.length = 24;
        t.tx_buffer = tx;
        t.rx_buffer = rx;

        err = dev->use_polling ? spi_device_polling_transmit(dev->spi, &t)
              : spi_device_transmit(dev->spi, &t);
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "SPI transfer failed: %s", esp_err_to_name(err));
        return err;
    }

    uint16_t status = (uint16_t)(((uint16_t)rx[0] << 8) | rx[1]);
    uint8_t data8   = rx[2];

#if CONFIG_TXE81XX_STRICT_STATUS_CHECK
    uint8_t hdr = (uint8_t)((status >> 14) & 0x03u);
    uint8_t low = (uint8_t)(status & 0xFFu);
    if (hdr != 0x03u || low != 0x00u) {
        ESP_LOGW(TAG, "Invalid status: 0x%04X (hdr=%u low=0x%02X)", status, hdr, low);
        return ESP_ERR_INVALID_RESPONSE;
    }
#endif

    if (out_status16) {
        *out_status16 = status;
    }
    if (out_data8) {
        *out_data8 = data8;
    }
    TXE_LOGV("xfer24 ok cmd=0x%02X sel=0x%02X tx=0x%02X status=0x%04X rx=0x%02X", b0, b1, b2, status, data8);
    return ESP_OK;
}

static esp_err_t write_reg_raw(txe81xx_t *dev, uint8_t feature,
                               txe81xx_port_t port, bool multi_port,
                               uint8_t value)
{
    uint8_t b0 = txe_cmd_byte(feature, false);
    uint8_t b1 = txe_portsel_byte(port, multi_port);

    lock_dev(dev);
    esp_err_t err = txe_xfer24(dev, b0, b1, value, NULL, NULL);
    unlock_dev(dev);

    if (err == ESP_OK) {
        TXE_LOGV("WR feat=0x%02X port=%d value=0x%02X", feature, (int)port, value);
    }

    return err;
}

/* Read returns data in byte3 of the same 24-bit frame (after first 16 address bits). */
static esp_err_t read_reg_raw(txe81xx_t *dev, uint8_t feature,
                              txe81xx_port_t port, bool multi_port,
                              uint8_t *out_value)
{
    if (!out_value) {
        ESP_LOGE(TAG, "read_reg_raw out_value is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t b0 = txe_cmd_byte(feature, true);
    uint8_t b1 = txe_portsel_byte(port, multi_port);

    uint8_t data = 0;

    lock_dev(dev);
    esp_err_t err = txe_xfer24(dev, b0, b1, 0x00, NULL, &data);
    unlock_dev(dev);

    if (err != ESP_OK) {
        return err;
    }
    *out_value = data;
    TXE_LOGV("RD feat=0x%02X port=%d value=0x%02X", feature, (int)port, data);
    return ESP_OK;
}

static esp_err_t update_reg_raw(txe81xx_t *dev,
                                uint8_t feature,
                                txe81xx_port_t port,
                                uint8_t set_mask,
                                uint8_t clear_mask)
{
    uint8_t value = 0;
    esp_err_t err = read_reg_raw(dev, feature, port, false, &value);
    if (err != ESP_OK) {
        return err;
    }

    value |= set_mask;
    value &= (uint8_t)~clear_mask;

    return write_reg_raw(dev, feature, port, false, value);
}

static txe81xx_t *alloc_dev(void)
{
    txe81xx_t *dev = (txe81xx_t *)calloc(1, sizeof(*dev));
    if (!dev) {
        return NULL;
    }

#if CONFIG_TXE81XX_USE_MUTEX
    dev->mutex = xSemaphoreCreateMutex();
    if (!dev->mutex) {
        free(dev);
        return NULL;
    }
#endif
    return dev;
}

static void free_dev(txe81xx_t *dev)
{
    if (!dev) {
        return;
    }
#if CONFIG_TXE81XX_USE_MUTEX
    if (dev->mutex) {
        vSemaphoreDelete(dev->mutex);
    }
#endif
    free(dev);
}

/* ---- Public init/deinit ---- */
esp_err_t txe81xx_init_spi(const txe81xx_spi_config_t *cfg, txe81xx_handle_t *out)
{
    if (!cfg || !out) {
        ESP_LOGE(TAG, "init_spi invalid args (cfg/out)");
        return ESP_ERR_INVALID_ARG;
    }
    *out = NULL;
    if (cfg->mode > 3) {
        ESP_LOGE(TAG, "init_spi invalid SPI mode: %u", cfg->mode);
        return ESP_ERR_INVALID_ARG;
    }
    if (cfg->clock_hz <= 0) {
        ESP_LOGE(TAG, "init_spi invalid clock_hz: %d", cfg->clock_hz);
        return ESP_ERR_INVALID_ARG;
    }
    if (!GPIO_IS_VALID_OUTPUT_GPIO(cfg->cs_gpio)) {
        ESP_LOGE(TAG, "init_spi invalid cs_gpio: %d", cfg->cs_gpio);
        return ESP_ERR_INVALID_ARG;
    }
    if (!txe_chip_valid(cfg->chip)) {
        ESP_LOGE(TAG, "init_spi invalid chip enum: %d", (int)cfg->chip);
        return ESP_ERR_INVALID_ARG;
    }

    txe81xx_t *dev = alloc_dev();
    if (!dev) {
        return ESP_ERR_NO_MEM;
    }

    dev->owns_spi_device = true;
    dev->use_polling = cfg->use_polling;
    dev->chip = cfg->chip;
    dev->xfer24 = NULL;
    dev->xfer24_ctx = NULL;

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = cfg->clock_hz,
        .mode = cfg->mode,
        .spics_io_num = cfg->cs_gpio,
        .queue_size = (cfg->queue_size > 0) ? cfg->queue_size : 1,
        .cs_ena_pretrans = 2,
        .cs_ena_posttrans = 2,
    };

    esp_err_t err = spi_bus_add_device(cfg->host, &devcfg, &dev->spi);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "spi_bus_add_device failed: %s", esp_err_to_name(err));
        free_dev(dev);
        return err;
    }

    TXE_LOGV("init_spi ok host=%d cs=%d hz=%d mode=%u", (int)cfg->host, cfg->cs_gpio, cfg->clock_hz, cfg->mode);
    *out = dev;
    return ESP_OK;
}

esp_err_t txe81xx_init_handle(const txe81xx_handle_config_t *cfg, txe81xx_handle_t *out)
{
    if (!cfg || !out || (!cfg->spi && !cfg->xfer24)) {
        ESP_LOGE(TAG, "init_handle invalid args (cfg/out/spi_or_xfer24)");
        return ESP_ERR_INVALID_ARG;
    }
    *out = NULL;
    if (!txe_chip_valid(cfg->chip)) {
        ESP_LOGE(TAG, "init_handle invalid chip enum: %d", (int)cfg->chip);
        return ESP_ERR_INVALID_ARG;
    }

    txe81xx_t *dev = alloc_dev();
    if (!dev) {
        return ESP_ERR_NO_MEM;
    }

    dev->spi = cfg->spi;
    dev->owns_spi_device = false;
    dev->use_polling = cfg->use_polling;
    dev->chip = cfg->chip;
    dev->xfer24 = cfg->xfer24;
    dev->xfer24_ctx = cfg->xfer24_ctx;

    TXE_LOGV("init_handle ok chip=%d", (int)cfg->chip);
    *out = dev;
    return ESP_OK;
}

esp_err_t txe81xx_deinit(txe81xx_handle_t devh)
{
    if (!devh) {
        ESP_LOGE(TAG, "deinit called with NULL handle");
        return ESP_ERR_INVALID_ARG;
    }
    txe81xx_t *dev = devh;
    if (!txe_dev_ready(dev)) {
        ESP_LOGE(TAG, "deinit called on non-ready device");
        return ESP_ERR_INVALID_STATE;
    }

    (void)txe81xx_irq_uninstall(dev);

    lock_dev(dev);
    spi_device_handle_t spi = dev->spi;
    bool owns = dev->owns_spi_device;
    dev->spi = NULL;
    unlock_dev(dev);

    if (owns && spi) {
        (void)spi_bus_remove_device(spi);
    }

    TXE_LOGV("deinit complete");
    free_dev(dev);
    return ESP_OK;
}

/* ---- Generic feature access ---- */
esp_err_t txe81xx_write_feature8(txe81xx_handle_t devh, uint8_t feature, txe81xx_port_t port, uint8_t value)
{
    txe81xx_t *dev = devh;
    if (!txe_dev_ready(dev)) {
        return ESP_ERR_INVALID_STATE;
    }
    if (!txe_port_valid(dev, port)) {
        return ESP_ERR_INVALID_ARG;
    }
    return write_reg_raw(dev, feature, port, false, value);
}

esp_err_t txe81xx_read_feature8(txe81xx_handle_t devh, uint8_t feature, txe81xx_port_t port, uint8_t *out_value)
{
    txe81xx_t *dev = devh;
    if (!out_value) {
        return ESP_ERR_INVALID_ARG;
    }
    if (!txe_dev_ready(dev)) {
        return ESP_ERR_INVALID_STATE;
    }
    if (!txe_port_valid(dev, port)) {
        return ESP_ERR_INVALID_ARG;
    }
    return read_reg_raw(dev, feature, port, false, out_value);
}

/* ---- Convenience ---- */
esp_err_t txe81xx_software_reset(txe81xx_handle_t devh)
{
    txe81xx_t *dev = devh;
    if (!txe_dev_ready(dev)) {
        return ESP_ERR_INVALID_STATE;
    }
    return write_reg_raw(dev, TXE_FEAT_SOFT_RESET, TXE_PORT0, false, TXE_SOFT_RESET_DATA);
}

esp_err_t txe81xx_read_device_id(txe81xx_handle_t devh, uint8_t *out_id)
{
    txe81xx_t *dev = devh;
    if (!out_id) {
        return ESP_ERR_INVALID_ARG;
    }
    if (!txe_dev_ready(dev)) {
        return ESP_ERR_INVALID_STATE;
    }
    return read_reg_raw(dev, TXE_FEAT_DEVICE_ID, TXE_PORT0, false, out_id);
}

esp_err_t txe81xx_read_fault_status(txe81xx_handle_t devh, uint8_t *out_status)
{
    txe81xx_t *dev = devh;
    if (!out_status) {
        return ESP_ERR_INVALID_ARG;
    }
    if (!txe_dev_ready(dev)) {
        return ESP_ERR_INVALID_STATE;
    }
    return read_reg_raw(dev, TXE_FEAT_FAULT_STATUS, TXE_PORT0, false, out_status);
}

esp_err_t txe81xx_read_inputs(txe81xx_handle_t devh, txe81xx_port_t port, uint8_t *out_value)
{
    txe81xx_t *dev = devh;
    if (!out_value) {
        return ESP_ERR_INVALID_ARG;
    }
    if (!txe_dev_ready(dev)) {
        return ESP_ERR_INVALID_STATE;
    }
    if (!txe_port_valid(dev, port)) {
        return ESP_ERR_INVALID_ARG;
    }
    return read_reg_raw(dev, TXE_FEAT_INPUT_PORT, port, false, out_value);
}

esp_err_t txe81xx_read_outputs(txe81xx_handle_t devh, txe81xx_port_t port, uint8_t *out_value)
{
    txe81xx_t *dev = devh;
    if (!out_value) {
        return ESP_ERR_INVALID_ARG;
    }
    if (!txe_dev_ready(dev)) {
        return ESP_ERR_INVALID_STATE;
    }
    if (!txe_port_valid(dev, port)) {
        return ESP_ERR_INVALID_ARG;
    }
    return read_reg_raw(dev, TXE_FEAT_OUTPUT_PORT, port, false, out_value);
}

esp_err_t txe81xx_write_outputs(txe81xx_handle_t devh, txe81xx_port_t port, uint8_t value)
{
    txe81xx_t *dev = devh;
    if (!txe_dev_ready(dev)) {
        return ESP_ERR_INVALID_STATE;
    }
    if (!txe_port_valid(dev, port)) {
        return ESP_ERR_INVALID_ARG;
    }
    return write_reg_raw(dev, TXE_FEAT_OUTPUT_PORT, port, false, value);
}

esp_err_t txe81xx_set_direction(txe81xx_handle_t devh, txe81xx_port_t port, uint8_t dir_mask)
{
    txe81xx_t *dev = devh;
    if (!txe_dev_ready(dev)) {
        return ESP_ERR_INVALID_STATE;
    }
    if (!txe_port_valid(dev, port)) {
        return ESP_ERR_INVALID_ARG;
    }
    return write_reg_raw(dev, TXE_FEAT_DIRECTION, port, false, dir_mask);
}

esp_err_t txe81xx_update_direction(txe81xx_handle_t devh, txe81xx_port_t port, uint8_t set_mask, uint8_t clear_mask)
{
    txe81xx_t *dev = devh;
    if (!txe_dev_ready(dev)) {
        return ESP_ERR_INVALID_STATE;
    }
    if (!txe_port_valid(dev, port)) {
        return ESP_ERR_INVALID_ARG;
    }
    return update_reg_raw(dev, TXE_FEAT_DIRECTION, port, set_mask, clear_mask);
}

esp_err_t txe81xx_set_pushpull_od(txe81xx_handle_t devh, txe81xx_port_t port, uint8_t mask)
{
    txe81xx_t *dev = devh;
    if (!txe_dev_ready(dev)) {
        return ESP_ERR_INVALID_STATE;
    }
    if (!txe_port_valid(dev, port)) {
        return ESP_ERR_INVALID_ARG;
    }
    return write_reg_raw(dev, TXE_FEAT_PP_OD, port, false, mask);
}

esp_err_t txe81xx_update_pushpull_od(txe81xx_handle_t devh, txe81xx_port_t port, uint8_t set_mask, uint8_t clear_mask)
{
    txe81xx_t *dev = devh;
    if (!txe_dev_ready(dev)) {
        return ESP_ERR_INVALID_STATE;
    }
    if (!txe_port_valid(dev, port)) {
        return ESP_ERR_INVALID_ARG;
    }
    return update_reg_raw(dev, TXE_FEAT_PP_OD, port, set_mask, clear_mask);
}

esp_err_t txe81xx_set_pull_enable_mask(txe81xx_handle_t devh, txe81xx_port_t port, uint8_t enable_mask)
{
    txe81xx_t *dev = devh;
    if (!txe_dev_ready(dev)) {
        return ESP_ERR_INVALID_STATE;
    }
    if (!txe_port_valid(dev, port)) {
        return ESP_ERR_INVALID_ARG;
    }
    return write_reg_raw(dev, TXE_FEAT_PULL_EN, port, false, enable_mask);
}

esp_err_t txe81xx_set_pull_select_mask(txe81xx_handle_t devh, txe81xx_port_t port, uint8_t select_mask)
{
    txe81xx_t *dev = devh;
    if (!txe_dev_ready(dev)) {
        return ESP_ERR_INVALID_STATE;
    }
    if (!txe_port_valid(dev, port)) {
        return ESP_ERR_INVALID_ARG;
    }
    return write_reg_raw(dev, TXE_FEAT_PULL_SEL, port, false, select_mask);
}

esp_err_t txe81xx_update_pull_enable_mask(txe81xx_handle_t devh, txe81xx_port_t port, uint8_t set_mask, uint8_t clear_mask)
{
    txe81xx_t *dev = devh;
    if (!txe_dev_ready(dev)) {
        return ESP_ERR_INVALID_STATE;
    }
    if (!txe_port_valid(dev, port)) {
        return ESP_ERR_INVALID_ARG;
    }
    return update_reg_raw(dev, TXE_FEAT_PULL_EN, port, set_mask, clear_mask);
}

esp_err_t txe81xx_update_pull_select_mask(txe81xx_handle_t devh, txe81xx_port_t port, uint8_t set_mask, uint8_t clear_mask)
{
    txe81xx_t *dev = devh;
    if (!txe_dev_ready(dev)) {
        return ESP_ERR_INVALID_STATE;
    }
    if (!txe_port_valid(dev, port)) {
        return ESP_ERR_INVALID_ARG;
    }
    return update_reg_raw(dev, TXE_FEAT_PULL_SEL, port, set_mask, clear_mask);
}

/* ---- IRQ implementation ---- */

static void IRAM_ATTR txe_gpio_isr(void *arg)
{
    txe81xx_t *dev = (txe81xx_t *)arg;
    if (!dev || !dev->irq_task) {
        return;
    }

    BaseType_t hp = pdFALSE;
    vTaskNotifyGiveFromISR(dev->irq_task, &hp);
    if (hp) {
        portYIELD_FROM_ISR();
    }
}

static void txe_irq_task_fn(void *arg)
{
    txe81xx_t *dev = (txe81xx_t *)arg;

    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // Drain bursts
        while (ulTaskNotifyTake(pdTRUE, 0) > 0) {}

        if (!dev->irq_cb) {
            continue;
        }

        // For each enabled port: read flags + inputs (flags read may clear depending on smart setting)
        for (int p = 0; p < txe_max_ports(dev->chip); p++) {
            uint8_t port_bit = (uint8_t)(1u << p);
            if ((dev->irq_ports_enabled & port_bit) == 0) {
                continue;
            }

            txe81xx_port_t port = (txe81xx_port_t)p;

            uint8_t flags = 0;
            uint8_t in = 0;

            // Read interrupt flags first (0x0E)
            if (read_reg_raw(dev, TXE_FEAT_INT_FLAG, port, false, &flags) != ESP_OK) {
                continue;
            }

            // If no flags, skip. (Some designs assert INT for port summary; safe to skip)
            if (flags == 0) {
                continue;
            }

            // Read inputs
            if (read_reg_raw(dev, TXE_FEAT_INPUT_PORT, port, false, &in) != ESP_OK) {
                continue;
            }

            dev->irq_cb(port, flags, in, dev->irq_cb_ctx);
        }
    }
}

esp_err_t txe81xx_irq_install(txe81xx_handle_t devh, const txe81xx_irq_config_t *cfg)
{
    txe81xx_t *dev = devh;
    if (!cfg) {
        ESP_LOGE(TAG, "irq_install cfg is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    if (!txe_dev_ready(dev)) {
        ESP_LOGE(TAG, "irq_install device not ready");
        return ESP_ERR_INVALID_STATE;
    }
    if (!cfg->cb) {
        ESP_LOGE(TAG, "irq_install callback is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    if (!GPIO_IS_VALID_GPIO(cfg->int_gpio)) {
        ESP_LOGE(TAG, "irq_install invalid int_gpio: %d", (int)cfg->int_gpio);
        return ESP_ERR_INVALID_ARG;
    }
    if (dev->irq_installed) {
        ESP_LOGE(TAG, "irq_install already installed");
        return ESP_ERR_INVALID_STATE;
    }

    dev->int_gpio = cfg->int_gpio;
    dev->int_active_low = cfg->active_low;
    dev->irq_cb = cfg->cb;
    dev->irq_cb_ctx = cfg->user_ctx;

    // Configure ESP GPIO as input with interrupt
    gpio_config_t io = {
        .pin_bit_mask = 1ULL << cfg->int_gpio,
                             .mode = GPIO_MODE_INPUT,

                             // You have an external pull-up on the INT line, so disable internal pulls:
                             .pull_up_en = GPIO_PULLUP_DISABLE,
                             .pull_down_en = GPIO_PULLDOWN_DISABLE,

                             // INT is inverted/active-low => interrupt on falling edge
                             .intr_type = cfg->active_low ? GPIO_INTR_NEGEDGE : GPIO_INTR_POSEDGE,
    };
    esp_err_t err = gpio_config(&io);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "gpio_config failed: %s", esp_err_to_name(err));
        return err;
    }

    // Install ISR service (safe to call multiple times; ESP-IDF returns OK if already installed in some versions,
    // but to be robust we ignore ESP_ERR_INVALID_STATE).
    err = gpio_install_isr_service(0);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "gpio_install_isr_service failed: %s", esp_err_to_name(err));
        return err;
    }

    uint32_t stack = cfg->task_stack ? cfg->task_stack : 3072;
    UBaseType_t prio = cfg->task_prio ? cfg->task_prio : 10;

    // Start task
    if (cfg->task_core >= 0) {
        if (xTaskCreatePinnedToCore(txe_irq_task_fn, "txe_irq", stack, dev, prio, &dev->irq_task, cfg->task_core) != pdPASS) {
            ESP_LOGE(TAG, "irq task create pinned failed");
            return ESP_ERR_NO_MEM;
        }
    } else {
        if (xTaskCreate(txe_irq_task_fn, "txe_irq", stack, dev, prio, &dev->irq_task) != pdPASS) {
            ESP_LOGE(TAG, "irq task create failed");
            return ESP_ERR_NO_MEM;
        }
    }

    // Hook ISR handler
    err = gpio_isr_handler_add(cfg->int_gpio, txe_gpio_isr, dev);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "gpio_isr_handler_add failed: %s", esp_err_to_name(err));
        if (dev->irq_task) {
            TaskHandle_t t = dev->irq_task;
            dev->irq_task = NULL;
            vTaskDelete(t);
        }
        return err;
    }

    TXE_LOGV("irq_install ok gpio=%d active_low=%d", (int)cfg->int_gpio, cfg->active_low ? 1 : 0);
    dev->irq_installed = true;
    return ESP_OK;
}

esp_err_t txe81xx_irq_uninstall(txe81xx_handle_t devh)
{
    txe81xx_t *dev = devh;
    if (!txe_dev_ready(dev)) {
        ESP_LOGE(TAG, "irq_uninstall device not ready");
        return ESP_ERR_INVALID_STATE;
    }
    if (!dev->irq_installed) {
        return ESP_OK;
    }

    (void)gpio_isr_handler_remove(dev->int_gpio);

    if (dev->irq_task) {
        TaskHandle_t t = dev->irq_task;
        dev->irq_task = NULL;
        vTaskDelete(t);
    }

    TXE_LOGV("irq_uninstall complete gpio=%d", (int)dev->int_gpio);
    dev->irq_installed = false;
    dev->irq_ports_enabled = 0;
    dev->irq_cb = NULL;
    dev->irq_cb_ctx = NULL;
    return ESP_OK;
}

esp_err_t txe81xx_irq_configure_port(txe81xx_handle_t devh,
                                     txe81xx_port_t port,
                                     bool smart_enable,
                                     uint8_t mask)
{
    txe81xx_t *dev = devh;
    if (!txe_dev_ready(dev)) {
        ESP_LOGE(TAG, "irq_configure_port device not ready");
        return ESP_ERR_INVALID_STATE;
    }
    if (!txe_port_valid(dev, port)) {
        ESP_LOGE(TAG, "irq_configure_port invalid port=%d", (int)port);
        return ESP_ERR_INVALID_ARG;
    }

    // Smart interrupt register is port-level bits: 0=enabled, 1=disabled
    // We'll read-modify-write port bits using port0 addressing (port selection not meaningful here; use TXE_PORT0).
    uint8_t smart = 0x00;
    esp_err_t err = read_reg_raw(dev, TXE_FEAT_SMART_INT, TXE_PORT0, false, &smart);
    if (err != ESP_OK) {
        return err;
    }

    uint8_t bit = (uint8_t)(1u << (uint8_t)port);
    if (smart_enable) {
        smart &= (uint8_t)~bit; // 0 => enabled
    } else {
        smart |= bit;           // 1 => disabled
    }
    err = write_reg_raw(dev, TXE_FEAT_SMART_INT, TXE_PORT0, false, smart);
    if (err != ESP_OK) {
        return err;
    }

    // Interrupt mask per port (0=enabled, 1=masked)
    err = write_reg_raw(dev, TXE_FEAT_INT_MASK, port, false, mask);
    if (err != ESP_OK) {
        return err;
    }

    // Mark port enabled for servicing in IRQ task (so we read its flags)
    dev->irq_ports_enabled |= bit;

    TXE_LOGV("irq_configure_port ok port=%d smart=%d mask=0x%02X", (int)port, smart_enable ? 1 : 0, mask);

    return ESP_OK;
}

esp_err_t txe81xx_irq_enable_pins(txe81xx_handle_t devh,
                                  txe81xx_port_t port,
                                  uint8_t pins_mask)
{
    txe81xx_t *dev = devh;
    if (!txe_dev_ready(dev)) {
        ESP_LOGE(TAG, "irq_enable_pins device not ready");
        return ESP_ERR_INVALID_STATE;
    }
    if (!txe_port_valid(dev, port)) {
        ESP_LOGE(TAG, "irq_enable_pins invalid port=%d", (int)port);
        return ESP_ERR_INVALID_ARG;
    }

    // Read current mask, then set enabled pins to 0
    uint8_t cur = 0xFF;
    esp_err_t err = read_reg_raw(dev, TXE_FEAT_INT_MASK, port, false, &cur);
    if (err != ESP_OK) {
        return err;
    }

    // pins_mask=1 => enable => mask bit must become 0
    cur &= (uint8_t)~pins_mask;

    err = write_reg_raw(dev, TXE_FEAT_INT_MASK, port, false, cur);
    if (err != ESP_OK) {
        return err;
    }

    dev->irq_ports_enabled |= (uint8_t)(1u << (uint8_t)port);
    TXE_LOGV("irq_enable_pins ok port=%d pins=0x%02X new_mask=0x%02X", (int)port, pins_mask, cur);
    return ESP_OK;
}

esp_err_t txe81xx_glitch_filter_enable(txe81xx_handle_t devh,
                                       txe81xx_port_t port,
                                       uint8_t pins_mask)
{
    txe81xx_t *dev = devh;
    if (!txe_dev_ready(dev)) {
        ESP_LOGE(TAG, "glitch_filter_enable device not ready");
        return ESP_ERR_INVALID_STATE;
    }
    if (!txe_port_valid(dev, port)) {
        ESP_LOGE(TAG, "glitch_filter_enable invalid port=%d", (int)port);
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t cur = 0;
    esp_err_t err = read_reg_raw(dev, TXE_FEAT_GLITCH_EN, port, false, &cur);
    if (err != ESP_OK) {
        return err;
    }
    cur |= pins_mask;
    err = write_reg_raw(dev, TXE_FEAT_GLITCH_EN, port, false, cur);
    if (err == ESP_OK) {
        TXE_LOGV("glitch_filter_enable ok port=%d pins=0x%02X reg=0x%02X", (int)port, pins_mask, cur);
    }
    return err;
}

esp_err_t txe81xx_update_glitch_filter(txe81xx_handle_t devh,
                                       txe81xx_port_t port,
                                       uint8_t set_mask,
                                       uint8_t clear_mask)
{
    txe81xx_t *dev = devh;
    if (!txe_dev_ready(dev)) {
        ESP_LOGE(TAG, "update_glitch_filter device not ready");
        return ESP_ERR_INVALID_STATE;
    }
    if (!txe_port_valid(dev, port)) {
        ESP_LOGE(TAG, "update_glitch_filter invalid port=%d", (int)port);
        return ESP_ERR_INVALID_ARG;
    }
    return update_reg_raw(dev, TXE_FEAT_GLITCH_EN, port, set_mask, clear_mask);
}

esp_err_t txe81xx_irq_read_flags(txe81xx_handle_t devh, txe81xx_port_t port, uint8_t *out_flags)
{
    txe81xx_t *dev = devh;
    if (!out_flags) {
        return ESP_ERR_INVALID_ARG;
    }
    if (!txe_dev_ready(dev)) {
        return ESP_ERR_INVALID_STATE;
    }
    if (!txe_port_valid(dev, port)) {
        return ESP_ERR_INVALID_ARG;
    }
    return read_reg_raw(dev, TXE_FEAT_INT_FLAG, port, false, out_flags);
}

esp_err_t txe81xx_irq_read_port_status(txe81xx_handle_t devh, uint8_t *out_status)
{
    txe81xx_t *dev = devh;
    if (!out_status) {
        return ESP_ERR_INVALID_ARG;
    }
    if (!txe_dev_ready(dev)) {
        return ESP_ERR_INVALID_STATE;
    }
    return read_reg_raw(dev, TXE_FEAT_INT_PORT_STAT, TXE_PORT0, false, out_status);
}

esp_err_t txe81xx_irq_clear_pending(txe81xx_handle_t devh)
{
    txe81xx_t *dev = devh;
    if (!txe_dev_ready(dev)) {
        return ESP_ERR_INVALID_STATE;
    }

    uint8_t dummy = 0;
    esp_err_t err = read_reg_raw(dev, TXE_FEAT_INT_FLAG, TXE_PORT0, false, &dummy);
    if (err != ESP_OK) {
        return err;
    }

    if (txe_max_ports(dev->chip) > 1) {
        err = read_reg_raw(dev, TXE_FEAT_INT_FLAG, TXE_PORT1, false, &dummy);
        if (err != ESP_OK) {
            return err;
        }
    }

    if (txe_max_ports(dev->chip) > 2) {
        err = read_reg_raw(dev, TXE_FEAT_INT_FLAG, TXE_PORT2, false, &dummy);
        if (err != ESP_OK) {
            return err;
        }
    }

    return read_reg_raw(dev, TXE_FEAT_INT_PORT_STAT, TXE_PORT0, false, &dummy);
}
