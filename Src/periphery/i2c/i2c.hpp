/*
 * Copyright (C) 2024 Anastasiia Stepanova <asiiapine@gmail.com>
 * Distributed under the terms of the GPL v3 license, available in the file
 */

#ifndef SRC_PERIPHERY_I2C_I2C_HPP_
#define SRC_PERIPHERY_I2C_I2C_HPP_

#include <stddef.h>
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum i2c_error_e {
    I2C_SUCCESS = 0,
    I2C_BAD_PARAMETER,
    I2C_RUNTIME_ERROR,
    I2C_TRANSMIT_ERROR,
    I2C_RECEIVE_ERROR,
    I2C_ERROR,
} i2c_error_t;

int8_t isDeviceReady(uint8_t id, uint8_t n_trials);

int8_t i2cTransmit(uint8_t id, const uint8_t tx[], uint8_t len);
int8_t i2cReceive(uint8_t id, uint8_t *rx, uint8_t len);

/**
 * @brief               Reads N bits from specified register of the i2c device
 * @param id            Target device address: The device 7 bits address value
 *                      in datasheet must be shifted to the left before calling the interface
 * @param mem_addr      Target device register
 * @param pData         Pointer to data buffer
 * @param data_length   Length of received data
 * @retval              i2c_error_t status
 */
i2c_error_t i2cReadRegister(uint8_t id, uint8_t mem_addr,
                                    uint8_t *const pData,  uint8_t data_length);

/**
 * @brief Reads 8 bits from specified register of the i2c device
 * @param id Target device address: The device 7 bits address value
 *         in datasheet must be shifted to the left before calling the interface
 * @param mem_addr Target device register
 * @param pData Pointer to data buffer
 * @retval i2c_error_t status
 */
i2c_error_t i2cReadByteRegister(uint8_t id, uint8_t mem_addr,
                                uint8_t *const pData);

/**
 * @brief Reads 16 bits from specified register of the i2c device
 * @param id Target device address: The device 7 bits address value
 *         in datasheet must be shifted to the left before calling the interface
 * @param mem_addr Target device register
 * @param pData Pointer to data buffer
 * @retval i2c_error_t status
 */
i2c_error_t i2cReadTwoBytesRegister(uint8_t id, uint8_t mem_addr,
                                    uint16_t *const pData);

i2c_error_t i2cWriteBytesToRegister(uint8_t id, uint8_t reg,
                                    uint8_t const *const p_tx, size_t n_bytes);
i2c_error_t i2cWriteTwoBytesToRegister(uint8_t id, uint8_t const reg,
                                       uint16_t const tx_buffer);

#ifdef __cplusplus
}
#endif

#endif  // SRC_PERIPHERY_I2C_I2C_HPP_
