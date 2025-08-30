#pragma once

#include "ESC/BLHeli/BLHeliESC.hpp"
#include "TestUI.hpp"

#include "lvgl.h"

#include <unordered_map>

namespace pcp {
    class SettingsEditorUI : public TestUI {
    public:
        SettingsEditorUI() : TestUI() {}

        ~SettingsEditorUI() {}

        virtual void uiDidBecomeActive(void) override;
        virtual void uiWillBecomeInactive(void) override;

        virtual void updateUI(void) override;

        virtual const std::string& name(void) const { return _name; };

        virtual void deleteRootWidget(void) override;

    private:
        virtual void _createRootWidget(lv_obj_t* parent) override;

        void _enterProgrammingMode(void);
        void _tableDrawEvent(lv_event_t* event);

        std::unique_ptr<BLHeliESC> _esc = nullptr;

        std::optional<BLHeliESCConfig> _config;
        bool _configChanged = false;

        lv_obj_t* _dataTable;
        lv_obj_t* _spinner;

        const std::string _name = "Edit ESC Settings";

        friend void drawEvent(lv_event_t*);
    };
}  // namespace pcp
