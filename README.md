
1. Structure of the project
    =========================================================================================
- myProject/
             - CMakeLists.txt
             - sdkconfig
             - bootloader_components/ - boot_component/ - CMakeLists.txt
                                                        - Kconfig
                                                        - src1.c
             - components/ - component1/ - CMakeLists.txt
                                         - Kconfig
                                         - src1.c
                           - component2/ - CMakeLists.txt
                                         - Kconfig
                                         - src1.c
                                         - include/ - component2.h
             - main/       - CMakeLists.txt
                           - src1.c
                           - src2.c

             - build/
    ========================================================================================
   # 0. If want to ignore some files when commit
   create <.gitignore> file
{
                # Ignore compiled binaries
                *.exe
                *.dll

                # Ignore log files
                *.log

                # Ignore directories
                /node_modules/
                /build/
}



   # 1. Each component needs to declare the components that it depend on - create / modify CMakeLists.txt file
   CMake.txt 
                ifd_component_register(...
                            REQUIRES name_component1
                            PRIV_REQUIRES name_component2)

_____example______
A - needs B,C,D
B,C needs D
D independent
-> A REQUIRES B C
     PRIV_REQUIRES D

   # 2. How to create the components
   idf_component_register(SRCS "pngle.c" "decode_jpeg.c" "decode_png.c" "fontx.c" "ili9340.c"
                       INCLUDE_DIRS "include"
                       REQUIRES spiffs)
    https://stackoverflow.com/questions/70196263/how-to-add-an-esp-idf-library-to-an-external-library

more effect
_______________________________________________________________________
FILE(GLOB_RECURSE app_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.*)

list(APPEND EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/common_components/protocol_examples_common)

idf_component_register(SRCS ${app_sources}
                    INCLUDE_DIRS "."
                    REQUIRES esp32-camera esp_http_server nvs_flash)

_______________________________________________________________________

   # 3. Add dependencies 
    idf_component.yml


   # 4. ESP32 CAMERA
        4.1. Config for Cam
    CONFIG_ESP32_DEFAULT_CPU_FREQ_240=y

    CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y
    CONFIG_PARTITION_TABLE_OFFSET=0x10000

    CONFIG_FREERTOS_HZ=1000
    CONFIG_ESPTOOLPY_FLASHFREQ_80M=y
    CONFIG_ESPTOOLPY_FLASHMODE_QIO=y

    CONFIG_SPIRAM_SUPPORT=y
    CONFIG_ESP32_SPIRAM_SUPPORT=y
    CONFIG_SPIRAM_SPEED_80M=y
