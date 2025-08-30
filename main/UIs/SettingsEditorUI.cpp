#include "SettingsEditorUI.hpp"

#include "Log.hpp"

namespace pcp {
    void drawEvent(lv_event_t* event) {
        SettingsEditorUI* settingsEditor = reinterpret_cast<SettingsEditorUI*>(lv_event_get_user_data(event));
        settingsEditor->_tableDrawEvent(event);
    }

    void SettingsEditorUI::uiDidBecomeActive(void) {
        _enterProgrammingMode();
    }

    void SettingsEditorUI::uiWillBecomeInactive(void) {
        _esc = nullptr;
    }

    void SettingsEditorUI::_enterProgrammingMode(void) {
        _esc = std::make_unique<BLHeliESC>();
        _esc->enterProgrammingMode([this](std::optional<BLHeliESCConfig> escConfig) {
            _config = escConfig;
            _configChanged = true;
        });
    }

    struct SettingValue {
        BLHeliESCSetting setting;
        uint8_t value;
    };

    void SettingsEditorUI::updateUI(void) {
        if (!_configChanged || !_config.has_value()) {
            return;
        }

        static constexpr size_t kDataColumnWidth = 110;
        static constexpr size_t kBorderWidths = 4;
        const size_t width = lv_obj_get_width(_dataTable);
        const size_t settingNameColumnWidth = width - kDataColumnWidth - kBorderWidths;

        bsp_display_lock(0);
        lv_obj_add_flag(_spinner, LV_OBJ_FLAG_HIDDEN);

        const BLHeliESCConfig deviceConfig = _config.value();
        const std::unordered_map<BLHeliESCSetting, uint8_t>& settings = deviceConfig.settings();

        std::vector<BLHeliESCSetting> sortedSettings;
        sortedSettings.reserve(settings.size());
        for (auto kv : settings) {
            sortedSettings.push_back(kv.first);
        }
        std::sort(sortedSettings.begin(), sortedSettings.end());

        lv_obj_remove_flag(_dataTable, LV_OBJ_FLAG_HIDDEN);
        lv_table_set_row_count(_dataTable, settings.size() + 1);
        lv_table_set_column_count(_dataTable, 2);
        lv_table_set_cell_ctrl(_dataTable, 0, 0, LV_TABLE_CELL_CTRL_MERGE_RIGHT);
        lv_table_set_cell_value_fmt(_dataTable, 0, 0, "%s\n%s", deviceConfig.prettyLayout().c_str(), deviceConfig.name().c_str());
        lv_obj_set_style_text_font(_dataTable, &lv_font_montserrat_18, 0);
        lv_table_set_col_width(_dataTable, 1, kDataColumnWidth);
        lv_table_set_col_width(_dataTable, 0, settingNameColumnWidth);
        for (size_t i = 0; i < sortedSettings.size(); ++i) {
            const BLHeliESCSetting setting = sortedSettings[i];
            const uint8_t value = settings.at(setting);
            lv_table_set_cell_value(_dataTable, i + 1, 0, std::to_string(setting).c_str());
            lv_table_set_cell_value(_dataTable, i + 1, 1, setting_to_string(setting, value).c_str());
            if (value == defaultValueForSetting(setting)) {
                lv_table_set_cell_ctrl(_dataTable, i + 1, 1, LV_TABLE_CELL_CTRL_CUSTOM_1);
            }
        }
        bsp_display_unlock();
    }

    void SettingsEditorUI::_tableDrawEvent(lv_event_t* event) {
        lv_draw_task_t* drawTask = lv_event_get_draw_task(event);
        lv_draw_dsc_base_t* baseDescription = (lv_draw_dsc_base_t*)lv_draw_task_get_draw_dsc(drawTask);

        if (baseDescription->part == LV_PART_ITEMS) {
            const uint32_t row = baseDescription->id1;
            const uint32_t col = baseDescription->id2;

            lv_draw_label_dsc_t* labelDrawDescription = lv_draw_task_get_label_dsc(drawTask);
            lv_draw_fill_dsc_t* fillDrawDescription = lv_draw_task_get_fill_dsc(drawTask);

            if (labelDrawDescription != nullptr) {
                labelDrawDescription->font = &lv_font_montserrat_18;
            }

            if (row == 0) {
                if (labelDrawDescription != nullptr) {
                    labelDrawDescription->align = LV_TEXT_ALIGN_CENTER;
                    labelDrawDescription->font = &lv_font_montserrat_24;
                    labelDrawDescription->ofs_y = -5;
                }

                if (fillDrawDescription != nullptr) {
                    fillDrawDescription->color = lv_color_mix(lv_palette_main(LV_PALETTE_BLUE), fillDrawDescription->color, LV_OPA_20);
                    fillDrawDescription->opa = LV_OPA_COVER;
                }
            } else if (col == 1) {
                lv_draw_label_dsc_t* labelDrawDescription = lv_draw_task_get_label_dsc(drawTask);
                if (labelDrawDescription != nullptr) {
                    labelDrawDescription->align = LV_TEXT_ALIGN_RIGHT;
                }
            }

            if (lv_table_has_cell_ctrl(_dataTable, row, col, LV_TABLE_CELL_CTRL_CUSTOM_1)) {
                if (labelDrawDescription != nullptr) {
                    labelDrawDescription->color = lv_palette_darken(LV_PALETTE_GREEN, row % 2 == 0 ? 4 : 1);
                }
            }

            if (row != 0 && row % 2 == 0) {
                if (fillDrawDescription != nullptr) {
                    fillDrawDescription->color = lv_color_mix(lv_palette_main(LV_PALETTE_GREY), fillDrawDescription->color, LV_OPA_10);
                    fillDrawDescription->opa = LV_OPA_COVER;
                }
            }
        }
    }

    void SettingsEditorUI::deleteRootWidget(void) {
        if (_rootWidget != nullptr) {
            bsp_display_lock(0);
            lv_obj_delete(_rootWidget);
            bsp_display_unlock();
            _rootWidget = nullptr;
        }
    }

    void SettingsEditorUI::_createRootWidget(lv_obj_t* parent) {
        bsp_display_lock(0);
        {
            _rootWidget = lv_obj_create(parent);
            lv_obj_set_size(_rootWidget, lv_pct(100), lv_pct(100));
            lv_obj_set_pos(_rootWidget, 0, 0);
            lv_obj_set_style_pad_all(_rootWidget, 0, 0);

            _dataTable = lv_table_create(_rootWidget);
            lv_obj_set_size(_dataTable, lv_pct(100), lv_pct(100));
            lv_obj_center(_dataTable);
            lv_obj_add_flag(_dataTable, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_event_cb(_dataTable, drawEvent, LV_EVENT_DRAW_TASK_ADDED, this);
            lv_obj_add_flag(_dataTable, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);

            _spinner = lv_spinner_create(_rootWidget);
            lv_obj_set_size(_spinner, 96, 96);
            lv_obj_center(_spinner);
        }
        bsp_display_unlock();
    }
}  // namespace pcp
