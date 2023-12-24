
project(rd-rs485)
cmake_minimum_required(VERSION 3.16)
set(CMAKE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(CUSTOM_CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)
list(APPEND CMAKE_MODULE_PATH ${CUSTOM_CMAKE_MODULE_PATH})
#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

#---------------------------------------------------------------------------------------
# Options
#---------------------------------------------------------------------------------------

# Allow switching of target platform
set(BOARD_LIST
    rd10
    )
set(BOARD rd10 CACHE STRING "Default target platform is rak811")
set_property(CACHE BOARD PROPERTY STRINGS ${BOARD_LIST})

# Allow switching of Applications
set(APPLICATION_LIST
    tensometer
    )
set(APPLICATION tensometer CACHE STRING "Default Application is devices")
set_property(CACHE APPLICATION PROPERTY STRINGS ${APPLICATION_LIST})

# Switch for debugger support.
option(USE_DEBUGGER "Use Debugger" ON)

#---------------------------------------------------------------------------------------
# Target Boards
#---------------------------------------------------------------------------------------

if(BOARD STREQUAL rd10)
    set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/boards/rd10/ld/stm32f373cctx_flash.ld)
    include(stm32f3)
    add_subdirectory(${CMAKE_SOURCE_DIR}/boards/rd10)
endif()

#---------------------------------------------------------------------------------------
# General Components
#---------------------------------------------------------------------------------------

add_subdirectory(${CMAKE_SOURCE_DIR}/boards)
add_subdirectory(${CMAKE_SOURCE_DIR}/system)
add_subdirectory(${CMAKE_SOURCE_DIR}/utils)

#---------------------------------------------------------------------------------------
# Applications
#---------------------------------------------------------------------------------------

if(APPLICATION STREQUAL tensometer)
    add_subdirectory(${CMAKE_SOURCE_DIR}/apps/tensometer)
else()
    message(FATAL_ERROR "Unknown PROJECT")
endif()

message("board: ${BOARD}")
message("app: ${APPLICATION}")
message("log_level: ${LOG_LEVEL}")