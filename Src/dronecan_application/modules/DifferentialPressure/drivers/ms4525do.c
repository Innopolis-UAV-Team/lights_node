/*
 * Copyright (C) 2020-2023 Dmitry Ponomarev <ponomarevda96@gmail.com>
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "ms4525do.h"
#include <string.h>
#include "periphery/i2c/i2c.hpp"

#define I2C_ID              (0x28 << 1) + 1
#define I2C_RESPONSE_SIZE   4

static uint8_t ms4525do_rx_buf[I2C_RESPONSE_SIZE] = {0x00};

int8_t ms4525doInit() {
    return 0;
}

int8_t ms4525CollectData() {
    return i2cReceive(I2C_ID, ms4525do_rx_buf, I2C_RESPONSE_SIZE);
}

DifferentialPressureData ms4525ParseCollectedData() {
    int16_t dp_raw = 0;
    int16_t dT_raw = 0;

    dp_raw = (int16_t)((ms4525do_rx_buf[0] << 8) + ms4525do_rx_buf[1]);
    /* mask the used bits */
    dp_raw = 0x3FFF & dp_raw;
    dT_raw = (int16_t)((ms4525do_rx_buf[2] << 8) + ms4525do_rx_buf[3]);
    dT_raw = (0xFFE0 & dT_raw) >> 5;
    float temperature = ((200.0f * dT_raw) / 2047) - 50;

    // Calculate differential pressure. As its centered around 8000
    // and can go positive or negative
    const float P_min = -1.0f;
    const float P_max = 1.0f;
    const float PSI_to_Pa = 6894.757f;
    /*
    this equation is an inversion of the equation in the
    pressure transfer function figure on page 4 of the datasheet
    We negate the result so that positive differential pressures
    are generated when the bottom port is used as the static
    port on the pitot and top port is used as the dynamic port
    */
    float diff_press_PSI = -((dp_raw - 0.1f * 16383) * (P_max - P_min) / (0.8f * 16383) + P_min);
    float diff_press_pa_raw = diff_press_PSI * PSI_to_Pa;

    DifferentialPressureData data;
    data.temperature = temperature;
    data.diff_pressure = diff_press_pa_raw;
    return data;
}

void ms4525doFillBuffer(const uint8_t new_buffer[]) {
    memcpy(ms4525do_rx_buf, new_buffer, I2C_RESPONSE_SIZE);
}
