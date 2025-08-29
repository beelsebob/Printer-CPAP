#include "FanControlUI.hpp"

#include "Log.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace pcp {
    FanControlUI::FanControlUI() : TestUI() {}

    FanControlUI::~FanControlUI() {
        if (_fanInput != nullptr) {
            _fanInput->stop();
        }
        if (_motor != nullptr) {
            _motor->disarm();
        }
        deleteRootWidget();
    }

    void FanControlUI::deleteRootWidget(void) {
        _throttleLabel = nullptr;
        if (_rootWidget != nullptr) {
            bsp_display_lock(0);
            lv_obj_delete(_rootWidget);
            bsp_display_unlock();
            _rootWidget = nullptr;
        }
    }

    void FanControlUI::_createRootWidget(lv_obj_t* parent) {
        bsp_display_lock(0);

        _rootWidget = lv_obj_create(parent);
        lv_obj_set_size(_rootWidget, lv_pct(100), 180);
        lv_obj_set_pos(_rootWidget, 0, 0);

        _throttleLabel = lv_label_create(_rootWidget);
        lv_obj_set_size(_throttleLabel, lv_pct(100), lv_pct(66));
        lv_obj_set_pos(_throttleLabel, 0, 32);
        lv_obj_set_style_text_align(_throttleLabel, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_font(_throttleLabel, &lv_font_montserrat_44, LV_STATE_DEFAULT);

        bsp_display_unlock();
    }

    void FanControlUI::_setupMotor(void) {
        _motor = std::make_unique<BLHeliESC>();
        _motor->arm();
    }

    void FanControlUI::_setupFanInput(void) {
        _fanInput = std::make_unique<FanInput>();
        _fanInput->start();
    }

    void FanControlUI::uiWillBecomeActive(void) {}

    void FanControlUI::uiDidBecomeActive(void) {
        _setupMotor();
        _setupFanInput();
    }

    void FanControlUI::uiWillBecomeInactive(void) {
        _motor->disarm();
        _motor = nullptr;
        _fanInput->stop();
        _fanInput = nullptr;
    }

    void FanControlUI::uiDidBecomeInactive(void) {}

    void FanControlUI::uiWillUpdate(void) {
        assert(_motor != nullptr);
        assert(_fanInput != nullptr);

        if (_motor->isArmed()) {
            _motor->setThrottle(_fanInput->dutyCyclePercentage());
        }
    }

    void FanControlUI::updateUI(void) {
        if (_motor == nullptr) {
            assert(false && "Update UI called with no motor connection established.");
            lv_label_set_text(_throttleLabel, "<ERROR>");
        } else {
            lv_label_set_text(_throttleLabel, _motor->stateString().c_str());
        }

        TestUI::updateUI();
    }
}  // namespace pcp
