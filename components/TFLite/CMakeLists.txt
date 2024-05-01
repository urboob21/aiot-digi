FILE(GLOB_RECURSE app_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.*)

idf_component_register(SRCS ${app_sources}
                    INCLUDE_DIRS "."
                    REQUIRES esp_http_server espressif__esp-tflite-micro espressif__esp32-camera Helper esp_timer FlowControl Image time_sntp)