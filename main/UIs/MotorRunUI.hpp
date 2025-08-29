#pragma once

#include "ESC/BLHeli/BLHeliESC.hpp"
#include "FanInput.hpp"
#include "TestUI.hpp"

#include "lvgl.h"

#if defined(BSP_CAPS_BUTTONS) && BSP_CAPS_BUTTONS
#include "iot_button.h"
#endif

#include <mutex>
#include <queue>

using mcpwm_cmpr_handle_t = struct mcpwm_cmpr_t*;

namespace pcp {
    class MotorRunUI : public TestUI {
    public:
        MotorRunUI();
        ~MotorRunUI();

        virtual void uiWillBecomeActive(void) override;
        virtual void uiDidBecomeActive(void) override;
        virtual void uiWillBecomeInactive(void) override;
        virtual void uiDidBecomeInactive(void) override;

        virtual void uiWillUpdate(void) override;
        virtual void updateUI(void) override;

        virtual const std::string& name(void) const { return _name; }

        void armStopPressed();
        void downPressed();
        void upPressed();

        virtual void deleteRootWidget(void) override;

    private:
        virtual void _createRootWidget(lv_obj_t* parent);

        void _setupButtons(void);
        void _setupMotor(void);

        lv_obj_t* _createButton(lv_obj_t* parent, lv_align_t align, int32_t x, const char* text);
#if defined(BSP_CAPS_BUTTONS) && BSP_CAPS_BUTTONS
        void _configurePhysicalButton(const std::optional<uint8_t>& gpio, button_handle_t* buttonHandle, button_cb_t callback);
#endif

        std::string _throttleText(void) const;
        void _loop(void);

        void _armCompleted(void);
        void _disarmCompleted(void);

        lv_obj_t* _throttleLabel = nullptr;
        lv_obj_t* _downButton = nullptr;
        lv_obj_t* _upButton = nullptr;
        lv_obj_t* _armStopButton = nullptr;
        lv_obj_t* _spinner = nullptr;
#if defined(BSP_CAPS_BUTTONS) && BSP_CAPS_BUTTONS
        button_handle_t _middleButton = nullptr;
        button_handle_t _leftButton = nullptr;
        button_handle_t _rightButton = nullptr;
#endif

        std::mutex _queuedActionsMutex;
        std::queue<std::function<void(void)>> _queuedActions;

        std::unique_ptr<BLHeliESC> _motor = nullptr;

#if defined(BSP_CAPS_BUTTONS) && BSP_CAPS_BUTTONS
        friend void _middleButtonPressed(void* buttonHandle, void* userData);
        friend void _leftButtonPressed(void* buttonHandle, void* userData);
        friend void _rightButtonPressed(void* buttonHandle, void* userData);
#endif

        const std::string _name = "Run Motor";
    };
}  // namespace pcp
