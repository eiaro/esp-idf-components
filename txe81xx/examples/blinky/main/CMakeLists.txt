# SPDX-FileCopyrightText: 2026 Ronny Eia <3652665+eiaro@users.noreply.github.com>
#
# SPDX-License-Identifier: MIT

idf_component_register(SRCS "blinky_main.c"
                       PRIV_REQUIRES esp_driver_spi
                       REQUIRES txe81xx)
