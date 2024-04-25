#include "PWMModule.hpp"

#define CHANNEL(channel) IntParamsIndexes::PARAM_PWM_##channel##_CH
#define MIN(channel) IntParamsIndexes::PARAM_PWM_##channel##_MIN
#define MAX(channel) IntParamsIndexes::PARAM_PWM_##channel##_MAX
#define DEF(channel) IntParamsIndexes::PARAM_PWM_##channel##_DEF
#define FB(channel) IntParamsIndexes::PARAM_PWM_##channel##_FB

Logger PWMModule::logger = Logger("PWMModule");

uint16_t PWMModule::ttl_cmd = 0;
uint16_t PWMModule::pwm_freq = 1000;
uint8_t PWMModule::pwm_cmd_type = 0;

ModuleStatus PWMModule::module_status = ModuleStatus::MODULE_OK;

PWMModule PWMModule::instance = PWMModule();

PWMModule::PWMModule() {
    update_params();
    init();
}

PwmChannelInfo PWMModule::params = {.pin = PwmPin::PWM_5};

PwmChannelsParamsNames PWMModule::params_names = {
    .min = MIN(5), .max = MAX(5), .def = DEF(5), .ch = CHANNEL(5), .fb = FB(5)};

PWMModule& PWMModule::get_instance() {
    static bool instance_initialized = false;
    if (!instance_initialized) {
        instance_initialized = true;
        instance.init();
    }
    return instance;
}

void PWMModule::init() {
    logger.init("PWMModule");
    PwmPeriphery::init(params.pin);
}

void PWMModule::spin_once() {
    uint32_t crnt_time_ms = HAL_GetTick();

    static uint32_t next_update_ms = 0;
    if (crnt_time_ms > next_update_ms) {
        next_update_ms = crnt_time_ms + 1000;
        instance.update_params();
        instance.apply_params();
    }

    if (crnt_time_ms > params.cmd_end_time_ms) {
        params.command_val = params.def;
    }
    PwmPeriphery::set_duration(params.pin, params.command_val);

    status_pub_timeout_ms = 1;
    static uint32_t next_pub_ms = status_pub_timeout_ms;

    if (module_status == ModuleStatus::MODULE_OK &&
        crnt_time_ms > next_pub_ms) {
        publish_state();
        next_pub_ms = crnt_time_ms + status_pub_timeout_ms;
    }
}

void PWMModule::update_params() {
    module_status = ModuleStatus::MODULE_OK;
    bool params_error = false;

    pwm_freq = paramsGetIntegerValue(IntParamsIndexes::PARAM_PWM_FREQUENCY);
    pwm_cmd_type = paramsGetIntegerValue(IntParamsIndexes::PARAM_PWM_CMD_TYPE);

    ttl_cmd = paramsGetIntegerValue(IntParamsIndexes::PARAM_PWM_CMD_TTL_MS);
    params.fb = paramsGetIntegerValue(params_names.fb);
    auto channel = paramsGetIntegerValue(params_names.ch);
    auto min = paramsGetIntegerValue(params_names.min);
    auto max = paramsGetIntegerValue(params_names.max);
    auto def = paramsGetIntegerValue(params_names.def);

    status_pub_timeout_ms = 100;
    uint8_t max_channel = 0;

    switch (pwm_cmd_type) {
        case 0:
            max_channel = NUMBER_OF_RAW_CMD_CHANNELS - 1;
            break;
        case 1:
            max_channel = 15;
            break;
        default:
            max_channel = 255;
            break;
    }

    static uint32_t last_warn_pub_time_ms = 0;

    if (channel < max_channel) {
        params.channel = channel;
    } else {
        params.channel = max_channel;
        params_error = true;
    }
    params.def = def;
    if (min == max) {
        params_error = true;
    } else {
        params.min = max;
        params.max = min;
    }
    if (params_error) {
        module_status = ModuleStatus::MODULE_WARN;
        if (last_warn_pub_time_ms < HAL_GetTick()) {
            last_warn_pub_time_ms = HAL_GetTick() + 10000;
            logger.log_debug("check parameters");
        }
    }
}

void PWMModule::apply_params() {
    uint16_t data_type_id = 0;
    uint64_t data_type_signature = 0;

    for (int i = 0; i < static_cast<uint8_t>(PwmPin::PWM_AMOUNT); i++) {
        if (PwmPeriphery::get_frequency(params.pin) != pwm_freq) {
            PwmPeriphery::set_frequency(params.pin, pwm_freq);
        }
        switch (pwm_cmd_type) {
            case 0:
                callback = raw_command_callback;
                data_type_signature = UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_SIGNATURE;
                data_type_id = UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_ID;
                publish_state = publish_esc_status;
                break;

            case 1:
                callback = array_command_callback;
                data_type_signature =
                    UAVCAN_EQUIPMENT_ACTUATOR_ARRAY_COMMAND_SIGNATURE;
                data_type_id = UAVCAN_EQUIPMENT_ACTUATOR_ARRAY_COMMAND_ID;
                publish_state = publish_esc_status;
                // publish_state = publish_actuator_status;
                break;

            default:
                return;
        }
    }
    if (module_status == ModuleStatus::MODULE_OK) {
        uavcanSubscribe(data_type_signature, data_type_id, callback);
    }
}

void PWMModule::publish_esc_status() {
    static uint8_t transfer_id = 0;
    EscStatus_t msg{};
    auto crnt_time_ms = HAL_GetTick();
    static uint32_t
        next_status_pub_ms;
    if (params.channel < 0 || params.fb == 0 ||
        next_status_pub_ms > crnt_time_ms) {
        return;
    }
    msg.esc_index = params.channel;
    auto pwm_val = PwmPeriphery::get_duration(params.pin);
    auto scaled_value = mapPwmToPct(pwm_val, params.min, params.max);
    msg.power_rating_pct = (uint8_t)(scaled_value);
    if (dronecan_equipment_esc_status_publish(&msg, &transfer_id) == 0) {
        transfer_id++;
        next_status_pub_ms = crnt_time_ms + ((params.fb > 1) ? 100 : 1000);
    }
}

// void PWMModule::publish_actuator_status() {
//     static uint8_t transfer_id = 0;
//     ActuatorStatus_t msg {};

//     if (params.channel < 0) {
//         return;
//     }
//     msg.actuator_id = params.channel;
//     auto pwm_val = PwmPeriphery::get_duration(params.pin);
//     auto scaled_value = mapPwmToPct(pwm_val, params.min, params.max);
//     msg.power_rating_pct = (uint8_t) scaled_value;
//     if (dronecan_equipment_actuator_status_publish(&msg, &transfer_id) ==
//     0) {
//         transfer_id++;
//     }
// }

void PWMModule::raw_command_callback(CanardRxTransfer* transfer) {
    if (module_status != ModuleStatus::MODULE_OK) return;
    RawCommand_t command;
    auto pwm = &params;
    int8_t ch_num =
        dronecan_equipment_esc_raw_command_deserialize(transfer, &command);
    if ((ch_num <= 0) || (pwm->channel < 0)) {
        return;
    }
    if (command.raw_cmd[pwm->channel] >= 0) {
        pwm->cmd_end_time_ms = HAL_GetTick() + ttl_cmd;
        pwm->command_val = mapRawCommandToPwm(command.raw_cmd[pwm->channel],
                                                pwm->min, pwm->max, pwm->def);
    } else {
        pwm->command_val = pwm->def;
    }
}

void PWMModule::array_command_callback(CanardRxTransfer* transfer) {
    if (module_status != ModuleStatus::MODULE_OK) return;
    ArrayCommand_t command;
    auto pwm = &params;
    int8_t ch_num = dronecan_equipment_actuator_arraycommand_deserialize(
        transfer, &command);
    if (ch_num <= 0 || pwm->channel < 0) {
        return;
    }
    for (uint8_t j = 0; j < ch_num; j++) {
        if (command.commads[j].actuator_id != pwm->channel) {
            continue;
        }
        pwm->cmd_end_time_ms = HAL_GetTick() + ttl_cmd;
        pwm->command_val = mapActuatorCommandToPwm(
            command.commads[j].command_value, pwm->min, pwm->max, pwm->def);
    }
}
