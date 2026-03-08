/*
 * SPDX-FileCopyrightText: 2026 Ronny Eia <3652665+eiaro@users.noreply.github.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "esp_err.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    TXE_CHIP_8116 = 0,
    TXE_CHIP_8124 = 1,
} txe81xx_chip_t;

typedef enum {
    TXE_PORT0 = 0,
    TXE_PORT1 = 1,
    TXE_PORT2 = 2, // not valid for TXE8116
} txe81xx_port_t;

typedef struct txe81xx_t txe81xx_t;
typedef txe81xx_t *txe81xx_handle_t;

typedef struct {
    spi_host_device_t host;
    int cs_gpio;
    int clock_hz;
    uint8_t mode;       // datasheet: 0
    int queue_size;
    bool use_polling;
    txe81xx_chip_t chip;
} txe81xx_spi_config_t;

typedef struct {
    spi_device_handle_t spi; // borrowed
    bool use_polling;
    txe81xx_chip_t chip;
} txe81xx_handle_config_t;

/* ---- Core init/deinit ---- */
esp_err_t txe81xx_init_spi(const txe81xx_spi_config_t *cfg, txe81xx_handle_t *out);
esp_err_t txe81xx_init_handle(const txe81xx_handle_config_t *cfg, txe81xx_handle_t *out);
esp_err_t txe81xx_deinit(txe81xx_handle_t dev);

/* ---- Generic register access (8-bit, port-selected) ---- */
esp_err_t txe81xx_write_feature8(txe81xx_handle_t dev, uint8_t feature, txe81xx_port_t port, uint8_t value);
esp_err_t txe81xx_read_feature8(txe81xx_handle_t dev, uint8_t feature, txe81xx_port_t port, uint8_t *out_value);

/* ---- Convenience I/O ---- */
esp_err_t txe81xx_software_reset(txe81xx_handle_t dev);
esp_err_t txe81xx_read_device_id(txe81xx_handle_t dev, uint8_t *out_id);
esp_err_t txe81xx_read_fault_status(txe81xx_handle_t dev, uint8_t *out_status);

esp_err_t txe81xx_read_inputs(txe81xx_handle_t dev, txe81xx_port_t port, uint8_t *out_value);
esp_err_t txe81xx_read_outputs(txe81xx_handle_t dev, txe81xx_port_t port, uint8_t *out_value);
esp_err_t txe81xx_write_outputs(txe81xx_handle_t dev, txe81xx_port_t port, uint8_t value);

// Direction: 0=input (Hi-Z), 1=output
esp_err_t txe81xx_set_direction(txe81xx_handle_t dev, txe81xx_port_t port, uint8_t dir_mask);
esp_err_t txe81xx_update_direction(txe81xx_handle_t dev, txe81xx_port_t port, uint8_t set_mask, uint8_t clear_mask);

// Push-pull / open-drain register (feature 0x06)
esp_err_t txe81xx_set_pushpull_od(txe81xx_handle_t dev, txe81xx_port_t port, uint8_t mask);
esp_err_t txe81xx_update_pushpull_od(txe81xx_handle_t dev, txe81xx_port_t port, uint8_t set_mask, uint8_t clear_mask);

/* ---- Pull config (feature 0x08 / 0x09) ---- */
esp_err_t txe81xx_set_pull_enable_mask(txe81xx_handle_t dev, txe81xx_port_t port, uint8_t enable_mask);
esp_err_t txe81xx_set_pull_select_mask(txe81xx_handle_t dev, txe81xx_port_t port, uint8_t select_mask); // 0=down, 1=up (per datasheet convention)
esp_err_t txe81xx_update_pull_enable_mask(txe81xx_handle_t dev, txe81xx_port_t port, uint8_t set_mask, uint8_t clear_mask);
esp_err_t txe81xx_update_pull_select_mask(txe81xx_handle_t dev, txe81xx_port_t port, uint8_t set_mask, uint8_t clear_mask);

/* ---- Interrupt support (TXE INT pin -> ESP GPIO) ----
 *
 * Datasheet bits:
 *  - Smart Interrupt register (0x0B): bit=0 enabled, bit=1 disabled (per port)
 *  - Interrupt Mask register (0x0C): bit=0 interrupt enabled, bit=1 masked
 *  - Interrupt Flag Status (0x0E): read clears when smart disabled; provides pin flags
 *  - Interrupt Port Status (0x0F): port snapshot
 *  - Input Glitch Filter Enable (0x0D): per-pin enable
 */

typedef void (*txe81xx_irq_cb_t)(txe81xx_port_t port,
                                 uint8_t flags,       // which pins triggered (from 0x0E)
                                 uint8_t port_state,  // current input state (from 0x02)
                                 void *user_ctx);

typedef struct {
    gpio_num_t int_gpio;     // ESP GPIO connected to TXE INT pin
    bool active_low;         // true if INT is active-low (common)
    txe81xx_irq_cb_t cb;
    void *user_ctx;

    // FreeRTOS task settings
    uint32_t task_stack;     // e.g. 3072
    UBaseType_t task_prio;   // e.g. 10
    int task_core;           // -1 = no pin, else core id
} txe81xx_irq_config_t;

/**
 * Installs ESP GPIO ISR and starts an IRQ handling task.
 * Does NOT configure TXE interrupt registers by itself.
 */
esp_err_t txe81xx_irq_install(txe81xx_handle_t dev, const txe81xx_irq_config_t *cfg);

/**
 * Stops IRQ task and removes ISR handler.
 */
esp_err_t txe81xx_irq_uninstall(txe81xx_handle_t dev);

/**
 * Configure TXE interrupt for a port:
 *  - smart_enable: if true => smart interrupt enabled (bit=0), else disabled (bit=1)
 *  - mask: 1=masked, 0=enabled per pin (datasheet)
 */
esp_err_t txe81xx_irq_configure_port(txe81xx_handle_t dev,
                                     txe81xx_port_t port,
                                     bool smart_enable,
                                     uint8_t mask);

/**
 * Convenience: enable interrupts for specific pins (pins_mask=1 means enable).
 * Internally writes Interrupt Mask reg with 0 for enabled pins.
 */
esp_err_t txe81xx_irq_enable_pins(txe81xx_handle_t dev,
                                  txe81xx_port_t port,
                                  uint8_t pins_mask);

/**
 * Enable glitch filter for selected pins (feature 0x0D).
 */
esp_err_t txe81xx_glitch_filter_enable(txe81xx_handle_t dev,
                                       txe81xx_port_t port,
                                       uint8_t pins_mask);
esp_err_t txe81xx_update_glitch_filter(txe81xx_handle_t dev,
                                       txe81xx_port_t port,
                                       uint8_t set_mask,
                                       uint8_t clear_mask);

esp_err_t txe81xx_irq_read_flags(txe81xx_handle_t dev, txe81xx_port_t port, uint8_t *out_flags);
esp_err_t txe81xx_irq_read_port_status(txe81xx_handle_t dev, uint8_t *out_status);
esp_err_t txe81xx_irq_clear_pending(txe81xx_handle_t dev);

#ifdef __cplusplus
}
#endif
