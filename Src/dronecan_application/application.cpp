/***
 * Copyright (C) 2024 Anastasiia Stepanova  <asiiapine96@gmail.com>
 *  Distributed under the terms of the GPL v3 license, available in the file LICENSE.
***/ 

#include "application.hpp"
#include "dronecan.h"
#include "main.h"
#include "params.hpp"
#include "logger.hpp"
#include "modules/LightsModule.hpp"
#include "modules/CircuitStatusModule.hpp"


#ifdef HAL_IWDG_MODULE_ENABLED
extern IWDG_HandleTypeDef hiwdg;
#endif /* HAL_IWDG_MODULE_ENABLED */

void application_entry_point() {

    paramsInit(static_cast<uint8_t>(IntParamsIndexes::INTEGER_PARAMS_AMOUNT), NUM_OF_STR_PARAMS);
    paramsLoadFromFlash();

    auto node_id = paramsGetIntegerValue(IntParamsIndexes::PARAM_UAVCAN_NODE_ID);

    const auto node_name = "co.raccoonlab.mini";
    Logger logger = Logger(node_name);
    auto node_name_param_idx = static_cast<ParamIndex_t>(IntParamsIndexes::INTEGER_PARAMS_AMOUNT);

    paramsSetStringValue(node_name_param_idx, 19, (const uint8_t*)node_name);
    uavcanSetNodeName(node_name);

    uavcanInitApplication(node_id);
    
    auto default_color = paramsGetIntegerValue(IntParamsIndexes::PARAM_LIGHTS_DEFAULT_COLOR);
    auto blink_period = paramsGetIntegerValue(IntParamsIndexes::PARAM_LIGHTS_BLINK_PERIOD_MS);
    uint8_t max_ext_intensity_ptc = paramsGetIntegerValue(IntParamsIndexes::PARAM_LIGHTS_MAX_INTENSITY);
    uint8_t duty_cycle_ptc =  paramsGetIntegerValue(IntParamsIndexes::PARAM_LIGHTS_DUTY_CYCLE_PTC);
    
    RgbSimpleColor default_rgb_color = RgbSimpleColor(default_color);
    uint16_t duty_cycle = blink_period * (duty_cycle_ptc/100.0);

    LightsModule* light_module = LightsModule::GetInstance(duty_cycle_ptc, blink_period, max_ext_intensity_ptc, default_rgb_color);

    CircuitStatusModule* status_module = CircuitStatusModule::GetInstance(light_module);

    while(true) {
        light_module->spin_once();
        status_module->spin_once();

        uavcanSpinOnce();
    }

    #ifdef HAL_IWDG_MODULE_ENABLED
    HAL_IWDG_Refresh(&hiwdg);
    #endif  // HAL_IWDG_MODULE_ENABLED
}