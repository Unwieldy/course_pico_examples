cmake_minimum_required(VERSION 3.13)

# Set the PICO_BOARD variable
set(PICO_BOARD pico_w)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Define the project
project(tcp_client C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the Pico SDK
pico_sdk_init()

add_executable(${PROJECT_NAME} tcp_client.c)

# pull in common dependencies
target_link_libraries(${PROJECT_NAME}
        pico_cyw43_arch_lwip_threadsafe_background
        pico_stdlib
)

target_include_directories(tcp_client PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
)

# enable usb or uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(${PROJECT_NAME})
