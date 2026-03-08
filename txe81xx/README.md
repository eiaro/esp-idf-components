# esp-idf-txe81xx

ESP-IDF component driver for the TXE81XX family (TXE8116 / TXE8124) over SPI.

## Features

- SPI register access abstraction for TXE81XX feature map
- TXE8116 (2 ports) and TXE8124 (3 ports) support
- Input/output, direction, pull config, push-pull/open-drain helpers
- Optional IRQ support via TXE INT pin -> ESP GPIO
- Kconfig options for mutex, strict status checking, and verbose logging

## Requirements

- ESP-IDF `>=5.1`

## Install from ESP-IDF Component Registry

In your ESP-IDF project:

```bash
idf.py add-dependency "eiaro/txe81xx^1.0.0"
```

Then include and use the API from `txe81xx.h`.

## Local development

This repository itself is an ESP-IDF component (root-level `CMakeLists.txt`).

The example app in `examples/blinky` uses `idf_component.yml` with `override_path` so it always builds against the local checkout while developing.

## Quick usage

```c
txe81xx_spi_config_t cfg = {
	.host = SPI2_HOST,
	.cs_gpio = 5,
	.clock_hz = 1000000,
	.mode = 0,
	.queue_size = 1,
	.use_polling = true,
	.chip = TXE_CHIP_8116,
};

txe81xx_handle_t dev = NULL;
ESP_ERROR_CHECK(txe81xx_init_spi(&cfg, &dev));
ESP_ERROR_CHECK(txe81xx_set_direction(dev, TXE_PORT0, 0x01));
ESP_ERROR_CHECK(txe81xx_write_outputs(dev, TXE_PORT0, 0x01));
```

## Example: blinky

The example toggles `PORT0 bit0` every 500 ms.

```bash
cd examples/blinky
idf.py set-target esp32s3
idf.py build
idf.py -p <PORT> flash monitor
```

Default SPI pins are defined in `examples/blinky/main/blinky_main.c` and can be changed there to match your board wiring.
