#pragma once

#include "driver/gpio.h"

namespace pcp {
#if CONFIG_IDF_TARGET_ESP32
    static constexpr gpio_num_t kFanInputGPIO = GPIO_NUM_16;
    static constexpr gpio_num_t kMotorOutputGPIO = GPIO_NUM_17;
#elif CONFIG_IDF_TARGET_ESP32S3 
    static constexpr gpio_num_t kFanInputGPIO = GPIO_NUM_5;
    static constexpr gpio_num_t kMotorOutputGPIO = GPIO_NUM_17;
#else
    #error "Unsupported Platform " CONFIG_IDF_TARGET
#endif

    static_assert(kFanInputGPIO, "Fan Input GPIO must be defined");
    static_assert(kMotorOutputGPIO, "Motor Output GPIO must be defined");
}
