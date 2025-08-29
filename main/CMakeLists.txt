file(GLOB_RECURSE SRC "*.cpp")

set(REQ)

set(PRIVREQ 
     driver
     esp_common
     esp_driver_mcpwm 
     esp_driver_uart
     lvgl
)

if (CONFIG_PCP_HW_M5_BASIC)
  MESSAGE(STATUS "Penis1")
  list(APPEND REQ m5stack_core espressif__m5stack_core)
  list(APPEND PRIVREQ button)
elseif (CONFIG_PCP_HW_M5_CORE2)
  MESSAGE(STATUS "Penis2")
  list(APPEND REQ m5stack_core_2 espressif__m5stack_core_2)
elseif (CONFIG_PCP_HW_M5_CORES3)
  MESSAGE(STATUS "Penis3")
  list(APPEND REQ m5stack_core_s3 espressif__m5stack_core_s3)
elseif (CONFIG_PCP_HW_GENERIC)
  MESSAGE(STATUS "Penis4")
  list(APPEND REQ penis)
endif()

message(STATUS "REQUIRES components: ${REQ}")
message(STATUS "PRIV_REQUIRES components: ${PRIVREQ}")

set(REQ ${REQ})
set(PRIVREQ ${PRIVREQ})

idf_component_register(SRCS ${SRC}
                       INCLUDE_DIRS "."
                       REQUIRES ${REQ}
                       PRIV_REQUIRES ${PRIVREQ}
)
target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++23)
