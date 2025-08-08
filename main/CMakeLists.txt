file(GLOB_RECURSE SRC "*.cpp")
     # "main.cpp"
     # "PrinterCPAP.cpp")

set(REQ)

set(PRIVREQ 
     driver 
     esp_driver_mcpwm 
     lvgl
     button
)

if (CONFIG_PCP_HW_M5_BASIC)
  list(APPEND REQ m5stack_core espressif__m5stack_core)
endif()

message(STATUS "REQUIRES components: ${REQ}")
message(STATUS "PRIV_REQUIRES components: ${PRIVREQ}")
# set(REQ ${REQ})
# set(PRIVREQ ${PRIVREQ})

idf_component_register(SRCS ${SRC}
                       INCLUDE_DIRS "."
                       REQUIRES ${REQ}
                       PRIV_REQUIRES ${PRIVREQ}
)
