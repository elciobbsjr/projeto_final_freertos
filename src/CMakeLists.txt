# Cria o executável com main, configuração, tarefas e drivers de sensores
add_executable(picow_freertos
    # Arquivos principais
    main.c
    config_geral.c

    # Tarefas (FreeRTOS)
    tasks/task_giroscopio_mpu6500.c
    tasks/task_temperatura_aht10.c
    tasks/task_distancia_vl53l0x.c
    tasks/task_emergencia.c  # <-- Adicione isso!

    # Drivers de sensores
    drivers/mpu6500.c
    drivers/aht10.c
    drivers/vl53l0x.c
)


# Corrige a saída para build/ em vez de build/src/
set_target_properties(picow_freertos PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Adiciona o heap_4.c do FreeRTOS (alocação de memória)
target_sources(picow_freertos
    PRIVATE
    ${FREERTOS_KERNEL_PATH}/portable/MemMang/heap_4.c
)

# Diretórios de include (onde estão os headers dos sensores, tarefas e config)
target_include_directories(picow_freertos PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}             # src/
    ${CMAKE_CURRENT_LIST_DIR}/../inc     # inc/ (headers personalizados)
)

# Bibliotecas necessárias
target_link_libraries(picow_freertos 
    pico_stdlib
    hardware_i2c
    FreeRTOS-Kernel
)

# Gera os binários UF2, HEX e BIN automaticamente
pico_add_extra_outputs(picow_freertos)
