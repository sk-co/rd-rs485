##
project(rd10)
cmake_minimum_required(VERSION 3.16)
enable_language(ASM)

#---------------------------------------------------------------------------------------
# Target
#---------------------------------------------------------------------------------------

file(GLOB ${PROJECT_NAME}_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/utilities.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/stm32/stm32f3xx/system_stm32f3xx.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/stm32/stm32f3xx/STM32F3xx_HAL_Driver/Src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/startup_stm32f373xc.s"
#    "${CMAKE_CURRENT_SOURCE_DIR}/../../peripherals/memory/sst25vf.cpp"
    )

add_library(${PROJECT_NAME} OBJECT EXCLUDE_FROM_ALL ${${PROJECT_NAME}_SOURCES})

target_compile_definitions(${PROJECT_NAME} PUBLIC -DCORE_CM4 -DUSE_HAL_DRIVER
    -DSTM32F373xC
    -DBOARD_NAME="RD"
    -DHWVERSION_HI=1 -DHWVERSION_LO=1)

# Add define if debbuger support is enabled
target_compile_definitions(${PROJECT_NAME} PUBLIC $<$<BOOL:${USE_DEBUGGER}>:USE_DEBUGGER>)
if(LOG_LEVEL)
  target_compile_definitions(${PROJECT_NAME} PUBLIC -DLOG_LEVEL=${LOG_LEVEL})
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/stm32/stm32f3xx/STM32F3xx_HAL_Driver/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/cmsis
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/stm32
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/stm32/stm32f3xx
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/stm32/cmsis
    ${CMAKE_CURRENT_SOURCE_DIR}/../../peripherals/rtc
#    ${CMAKE_CURRENT_SOURCE_DIR}/../../peripherals/memory
#    $<TARGET_PROPERTY:board,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:system,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:utils,INTERFACE_INCLUDE_DIRECTORIES>
)
