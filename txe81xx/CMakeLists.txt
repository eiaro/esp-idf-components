# SPDX-FileCopyrightText: 2026 Ronny Eia <3652665+eiaro@users.noreply.github.com>
#
# SPDX-License-Identifier: MIT

idf_component_register(
    SRCS "src/txe81xx.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES "driver esp_driver_spi"
    )
