#pragma once

#include "driver/gpio.h"
#include "driver/uart.h"

#include "bsp/esp-bsp.h"
#include "esp_mac.h"

namespace pcp {
    // For M5 stack devices we choose GPIOs such that you plug things in like so:
    //
    // The Fan output of your motherboard:
    // G -> G
    // 12V -> Not Connected
    // Tacho -> Port A signal pin 1
    // PWM -> Port A signal pin 2
    //
    // The one wire interface of your BLHeli ESC:
    // G -> G
    // 5V (if present) -> Port B 5V
    // Signal -> Port B signal pin 1
    //
    // If you don't have an M5 device, we choose pins fairly arbitrarily, use those,
    // or don't - I'm not the boss of you.
#if defined(BSP_BOARD_M5STACK_CORE)
    static constexpr gpio_num_t kFanTachoOutputGPIO = GPIO_NUM_21;
    static constexpr gpio_num_t kFanPWMInputGPIO = GPIO_NUM_22;
    static constexpr gpio_num_t kMotorOutputGPIO = GPIO_NUM_26;
    static constexpr uart_port_t kMotorUART = UART_NUM_2;
#elif defined(BSP_BOARD_M5STACK_CORE_2)
    static constexpr gpio_num_t kFanTachoOutputGPIO = GPIO_NUM_32;
    static constexpr gpio_num_t kFanPWMInputGPIO = GPIO_NUM_33;
    static constexpr gpio_num_t kMotorOutputGPIO = GPIO_NUM_26;
    static constexpr uart_port_t kMotorUART = UART_NUM_2;
#elif defined(BSP_BOARD_M5STACK_CORE_S3)
    static constexpr gpio_num_t kFanTachoOutputGPIO = GPIO_NUM_2;
    static constexpr gpio_num_t kFanPWMInputGPIO = GPIO_NUM_1;
    static constexpr gpio_num_t kMotorOutputGPIO = GPIO_NUM_9;
    static constexpr uart_port_t kMotorUART = UART_NUM_2;
#elif CONFIG_IDF_TARGET_ESP32
    static constexpr gpio_num_t kFanTachoOutputGPIO = GPIO_NUM_32;
    static constexpr gpio_num_t kFanPWMInputGPIO = GPIO_NUM_33;
    static constexpr gpio_num_t kMotorOutputGPIO = GPIO_NUM_26;
    static constexpr uart_port_t kMotorUART = UART_NUM_2;
#elif CONFIG_IDF_TARGET_ESP32S3
    static constexpr gpio_num_t kFanTachoOutputGPIO = GPIO_NUM_2;
    static constexpr gpio_num_t kFanPWMInputGPIO = GPIO_NUM_1;
    static constexpr gpio_num_t kMotorOutputGPIO = GPIO_NUM_9;
    static constexpr uart_port_t kMotorUART = UART_NUM_2;
#else
#error "Unsupported Platform " CONFIG_IDF_TARGET
#endif

    static constexpr gpio_num_t kMotorInputGPIO = kMotorOutputGPIO;

    static_assert(kFanTachoOutputGPIO, "Fan Output GPIO must be defined");
    static_assert(kFanPWMInputGPIO, "Fan Input GPIO must be defined");
    static_assert(kMotorOutputGPIO, "Motor Output GPIO must be defined");
    static_assert(kMotorInputGPIO, "Motor Input GPIO must be defined");
    static_assert(kMotorUART, "Motor UART must be defined");
}  // namespace pcp
