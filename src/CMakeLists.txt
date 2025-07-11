cmake_minimum_required(VERSION 3.13)

# Caminhos para SDK e extras
set(PICO_SDK_PATH "C:/Users/Elcio.SAMSUNGBOOK-2DE/.pico-sdk/sdk/2.1.1")
set(PICO_EXTRAS_PATH "${PICO_SDK_PATH}/lib/pico-extras")

# Importa SDK e extras
include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include(${PICO_EXTRAS_PATH}/external/pico_extras_import.cmake)

# Nome do projeto
project(picow_freertos C CXX ASM)

# Inicializa SDK
pico_sdk_init()

# Cria o executável com os arquivos do projeto
add_executable(picow_freertos
    # Arquivos principais
    main.c
    config_geral.c
    utils_print.c


    # Tarefas
    tasks/task_giroscopio_mpu6500.c
    tasks/task_temperatura_aht10.c
    tasks/task_distancia_vl53l0x.c
    tasks/task_emergencia.c
    tasks/task_wifi.c
    tasks/task_oximetro_max30102.c        # ✅ Nova task adicionada

    # Drivers
    drivers/mpu6500.c
    drivers/aht10.c
    drivers/vl53l0x.c
)

# Define a saída do executável
set_target_properties(picow_freertos PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Adiciona o heap do FreeRTOS
target_sources(picow_freertos
    PRIVATE
    ${FREERTOS_KERNEL_PATH}/portable/MemMang/heap_4.c
)

# Diretórios de includes
target_include_directories(picow_freertos
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}             # src/
    ${CMAKE_CURRENT_LIST_DIR}/../inc     # inc/ (onde está lwipopts.h)
)

# Vincula as bibliotecas necessárias
target_link_libraries(picow_freertos 
    pico_stdlib
    hardware_i2c
    FreeRTOS-Kernel
    pico_cyw43_arch_lwip_threadsafe_background
    hardware_pwm
)

# Gera os arquivos .uf2, .hex e .bin
pico_add_extra_outputs(picow_freertos)
