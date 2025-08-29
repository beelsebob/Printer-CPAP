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
    class FanControlUI : public TestUI {
    public:
        FanControlUI();
        ~FanControlUI();

        virtual void uiWillBecomeActive(void) override;
        virtual void uiDidBecomeActive(void) override;
        virtual void uiWillBecomeInactive(void) override;
        virtual void uiDidBecomeInactive(void) override;

        virtual void uiWillUpdate(void) override;
        virtual void updateUI(void) override;

        virtual const std::string& name(void) const { return _name; }

        virtual void deleteRootWidget(void) override;

    private:
        virtual void _createRootWidget(lv_obj_t* parent);

        void _setupMotor(void);
        void _setupFanInput(void);

        std::string _throttleText(void) const;
        void _loop(void);

        void _armCompleted(void);

        lv_obj_t* _throttleLabel = nullptr;

        std::unique_ptr<BLHeliESC> _motor;
        std::unique_ptr<FanInput> _fanInput = nullptr;

        const std::string _name = "Fan Control";
    };
}  // namespace pcp
