project(tensometer)
cmake_minimum_required(VERSION 3.16)

# Set firmware version
macro(fn_set_version PROJECT V_HI V_LO)
  set(${PROJECT}_V_HI ${V_HI})
  set(${PROJECT}_V_LO ${V_LO})
endmacro()
fn_set_version(${PROJECT_NAME} 1 0)
MATH(EXPR FIRMWARE_VERSION "(${${PROJECT_NAME}_V_HI} << 8) + ${${PROJECT_NAME}_V_LO}")

set(SRC
    "${CMAKE_CURRENT_LIST_DIR}/../../peripherals/rs485.cpp"
    )
#set(INC_DIRS
#    "${CMAKE_CURRENT_LIST_DIR}/../../protocol/di15"
#    )

#-------------------------------------------------------------------------------

file(GLOB ${PROJECT_NAME}_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/${SUB_PROJECT_PATH}/${BOARD}/*.c"
    "${CMAKE_CURRENT_LIST_DIR}/${SUB_PROJECT_PATH}/${BOARD}/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/${SUB_PROJECT_PATH}/*.c"
    "${CMAKE_CURRENT_LIST_DIR}/${SUB_PROJECT_PATH}/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/../../common/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/../../common/crypto/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/../../peripherals/gtime.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/../../peripherals/memory/ram_23k256.cpp"
    )

add_executable(${PROJECT_NAME}
    ${${PROJECT_NAME}_SOURCES}
    ${${PROJECT_NAME}_COMMON}
    ${SRC}
    $<TARGET_OBJECTS:${BOARD}>
    $<TARGET_OBJECTS:system>
    $<TARGET_OBJECTS:utils>
    )

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/${SUB_PROJECT_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/${SUB_PROJECT_PATH}/${BOARD}
    ${INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/common/app_core
    ${CMAKE_CURRENT_LIST_DIR}/../../common
    ${CMAKE_CURRENT_LIST_DIR}/../../common/crypto
    ${CMAKE_CURRENT_LIST_DIR}/../../peripherals
    ${CMAKE_CURRENT_LIST_DIR}/../../peripherals/memory
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:system,INTERFACE_INCLUDE_DIRECTORIES>>
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:${BOARD},INTERFACE_INCLUDE_DIRECTORIES>>
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:utils,INTERFACE_INCLUDE_DIRECTORIES>>
    )

target_compile_definitions(${PROJECT_NAME} PRIVATE $<TARGET_PROPERTY:${BOARD},INTERFACE_COMPILE_DEFINITIONS>)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    -DFIRMWARE_VERSION_HI=${${PROJECT_NAME}_V_HI} -DFIRMWARE_VERSION_LO=${${PROJECT_NAME}_V_LO})
if (LOG_LEVEL)
  target_compile_definitions(${PROJECT_NAME} PUBLIC -DLOG_LEVEL=${LOG_LEVEL})
endif ()
target_link_libraries(${PROJECT_NAME} m)

#-------------------------------------------------------------------------------
# Debugging and Binutils
include(binutils-arm-none-eabi)
set(OUT_FILE_VERSION ${${PROJECT_NAME}_V_HI}.${${PROJECT_NAME}_V_LO})
set(OUT_FILE_PATH "${CMAKE_BINARY_DIR}/BUILD/${PROJECT_NAME}/v${OUT_FILE_VERSION}/${BOARD}")
set(OUT_FILE_NAME "${PROJECT_NAME}-${BOARD}-v${OUT_FILE_VERSION}")
string(REPLACE ksd ksi OUT_FILE_NAME ${OUT_FILE_NAME} )
if(${BOOTLOADER})
  set(OUT_FILE_NAME ${OUT_FILE_NAME}_b)
endif()
fn_add_generation_hex_bin(${PROJECT_NAME} ${OUT_FILE_NAME} ${OUT_FILE_PATH})