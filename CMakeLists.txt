# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.1)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(beebox C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(beebox 
    main.c
    src/network/thingsboard.c
    src/network/thingsboard_tls.c
    src/sensors/dht22/dht22.c
    src/sensors/mpu6050/mpu6050.c
    src/sensors/ds18b20/ds18b20.c
    src/sensors/hx711/hx711.c
    src/sensors/ds1302/ds1302.c
    src/utils/utils.c
    src/sensors/microsd/hw_config.c
    src/sensors/microsd/microsd.c
)

# Tell CMake where to find the executable source file
add_subdirectory(lib/FatFs_SPI build)

pico_set_program_name(beebox "beebox")
pico_set_program_version(beebox "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(beebox 0)
pico_enable_stdio_usb(beebox 1)

# Add the standard library to the build
target_link_libraries(beebox
        pico_stdlib
        FatFs_SPI
)

# Add the standard include files to the build
target_include_directories(beebox PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/network/tls_config
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/src/sensors/dht22
        ${CMAKE_CURRENT_LIST_DIR}/src/sensors/ds18b20
        ${CMAKE_CURRENT_LIST_DIR}/src/sensors/mpu6050
        ${CMAKE_CURRENT_LIST_DIR}/src/sensors/hx711
        ${CMAKE_CURRENT_LIST_DIR}/src/sensors/ds1302
        ${CMAKE_CURRENT_LIST_DIR}/src/sensors/microsd
        ${CMAKE_CURRENT_LIST_DIR}/src/utils
        ${CMAKE_CURRENT_LIST_DIR}/src/network
        ${CMAKE_CURRENT_LIST_DIR}/src/network/tls_config
)

# Add any user requested libraries
target_link_libraries(beebox
        hardware_spi
        hardware_i2c
        pico_cyw43_arch_lwip_threadsafe_background
        pico_lwip_mbedtls
        pico_mbedtls
)

target_compile_definitions(beebox PRIVATE
    ALTCP_MBEDTLS_AUTHMODE=MBEDTLS_SSL_VERIFY_REQUIRED
)

pico_add_extra_outputs(beebox)