#pragma once

#include "lvgl.h"

#include "bsp/esp-bsp.h"

#include <memory>
#include <string>

#if !defined(BSP_CAPS_DISPLAY) || !BSP_CAPS_DISPLAY
#error "UIs require a device with a BSP Display"
#endif

namespace pcp {
    class TestUI {
    public:
        TestUI() {}

        ~TestUI();

        virtual void uiWillUpdate(void) {}

        virtual void updateUI(void);

        virtual void uiDidUpdate(void) {}

        virtual void uiWillBecomeActive(void) {}

        virtual void uiDidBecomeActive(void) {}

        virtual void uiWillBecomeInactive(void) {}

        virtual void uiDidBecomeInactive(void) {}

        virtual const std::string& name(void) const { return _name; };

        lv_obj_t* rootWidget(lv_obj_t* parent);
        virtual void deleteRootWidget(void);

    protected:
        virtual void _createRootWidget(lv_obj_t* parent) = 0;

        lv_obj_t* _rootWidget = nullptr;

        const std::string _name = "";
    };
}  // namespace pcp
