#include "PrinterCPAP.hpp"

#include "Log.hpp"

#include "bsp/esp-bsp.h"

#include "button_gpio.h"

#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef BSP_BUTTON_LEFT
#define USE_LEFT_BUTTON 1
#else
#define USE_LEFT_BUTTON 0
#endif

#ifdef BSP_BUTTON_MIDDLE
#define USE_MIDDLE_BUTTON 1
#else
#define USE_MIDDLE_BUTTON 0
#endif

#ifdef BSP_BUTTON_RIGHT
#define USE_RIGHT_BUTTON 1
#else
#define USE_RIGHT_BUTTON 0
#endif

#define USE_BUTTONS (USE_LEFT_BUTTON && USE_MIDDLE_BUTTON && USE_RIGHT_BUTTON)

namespace pcp {
    static constexpr uint32_t kButtonHeight = 32;

    void _downPressed(lv_event_t* event);
    void _upPressed(lv_event_t* event);
    void _armStopPressed(lv_event_t* event);
    void _middleButtonPressed(void *button_handle, void *user_data);
    void _leftButtonPressed(void *button_handle, void *user_data);
    void _rightButtonPressed(void *button_handle, void *user_data);

    PrinterCPAP::PrinterCPAP() : _motor() {
        PCP_LOGI("Creating Printer CPAP Controller");
        _turnOffSpeaker();
        _setupScreen();
    }

    PrinterCPAP::~PrinterCPAP() {
        if (_throttleLabel != nullptr) {
            lv_obj_delete(_throttleLabel);
        }
        if (_downButton != nullptr) {
            lv_obj_delete(_downButton);
        }
        if (_upButton != nullptr) {
            lv_obj_delete(_upButton);
        }
        if (_armStopButton != nullptr) {
            lv_obj_delete(_armStopButton);
        }
    }

    void PrinterCPAP::run(void) {
        while (true) {
            _loop();
        }
    }

    void PrinterCPAP::armStopPressed() {
        bsp_display_lock(0);
        lv_obj_remove_flag(_spinner, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(_downButton, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(_upButton, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(_armStopButton, LV_OBJ_FLAG_HIDDEN);
        bsp_display_unlock();
        if (_motorIsArmed) {
            _motor.unarm([this](void){ this->_unarmCompleted(); });
        } else {
            _motor.arm([this](void){ this->_armCompleted(); });
        }
    }

    void PrinterCPAP::downPressed() {
        _motor.decreaseThrottle(10, 250);
    }

    void PrinterCPAP::upPressed() {
        _motor.increaseThrottle(10, 250);
    }

    void PrinterCPAP::_armCompleted(void) {
        std::lock_guard<std::mutex> _guard(_queuedActionsMutex);
        _queuedActions.push([this]() {
            bsp_display_lock(0);
            for(size_t i = 0; i < lv_obj_get_child_cnt(_armStopButton); i++) {
                lv_obj_t* child = lv_obj_get_child(_armStopButton, i);
                if (lv_obj_has_class(child, &lv_label_class)) {
                    lv_label_set_text(child, LV_SYMBOL_PAUSE);
                    break;
                }
            }
            lv_obj_remove_flag(_armStopButton, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(_downButton, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(_upButton, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(_spinner, LV_OBJ_FLAG_HIDDEN);
            bsp_display_unlock();
            _motorIsArmed = true;
        });
    }

    void PrinterCPAP::_unarmCompleted(void) {
        std::lock_guard<std::mutex> _guard(_queuedActionsMutex);
        _queuedActions.push([this]() {
            bsp_display_lock(0);
            for(size_t i = 0; i < lv_obj_get_child_cnt(_armStopButton); i++) {
                lv_obj_t* child = lv_obj_get_child(_armStopButton, i);
                if (lv_obj_has_class(child, &lv_label_class)) {
                    lv_label_set_text(child, LV_SYMBOL_PLAY);
                    break;
                }
            }
            lv_obj_remove_flag(_armStopButton, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(_downButton, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(_upButton, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(_spinner, LV_OBJ_FLAG_HIDDEN);
            bsp_display_unlock();
            _motorIsArmed = false;
        });
    }

    void PrinterCPAP::_turnOffSpeaker(void) {
        gpio_config_t config = {
            .pin_bit_mask = 1 << 25,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_ENABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        esp_err_t err = gpio_config(&config);
        if (err != ESP_OK) {
            PCP_LOGW("Could not turn the speaker off: %s", esp_err_to_name(err));
        }
    }

    void PrinterCPAP::_setupScreen(void) {
        bsp_display_start();
        bsp_display_backlight_on();
        bsp_display_lock(0);
        lv_obj_t* screen = lv_screen_active();
        _throttleLabel = lv_label_create(screen);
        lv_obj_set_size(_throttleLabel, lv_pct(100), lv_pct(66));
        lv_obj_set_pos(_throttleLabel, 0, 48);
        lv_obj_set_style_text_align(_throttleLabel, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_font(_throttleLabel, &lv_font_montserrat_48, LV_STATE_DEFAULT);
        
        std::optional<gpio_num_t> buttonLeftGPIO;
        std::optional<gpio_num_t> buttonMiddleGPIO;
        std::optional<gpio_num_t> buttonRightGPIO;
#if USE_BUTTONS
        buttonLeftGPIO = BSP_BUTTON_LEFT;
        buttonMiddleGPIO = BSP_BUTTON_MIDDLE;
        buttonRightGPIO = BSP_BUTTON_RIGHT;
#endif

        _downButton = _createButton(screen, LV_ALIGN_BOTTOM_LEFT, 16, LV_SYMBOL_DOWN);
        _upButton = _createButton(screen, LV_ALIGN_BOTTOM_RIGHT, -16, LV_SYMBOL_UP);
        _armStopButton = _createButton(screen, LV_ALIGN_BOTTOM_MID, 0, LV_SYMBOL_PLAY);
        lv_obj_add_event_cb(_downButton, _downPressed, LV_EVENT_CLICKED, this);
        lv_obj_add_event_cb(_upButton, _upPressed, LV_EVENT_CLICKED, this);
        lv_obj_add_event_cb(_armStopButton, _armStopPressed, LV_EVENT_CLICKED, this);

        _configurePhysicalButton(buttonMiddleGPIO, &_middleButton, _middleButtonPressed);
        _configurePhysicalButton(buttonLeftGPIO, &_leftButton, _leftButtonPressed);
        _configurePhysicalButton(buttonRightGPIO, &_rightButton, _rightButtonPressed);

        lv_obj_add_flag(_downButton, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(_upButton, LV_OBJ_FLAG_HIDDEN);

        _spinner = lv_spinner_create(screen);
        lv_obj_set_width(_spinner, 32);
        lv_obj_set_height(_spinner, 32);
        lv_obj_align(_spinner, LV_ALIGN_BOTTOM_MID, 0, -2);
        lv_obj_add_flag(_spinner, LV_OBJ_FLAG_HIDDEN);
    
        bsp_display_unlock();
    }

    lv_obj_t* PrinterCPAP::_createButton(lv_obj_t* parent, lv_align_t align, int32_t x, const char* text) {
        lv_obj_t* button = lv_button_create(parent);
        lv_obj_set_size(button, lv_pct(25), kButtonHeight);
        lv_obj_align(button, align, x, -2);
        lv_obj_remove_flag(button, LV_OBJ_FLAG_PRESS_LOCK);
        lv_obj_add_flag(button, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_t* label = lv_label_create(button);
        lv_label_set_text(label, text);
        lv_obj_set_align(label, LV_ALIGN_CENTER);

        return button;
    }

    void PrinterCPAP::_configurePhysicalButton(const std::optional<uint8_t>& gpio, button_handle_t* buttonHandle, button_cb_t callback) {
        if (gpio.has_value()) {
            button_config_t buttonConfig = {
                .long_press_time = 0,
                .short_press_time = 0,
            };
            button_gpio_config_t buttonGPIOConfig = {
                .gpio_num = gpio.value(),
                .active_level = 0,
                .enable_power_save = true,
                .disable_pull = false,
            };
            esp_err_t err = iot_button_new_gpio_device(&buttonConfig, &buttonGPIOConfig, buttonHandle);
            if (err != ESP_OK) {
                PCP_LOGE("Could not bind button: %s", esp_err_to_name(err));
                return;
            }
            iot_button_register_cb(*buttonHandle, BUTTON_PRESS_END, nullptr, callback, this);
        }
    }

    void PrinterCPAP::_loop(void) {
        bsp_display_lock(0);
        
        lv_label_set_text_fmt(_throttleLabel, "%u%%", _motor.throttle());
        lv_task_handler();

        bsp_display_unlock();


        {
            std::lock_guard<std::mutex> locker(_queuedActionsMutex);
            while (!_queuedActions.empty()) {
                const std::function<void(void)>& action = _queuedActions.front();
                _queuedActions.pop(); 
                action();
            }
        }

        vTaskDelay(20 / portTICK_PERIOD_MS);
    }

    void _armStopPressed(lv_event_t* event) {
        PrinterCPAP* printerCPAP = reinterpret_cast<PrinterCPAP*>(lv_event_get_user_data(event));
        printerCPAP->armStopPressed();
    }

    void _downPressed(lv_event_t* event) {
        PrinterCPAP* printerCPAP = reinterpret_cast<PrinterCPAP*>(lv_event_get_user_data(event));
        printerCPAP->downPressed();
    }

    void _upPressed(lv_event_t* event) {
        PrinterCPAP* printerCPAP = reinterpret_cast<PrinterCPAP*>(lv_event_get_user_data(event));
        printerCPAP->upPressed();
    }

    void _middleButtonPressed(void* buttonHandle, void* userData) {
        PrinterCPAP* printerCPAP = reinterpret_cast<PrinterCPAP*>(userData);
        if (!lv_obj_has_flag(printerCPAP->_armStopButton, LV_OBJ_FLAG_HIDDEN)) {
            printerCPAP->armStopPressed();
        }
    }

    void _leftButtonPressed(void* buttonHandle, void* userData) {
        PrinterCPAP* printerCPAP = reinterpret_cast<PrinterCPAP*>(userData);
        if (!lv_obj_has_flag(printerCPAP->_downButton, LV_OBJ_FLAG_HIDDEN)) {
            printerCPAP->downPressed();
        }
    }

    void _rightButtonPressed(void* buttonHandle, void* userData) {
        PrinterCPAP* printerCPAP = reinterpret_cast<PrinterCPAP*>(userData);
        if (!lv_obj_has_flag(printerCPAP->_upButton, LV_OBJ_FLAG_HIDDEN)) {
            printerCPAP->upPressed();
        }
    }
}
