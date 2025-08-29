#include "RootUI.hpp"

#include "FanControlUI.hpp"
#include "MotorRunUI.hpp"
#include "SettingsEditorUI.hpp"

#include "Log.hpp"

#include "bsp/esp-bsp.h"

#include <numeric>

namespace pcp {
    void testUIRun(lv_event_t* event) {
        RootUI::MenuEventInfo* eventInfo = reinterpret_cast<RootUI::MenuEventInfo*>(lv_event_get_user_data(event));
        eventInfo->rootUI->_testUIRun(eventInfo->idx);
    }

    void menuBack(lv_event_t* event) {
        RootUI* rootUI = reinterpret_cast<RootUI*>(lv_event_get_user_data(event));
        rootUI->_menuBack();
    }

    RootUI::RootUI() : TestUI() {
        _turnOffSpeaker();
        _setupScreen();

        _testUIs.emplace_back(new MotorRunUI());
        _testUIs.emplace_back(new FanControlUI());
        _testUIs.emplace_back(new SettingsEditorUI());

        _createRootWidget(_screen);
        bsp_display_lock(0);
        uiWillBecomeActive();
        uiDidBecomeActive();
        bsp_display_unlock();
    }

    RootUI::~RootUI() {
        for (lv_obj_t* obj : _menuContents) {
            lv_obj_delete(obj);
        }
        if (_menuPage != nullptr) {
            lv_obj_delete(_menuPage);
        }
    }

    void RootUI::_turnOffSpeaker(void) {
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

    lv_obj_t* RootUI::_setupScreen(void) {
        bsp_display_cfg_t displayConfig = {.lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
                                           .buffer_size = BSP_LCD_DRAW_BUFF_SIZE,
                                           .double_buffer = BSP_LCD_DRAW_BUFF_DOUBLE,
                                           .flags = {
                                               .buff_dma = true,
                                               .buff_spiram = false,
                                           }};
        displayConfig.lvgl_port_cfg.task_stack = 16384;
        bsp_display_start_with_config(&displayConfig);
        bsp_display_backlight_on();
        bsp_display_lock(0);
        { _screen = lv_screen_active(); }
        bsp_display_unlock();
        return _screen;
    }

    void RootUI::deleteRootWidget(void) {
        _menuPage = nullptr;
        _menuPages.erase(_menuPages.begin(), _menuPages.end());
        _menuContents.erase(_menuContents.begin(), _menuContents.end());
        if (_rootWidget != nullptr) {
            bsp_display_lock(0);
            lv_obj_delete(_rootWidget);
            bsp_display_unlock();
            _rootWidget = nullptr;
        }
    }

    void RootUI::_createRootWidget(lv_obj_t* parent) {
        bsp_display_lock(0);
        {
            _rootWidget = lv_menu_create(parent);
            lv_obj_set_size(_rootWidget, lv_pct(100), lv_pct(100));
            lv_obj_center(_rootWidget);
            lv_obj_add_event_cb(_rootWidget, menuBack, LV_EVENT_CLICKED, this);
            lv_obj_set_style_pad_ver(lv_menu_get_main_header(_rootWidget), 12, 0);
            lv_color_t bg_color = lv_obj_get_style_bg_color(_rootWidget, 0);

            lv_obj_set_style_pad_hor(_rootWidget, lv_obj_get_style_pad_left(lv_menu_get_main_header(_rootWidget), 0), 0);
            lv_obj_set_style_bg_color(_rootWidget, lv_color_darken(lv_obj_get_style_bg_color(_rootWidget, 0), lv_color_brightness(bg_color) > 127 ? 10 : 50),
                                      0);

            _menuPage = lv_menu_page_create(_rootWidget, name().c_str());
            lv_obj_t* section = lv_menu_section_create(_menuPage);
            _menuContents.push_back(section);
            size_t idx = 0;
            for (std::unique_ptr<TestUI>& testUI : _testUIs) {
                const char* cName = testUI->name().c_str();

                lv_obj_t* menuContents = lv_menu_cont_create(section);
                _menuContents.push_back(menuContents);
                lv_obj_set_style_pad_ver(menuContents, 8, 0);
                lv_obj_t* label = lv_label_create(menuContents);
                _menuContents.push_back(label);
                lv_label_set_text(label, cName);

                lv_obj_t* page = lv_menu_page_create(_rootWidget, cName);
                _menuPages.push_back(page);
                _menuContents.push_back(page);

                _userInfos.emplace_back(new MenuEventInfo({.rootUI = this, .idx = idx}));
                lv_obj_add_event_cb(label, testUIRun, LV_EVENT_CLICKED, (void*)_userInfos.back().get());
                lv_obj_add_flag(label, LV_OBJ_FLAG_CLICKABLE);
                ++idx;
            }

            lv_menu_set_page(_rootWidget, _menuPage);
        }
        bsp_display_unlock();
    }

    void RootUI::_testUIRun(size_t idx) {
        bsp_display_lock(0);
        std::unique_ptr<TestUI>& testUI = _testUIs[idx];
        testUI->uiWillBecomeActive();
        uiWillBecomeInactive();
        lv_menu_set_page(_rootWidget, _menuPages[idx]);
        lv_obj_set_parent(testUI->rootWidget(_menuPages[idx]), _menuPages[idx]);
        _activeUIIndex = idx;
        testUI->uiDidBecomeActive();
        uiDidBecomeInactive();
        bsp_display_unlock();
    }

    void RootUI::_menuBack() {
        TestUI* activeTestUI = activeUI();
        if (activeTestUI != nullptr) {
            bsp_display_lock(0);
            activeTestUI->uiWillBecomeInactive();
            uiWillBecomeActive();
            lv_menu_set_page(_rootWidget, _menuPage);
            _activeUIIndex = std::numeric_limits<size_t>::max();
            activeTestUI->deleteRootWidget();
            activeTestUI->uiDidBecomeInactive();
            uiWillBecomeActive();
            bsp_display_unlock();
        }
    }

    void RootUI::run(void) {
        while (true) {
            _loop();
        }
    }

    void RootUI::uiWillBecomeActive(void) {
        TestUI::uiWillBecomeActive();
    }

    void RootUI::uiDidBecomeActive(void) {
        gpio_config_t motorOutConfig = {
            .pin_bit_mask = 1 << kMotorOutputGPIO,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&motorOutConfig);
        gpio_set_level(kMotorOutputGPIO, 0);

        TestUI::uiDidBecomeActive();
    }

    void RootUI::uiWillBecomeInactive(void) {
        gpio_reset_pin(kMotorOutputGPIO);
        TestUI::uiWillBecomeInactive();
    }

    void RootUI::uiDidBecomeInactive(void) {
        TestUI::uiDidBecomeInactive();
    }

    void RootUI::uiWillUpdate() {
        TestUI* child = activeUI();
        if (child != nullptr) {
            child->uiWillUpdate();
        }
    }

    void RootUI::updateUI() {
        TestUI* child = activeUI();
        if (child != nullptr) {
            child->updateUI();
        }
        TestUI::updateUI();
    }

    void RootUI::uiDidUpdate() {
        TestUI* child = activeUI();
        if (child != nullptr) {
            child->uiDidUpdate();
        }
    }

    void RootUI::_loop(void) {
        uiWillUpdate();

        bsp_display_lock(0);
        {
            updateUI();
            lv_task_handler();
        }
        bsp_display_unlock();

        uiDidUpdate();

        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}  // namespace pcp
