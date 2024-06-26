// Copyright (C) 2023 Dmitry Ponomarev <ponomarevda96@gmail.com>
// Distributed under the terms of the GPL v3 license, available in the file LICENSE.

#include "periphery/adc/adc.hpp"
#include "main.h"

extern ADC_HandleTypeDef hadc1;

static inline uint16_t adc_dma_buffer[static_cast<uint8_t>(AdcChannel::ADC_NUMBER_OF_CNANNELS)];

int8_t AdcPeriphery::init() {
    if (HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK) {
        return -1;
    }

    auto channels_amount = static_cast<uint8_t>(AdcChannel::ADC_NUMBER_OF_CNANNELS);
    if (HAL_ADC_Start_DMA(&hadc1, (uint32_t*)(void*)&adc_dma_buffer, channels_amount) != HAL_OK) {
        return -1;
    }

    _is_adc_already_inited = true;
    return 0;
}

uint16_t AdcPeriphery::get(AdcChannel channel) {
    if (!_is_adc_already_inited || channel >= AdcChannel::ADC_NUMBER_OF_CNANNELS) {
        return 0;
    }
    return adc_dma_buffer[static_cast<uint8_t>(channel)];
}

float AdcPeriphery::stm32Current(uint16_t curr) {
    if (curr < 2048) return 0;
    return (curr - 2048.00) / 90;
}

float AdcPeriphery::stm32Temperature(uint16_t temp) {
    return stm32TemperatureParse(temp);
}

float AdcPeriphery::stm32Voltage(uint16_t volt) {
    return volt/64.0;
}
float AdcPeriphery::stm32Voltage5V(uint16_t volt) {
    return volt/640.0;
}

#ifdef HAL_ADC_MODULE_ENABLED
/**
 * @note We assume that hadc->Instance == ADC1 always!
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)(void*)&adc_dma_buffer,
                        static_cast<uint8_t>(AdcChannel::ADC_NUMBER_OF_CNANNELS));
}
#endif
