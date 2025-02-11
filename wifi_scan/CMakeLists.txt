cmake_minimum_required(VERSION 3.13)

# Set the PICO_BOARD variable
set(PICO_BOARD pico_w)

# Include the Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Define the project
project(wifi_scan C CXX ASM)


# Set the C and C++ standards
set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME}
    main.c
)

target_link_libraries(${PROJECT_NAME} 
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
)
	
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${PROJECT_SOURCE_DIR}/include
)
     
if (NOT DEFINED ENV{WIFI_SSID})
	message(FATAL_ERROR "WIFI_SSID env var not set")
endif()
if (NOT DEFINED ENV{WIFI_PASSWORD})
	message(FATAL_ERROR "WIFI_PASSWORD env var not set")
endif()


target_compile_definitions(${PROJECT_NAME} PRIVATE
    WIFI_SSID=\"$ENV{WIFI_SSID}\"
    WIFI_PASSWORD=\"$ENV{WIFI_PASSWORD}\"
)

# Initialize the Pico SDK
pico_sdk_init()

# Add extra outputs (e.g., UF2 file)
pico_add_extra_outputs(${PROJECT_NAME})

# Enable USB and UART for the executable
pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)
