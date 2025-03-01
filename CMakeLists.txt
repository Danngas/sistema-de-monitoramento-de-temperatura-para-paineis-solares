# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================




cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PICO_BOARD pico CACHE STRING "Board type")

include(pico_sdk_import.cmake)
project(System_Monitor_Temp_PV C CXX ASM)
pico_sdk_init()

add_executable(System_Monitor_Temp_PV
    System_Monitor_Temp_PV.c
    lib/graphics.c
    lib/ssd1306.c
)

pico_set_program_name(System_Monitor_Temp_PV "System_Monitor_Temp_PV")
pico_set_program_version(System_Monitor_Temp_PV "0.1")
pico_enable_stdio_uart(System_Monitor_Temp_PV 0)
pico_enable_stdio_usb(System_Monitor_Temp_PV 1)

# Link com as bibliotecas necessárias
target_link_libraries(System_Monitor_Temp_PV pico_stdlib hardware_i2c hardware_adc hardware_pwm hardware_gpio pico_bootsel_via_double_reset pico_bootrom)

# Adicione o diretório atual aos caminhos de inclusão
target_include_directories(System_Monitor_Temp_PV PRIVATE ${CMAKE_CURRENT_LIST_DIR})

pico_add_extra_outputs(System_Monitor_Temp_PV)

