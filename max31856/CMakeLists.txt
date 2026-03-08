# SPDX-FileCopyrightText: 2025 Ronny Eia <3652665+eiaro@users.noreply.github.com>
#
# SPDX-License-Identifier: MIT

idf_component_register(
    SRCS "src/max31856.c"
    INCLUDE_DIRS "include"
    REQUIRES "esp_driver_gpio esp_driver_spi"
)
