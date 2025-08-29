#pragma once

#include "TestUI.hpp"

#include <memory>
#include <tuple>
#include <vector>

#if !defined(BSP_CAPS_DISPLAY) || !BSP_CAPS_DISPLAY
#error "UIs require a device with a BSP Display"
#endif

namespace pcp {
    class RootUI : TestUI {
    public:
        RootUI();
        ~RootUI();

        void run(void);

        virtual void uiWillBecomeActive(void) override;
        virtual void uiDidBecomeActive(void) override;
        virtual void uiWillBecomeInactive(void) override;
        virtual void uiDidBecomeInactive(void) override;

        virtual void uiWillUpdate() override;
        virtual void updateUI() override;
        virtual void uiDidUpdate() override;

        virtual void deleteRootWidget(void) override;

        virtual const std::string& name(void) const { return _name; }

        TestUI* activeUI(void) { return _activeUIIndex == std::numeric_limits<size_t>::max() ? nullptr : _testUIs[_activeUIIndex].get(); }

    private:
        struct MenuEventInfo {
            RootUI* rootUI;
            size_t idx;
        };

        void _turnOffSpeaker(void);
        lv_obj_t* _setupScreen(void);
        void _createRootWidget(lv_obj_t* parent);

        void _testUIRun(size_t idx);
        void _menuBack();

        void _loop(void);

        lv_obj_t* _screen;
        lv_obj_t* _menuPage;
        std::vector<lv_obj_t*> _menuPages;
        std::vector<lv_obj_t*> _menuContents;
        std::vector<std::unique_ptr<MenuEventInfo>> _userInfos;

        std::vector<std::unique_ptr<TestUI>> _testUIs;
        size_t _activeUIIndex = std::numeric_limits<size_t>::max();

        const std::string _name = "ESC Control";

        friend void testUIRun(lv_event_t* event);
        friend void menuBack(lv_event_t* event);
    };
}  // namespace pcp
