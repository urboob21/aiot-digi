
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
   # 0. ignore files when commit
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



   # 1. Each component needs to declare the components that it depend on 
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