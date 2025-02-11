cmake_minimum_required(VERSION 3.13)

# Set the PICO_BOARD variable
set(PICO_BOARD pico_w)

# Include the Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)


# Define the project
project(blink C CXX ASM)

# Set the C and C++ standards
set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 20)

# Initialize the SDK
pico_sdk_init()

add_subdirectory(src)
