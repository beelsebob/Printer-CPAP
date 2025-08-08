#include "PrinterCPAP.hpp"

#include "Log.hpp"

#if defined(BSP_CAPS_BUTTONS) && BSP_CAPS_BUTTONS
#include "button_gpio.h"
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace pcp {
#if defined(BSP_CAPS_DISPLAY) && BSP_CAPS_DISPLAY
    static constexpr uint32_t kButtonHeight = 32;

    void _downPressed(lv_event_t* event);
    void _upPressed(lv_event_t* event);
    void _armStopPressed(lv_event_t* event);
#endif
#if defined(BSP_CAPS_BUTTONS) && BSP_CAPS_BUTTONS
    void _middleButtonPressed(void *button_handle, void *user_data);
    void _leftButtonPressed(void *button_handle, void *user_data);
    void _rightButtonPressed(void *button_handle, void *user_data);
#endif

    PrinterCPAP::PrinterCPAP() : _motor() {
        PCP_LOGI("Creating Printer CPAP Controller");
        esp_timer_init();
        _turnOffSpeaker();
        _setupScreen();
        _setupButtons();
        _setupMotor();
        _setupFanInput();
    }

    PrinterCPAP::~PrinterCPAP() {
#if defined(BSP_CAPS_DISPLAY) && BSP_CAPS_DISPLAY
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
#endif
        _fanInput.stop();
    }

    void PrinterCPAP::run(void) {
        while (true) {
            _loop();
        }
    }

    void PrinterCPAP::armStopPressed() {
#if defined(BSP_CAPS_DISPLAY) && BSP_CAPS_DISPLAY
        bsp_display_lock(0);
        lv_obj_remove_flag(_spinner, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(_downButton, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(_upButton, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(_armStopButton, LV_OBJ_FLAG_HIDDEN);
        bsp_display_unlock();
#endif
        if (_motor.isArmed()) {
            _motor.disarm([this](void){ this->_disarmCompleted(); });
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
#if defined(BSP_CAPS_DISPLAY) && BSP_CAPS_DISPLAY
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
        });
#endif
    }

    void PrinterCPAP::_disarmCompleted(void) {
#if defined(BSP_CAPS_DISPLAY) && BSP_CAPS_DISPLAY
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
        });
#endif
    }

    void PrinterCPAP::_turnOffSpeaker(void) {
        esp_err_t err = ESP_OK;
#if defined(BSP_CAPS_AUDIO_SPEAKER) && BSP_CAPS_AUDIO_SPEAKER
#if defined(BSP_CAPS_AUDIO) && BSP_CAPS_AUDIO
        bsp_audio_codec_speaker_init();
#else
        err = bsp_speaker_init();
#endif
#endif
        if (err != ESP_OK) {
            PCP_LOGW("Could not turn the speaker off (by turning it on): %s", esp_err_to_name(err));
        }
    }

    void PrinterCPAP::_setupScreen(void) {
#if defined(BSP_CAPS_DISPLAY) && BSP_CAPS_DISPLAY
        bsp_display_start();
        bsp_display_backlight_on();
        bsp_display_lock(0);
        lv_obj_t* screen = lv_screen_active();
        _throttleLabel = lv_label_create(screen);
        lv_obj_set_size(_throttleLabel, lv_pct(100), lv_pct(66));
        lv_obj_set_pos(_throttleLabel, 0, 48);
        lv_obj_set_style_text_align(_throttleLabel, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_font(_throttleLabel, &lv_font_montserrat_48, LV_STATE_DEFAULT);
        
        _downButton = _createButton(screen, LV_ALIGN_BOTTOM_LEFT, 16, LV_SYMBOL_DOWN);
        _upButton = _createButton(screen, LV_ALIGN_BOTTOM_RIGHT, -16, LV_SYMBOL_UP);
        _armStopButton = _createButton(screen, LV_ALIGN_BOTTOM_MID, 0, LV_SYMBOL_PLAY);
        lv_obj_add_event_cb(_downButton, _downPressed, LV_EVENT_CLICKED, this);
        lv_obj_add_event_cb(_upButton, _upPressed, LV_EVENT_CLICKED, this);
        lv_obj_add_event_cb(_armStopButton, _armStopPressed, LV_EVENT_CLICKED, this);

        lv_obj_add_flag(_downButton, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(_upButton, LV_OBJ_FLAG_HIDDEN);

        _spinner = lv_spinner_create(screen);
        lv_obj_set_width(_spinner, 32);
        lv_obj_set_height(_spinner, 32);
        lv_obj_align(_spinner, LV_ALIGN_BOTTOM_MID, 0, -2);
        lv_obj_add_flag(_spinner, LV_OBJ_FLAG_HIDDEN);
    
        bsp_display_unlock();
#endif
    }

    void PrinterCPAP::_setupButtons(void) {
#if defined(BSP_CAPS_BUTTONS) && BSP_CAPS_BUTTONS
        _configurePhysicalButton(BSP_BUTTON_MIDDLE, &_middleButton, _middleButtonPressed);
        _configurePhysicalButton(BSP_BUTTON_LEFT, &_leftButton, _leftButtonPressed);
        _configurePhysicalButton(BSP_BUTTON_RIGHT, &_rightButton, _rightButtonPressed);
#endif
    }

#if defined(BSP_CAPS_DISPLAY) && BSP_CAPS_DISPLAY
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
#endif

#if defined(BSP_CAPS_BUTTONS) && BSP_CAPS_BUTTONS
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
#endif

    void PrinterCPAP::_setupMotor(void) {
    }

    void PrinterCPAP::_setupFanInput(void) {
        _fanInput.start();
    }

    void PrinterCPAP::_loop(void) {
        if (_motor.isArmed()) {
            _motor.setThrottle(_fanInput.dutyCyclePercentage());
        }

#if defined(BSP_CAPS_DISPLAY) && BSP_CAPS_DISPLAY
        bsp_display_lock(0);
        
        lv_label_set_text_fmt(_throttleLabel, "%u%%", _motor.throttle());
        lv_task_handler();

        bsp_display_unlock();
#endif

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

#if defined(BSP_CAPS_DISPLAY) && BSP_CAPS_DISPLAY
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
#endif

#if defined(BSP_CAPS_BUTTONS) && BSP_CAPS_BUTTONS
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
#endif
}
