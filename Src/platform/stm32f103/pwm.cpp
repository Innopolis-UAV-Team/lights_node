/***
 * Copyright (C) 2024 Anastasiia Stepanova  <asiiapine@gmail.com>
 *  Distributed under the terms of the GPL v3 license, available in the file
 *LICENSE.
 ***/

#include "periphery/pwm/pwm.hpp"

#include "main.h"

#ifdef htim1
extern TIM_HandleTypeDef htim1;
#endif
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;

Logger PwmPeriphery::_logger = Logger("PWM");

void PwmPeriphery::init(PwmPin pin) {
    switch (pin) {
#ifdef htim1

        case PwmPin::PWM_1:
            HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
            break;
#endif
        case PwmPin::PWM_2:
            HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
            break;

        case PwmPin::PWM_3:
            HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
            break;

        case PwmPin::PWM_4:
            HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
            break;

        case PwmPin::PWM_5:
            HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
            break;

        case PwmPin::PWM_6:
            HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
            break;

        default:
            break;
    }
}

void PwmPeriphery::set_duration(const PwmPin pwm_pin, uint16_t duration_us) {
    switch (pwm_pin) {
        case PwmPin::PWM_1:
            TIM1->CCR1 = duration_us;
            break;

        case PwmPin::PWM_2:
            TIM2->CCR1 = duration_us;
            break;

        case PwmPin::PWM_3:
            TIM3->CCR2 = duration_us;
            break;

        case PwmPin::PWM_4:
            TIM3->CCR1 = duration_us;
            break;

        case PwmPin::PWM_5:
            TIM2->CCR2 = duration_us;
            break;

        case PwmPin::PWM_6:
            TIM3->CCR3 = duration_us;
            break;

        default:
            break;
    }
}

uint16_t PwmPeriphery::get_duration(PwmPin pwm_pin) {
    uint16_t pwm_duration;

    switch (pwm_pin) {
        case PwmPin::PWM_1:
            pwm_duration = TIM1->CCR2;
            break;

        case PwmPin::PWM_2:
            pwm_duration = TIM2->CCR1;
            break;

        case PwmPin::PWM_3:
            pwm_duration = TIM3->CCR2;
            break;

        case PwmPin::PWM_4:
            pwm_duration = TIM3->CCR1;
            break;

        case PwmPin::PWM_5:
            pwm_duration = TIM2->CCR2;
            break;

        case PwmPin::PWM_6:
            pwm_duration = TIM3->CCR3;
            break;

        default:
            pwm_duration = 0;
            break;
    }

    return pwm_duration;
}

void PwmPeriphery::set_duty_cycle_pct(PwmPin pwm_pin, uint8_t duty_cycle_pct) {
    if (duty_cycle_pct > 100) {
        return;
    }

    float fraction = 1.0 - (duty_cycle_pct / 100.0);

    switch (pwm_pin) {
        case PwmPin::PWM_1:
            TIM1->CCR1 = (fraction * TIM1->ARR);
            break;

        case PwmPin::PWM_2:
            TIM2->CCR1 = (fraction * TIM2->ARR);
            break;

        case PwmPin::PWM_3:
            TIM3->CCR2 = (fraction * TIM3->ARR);
            break;

        case PwmPin::PWM_4:
            TIM3->CCR1 = (fraction * TIM3->ARR);
            break;

        case PwmPin::PWM_5:
            TIM2->CCR2 = (fraction * TIM2->ARR);
            break;

        case PwmPin::PWM_6:
            TIM3->CCR3 = (fraction * TIM3->ARR);
            break;

        default:
            _logger.log_debug("No such PIN");
            break;
    }
}

void PwmPeriphery::reset(PwmPin pwm_pin) {
    switch (pwm_pin) {
        case PwmPin::PWM_1:
            TIM1->CCR1 = TIM1->ARR;
            break;

        case PwmPin::PWM_2:
            TIM2->CCR1 = TIM2->ARR;
            break;

        case PwmPin::PWM_3:
            TIM3->CCR2 = TIM3->ARR;
            break;

        case PwmPin::PWM_4:
            TIM3->CCR1 = TIM3->ARR;
            break;

        case PwmPin::PWM_5:
            TIM2->CCR2 = TIM2->ARR;
            break;

        case PwmPin::PWM_6:
            TIM3->CCR3 = TIM3->ARR;
            break;
        default:
            _logger.log_debug("No such PIN");
            break;
    }
}

void PwmPeriphery::set_frequency(PwmPin pwm_pin, uint16_t frequency_hz) {
    if (pwm_pin > PwmPin::PWM_AMOUNT) {
        return;
    }
    volatile uint32_t* arr_reg;
    switch (pwm_pin) {
#ifdef htim1
        case PwmPin::PWM_1:
            arr_reg = &(htim1.Instance->ARR);
            break;
#endif
        case PwmPin::PWM_2:
            arr_reg = &(htim2.Instance->ARR);
            break;

        case PwmPin::PWM_3:
            arr_reg = &(htim2.Instance->ARR);
            break;

        case PwmPin::PWM_4:
            arr_reg = &(htim3.Instance->ARR);
            break;

        case PwmPin::PWM_5:
            arr_reg = &(htim2.Instance->ARR);
            break;

        case PwmPin::PWM_6:
            arr_reg = &(htim3.Instance->ARR);
            break;
        default:
            _logger.log_debug("No such PIN");
            return;
    }
    uint16_t period_us = 1000000 / frequency_hz;
    *arr_reg = period_us;
}

uint16_t PwmPeriphery::get_frequency(PwmPin pwm_pin) {
    if (pwm_pin > PwmPin::PWM_AMOUNT) {
        return 0;
    }
    volatile uint32_t* arr_reg;
    switch (pwm_pin) {
#ifdef htim1
        case PwmPin::PWM_1:
            arr_reg = &(htim1.Instance->ARR);
            break;
#endif
        case PwmPin::PWM_2:
            arr_reg = &(htim2.Instance->ARR);
            break;

        case PwmPin::PWM_3:
            arr_reg = &(htim2.Instance->ARR);
            break;

        case PwmPin::PWM_4:
            arr_reg = &(htim3.Instance->ARR);
            break;

        case PwmPin::PWM_5:
            arr_reg = &(htim2.Instance->ARR);
            break;

        case PwmPin::PWM_6:
            arr_reg = &(htim3.Instance->ARR);
            break;
        default:
            _logger.log_debug("No such PIN");
            return 0;
    }
    uint32_t frequency = 1000000 / *arr_reg;

    return frequency;
}
