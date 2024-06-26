/// This software is distributed under the terms of the MIT License.
/// Copyright (c) 2022-2023 Dmitry Ponomarev.
/// Author: Dmitry Ponomarev <ponomarevda96@gmail.com>

#ifndef SRC_MAIN_HPP_
#define SRC_MAIN_HPP_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

static inline uint32_t HAL_GetUIDw0() {return 0;}
static inline uint32_t HAL_GetUIDw1() {return 0;}
static inline uint32_t HAL_GetUIDw2() {return 0;}

uint32_t HAL_GetTick();
void HAL_NVIC_SystemReset();

uint32_t uavcanGetTimeMs();
typedef enum {
  GPIO_PIN_RESET = false,
  GPIO_PIN_SET
} GPIO_PinState;

#ifdef __cplusplus
}
#endif

#endif  // SRC_MAIN_HPP_
