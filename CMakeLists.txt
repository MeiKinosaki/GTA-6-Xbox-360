cmake_minimum_required(VERSION 3.25)
project(gta6_x360 C)

add_executable(gta6 src/main.c)
set_target_properties(gta6 PROPERTIES C_STANDARD 17 C_EXTENSIONS ON)
target_compile_options(gta6 PRIVATE -Wall -Wextra -Werror)

# XSDK links the title and packages it as XEX2 after linking.
xsdk_add_xex(gta6
    TITLE_ID 0x47544136
    NAME "Grand Theft Auto 6 Xbox 360"
    VERSION 0.1.0.0
)
