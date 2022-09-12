cmake_minimum_required(VERSION 3.16)

include(../pico-sdk/pico_sdk_init.cmake)


project(upb-firmware)



pico_sdk_init()

add_executable(${PROJECT_NAME} src/main.cpp)

add_subdirectory(pico-ssd1306)

# Link with the pico stdlib
target_link_libraries(upb-firmware pico_ssd1306 pico_stdlib hardware_adc hardware_i2c)

target_include_directories(upb-firmware
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/../
)
# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${PROJECT_NAME})

# Enable USB serial
pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(upb-firmware 1)
