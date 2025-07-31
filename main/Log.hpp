#include "esp_log.h"

#pragma once

namespace pcp {
    static const char* _logTag = "PrinterCPAP";
}

#define PCP_DRAM_LOGD(str, ...) ESP_DRAM_LOGD(pcp::_logTag, "%s:%d " str, __FILE__, __LINE__, ##__VA_ARGS__)
#define PCP_DRAM_LOGI(str, ...) ESP_DRAM_LOGI(pcp::_logTag, "%s:%d " str, __FILE__, __LINE__, ##__VA_ARGS__)
#define PCP_DRAM_LOGW(str, ...) ESP_DRAM_LOGW(pcp::_logTag, "%s:%d " str, __FILE__, __LINE__, ##__VA_ARGS__)
#define PCP_DRAM_LOGE(str, ...) ESP_DRAM_LOGE(pcp::_logTag, "%s:%d " str, __FILE__, __LINE__, ##__VA_ARGS__)

#define PCP_LOGD(str, ...) ESP_LOGD(pcp::_logTag, "%s:%d " str, __FILE__, __LINE__, ##__VA_ARGS__)
#define PCP_LOGI(str, ...) ESP_LOGI(pcp::_logTag, "%s:%d " str, __FILE__, __LINE__, ##__VA_ARGS__)
#define PCP_LOGW(str, ...) ESP_LOGW(pcp::_logTag, "%s:%d " str, __FILE__, __LINE__, ##__VA_ARGS__)
#define PCP_LOGE(str, ...) ESP_LOGE(pcp::_logTag, "%s:%d " str, __FILE__, __LINE__, ##__VA_ARGS__)
