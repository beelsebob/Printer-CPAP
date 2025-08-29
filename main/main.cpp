#include "UIs/RootUI.hpp"

#include "esp_timer.h"

#if defined(BSP_CAPS_DISPLAY) && BSP_CAPS_DISPLAY
#define USER_INTERFACE 1
#endif

void init(void) {
    // esp_timer_init();
}

extern "C" void app_main(void) {
    init();

#if USER_INTERFACE
    pcp::RootUI rootUI;
    rootUI.run();
#endif
}
