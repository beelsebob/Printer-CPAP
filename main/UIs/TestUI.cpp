#include "TestUI.hpp"

namespace pcp {
    TestUI::~TestUI() {
        if (_rootWidget != nullptr) {
            lv_obj_delete(_rootWidget);
        }
    }

    lv_obj_t* TestUI::rootWidget(lv_obj_t* parent) {
        if (_rootWidget == nullptr) {
            _createRootWidget(parent);
        }

        return _rootWidget;
    }

    void TestUI::updateUI(void) {
        lv_task_handler();
    }

    void TestUI::deleteRootWidget(void) {
        if (_rootWidget != nullptr) {
            bsp_display_lock(0);
            lv_obj_delete(_rootWidget);
            bsp_display_unlock();
            _rootWidget = nullptr;
        }
    }
}  // namespace pcp
