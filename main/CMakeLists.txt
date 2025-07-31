file(GLOB_RECURSE SRC
     "main.cpp"
     "PrinterCPAP.cpp")

idf_component_register(SRCS ${SRC}
                       INCLUDE_DIRS "."
                       REQUIRES driver esp_driver_mcpwm lvgl m5stack_core)