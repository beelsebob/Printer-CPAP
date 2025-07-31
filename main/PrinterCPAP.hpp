#pragma once

#include "Motor.hpp"

#include "lvgl.h"

#include "iot_button.h"

#include <mutex>
#include <queue>

using mcpwm_cmpr_handle_t = struct mcpwm_cmpr_t*;

namespace pcp {
    class PrinterCPAP {
    public:
        PrinterCPAP();
        ~PrinterCPAP();
        void run(void);

        void armStopPressed();
        void downPressed();
        void upPressed();

    private:
        void _turnOffSpeaker(void);
        void _setupScreen(void);

        lv_obj_t* _createButton(lv_obj_t* parent, lv_align_t align, int32_t x, const char* text);
        void _configurePhysicalButton(const std::optional<uint8_t>& gpio, button_handle_t* buttonHandle, button_cb_t callback);

        void _loop(void);

        void _armCompleted(void);
        void _unarmCompleted(void);

        lv_obj_t* _throttleLabel = nullptr;
        lv_obj_t* _downButton = nullptr;
        lv_obj_t* _upButton = nullptr;
        lv_obj_t* _armStopButton = nullptr;
        lv_obj_t* _spinner = nullptr;
        button_handle_t _middleButton = nullptr;
        button_handle_t _leftButton = nullptr;
        button_handle_t _rightButton = nullptr;
        
        std::mutex _queuedActionsMutex;
        std::queue<std::function<void(void)>> _queuedActions;

        Motor<1000, 2000> _motor;
        bool _motorIsArmed = false;

        friend void _middleButtonPressed(void* buttonHandle, void* userData);
        friend void _leftButtonPressed(void* buttonHandle, void* userData);
        friend void _rightButtonPressed(void* buttonHandle, void* userData);
    };
}
