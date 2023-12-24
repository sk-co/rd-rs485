##
##   ______                              _
##  / _____)             _              | |
## ( (____  _____ ____ _| |_ _____  ____| |__
##  \____ \| ___ |    (_   _) ___ |/ ___)  _ \
##  _____) ) ____| | | || |_| ____( (___| | | |
## (______/|_____)_|_|_| \__)_____)\____)_| |_|
## (C)2013-2018 Semtech
##  ___ _____ _   ___ _  _____ ___  ___  ___ ___
## / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
## \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
## |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
## embedded.connectivity.solutions.==============
##
## License:  Revised BSD License, see LICENSE.TXT file included in the project
## Authors:  Johannes Bruder ( STACKFORCE ), Miguel Luis ( Semtech )
##
##
## STM32L4 target specific CMake file
##

if(NOT DEFINED LINKER_SCRIPT)
  message(FATAL_ERROR "No linker script defined")
endif()
message("Linker script: ${LINKER_SCRIPT}")


#---------------------------------------------------------------------------------------
# Set compiler/linker flags
#---------------------------------------------------------------------------------------

# Object build options
set(OBJECT_GEN_FLAGS "-mthumb -fno-builtin -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -Wall \
  -Wextra -Wno-unused-parameter -ffunction-sections -fdata-sections -fomit-frame-pointer -mabi=aapcs \
  -fno-unroll-loops -ffast-math -ftree-vectorize -Wno-deprecated ")

set(CMAKE_C_FLAGS "${OBJECT_GEN_FLAGS} -std=gnu11 " CACHE INTERNAL "C Compiler options")
set(CMAKE_CXX_FLAGS "${OBJECT_GEN_FLAGS} -std=c++11 " CACHE INTERNAL "C++ Compiler options")
set(CMAKE_ASM_FLAGS "${OBJECT_GEN_FLAGS} -c -x assembler-with-cpp " CACHE INTERNAL "ASM Compiler options")

set(COMMON_C_FLAGS "${OBJECT_GEN_FLAGS} -nostdlib --specs=nano.specs" )
set(COMMON_CXX_FLAGS "${OBJECT_GEN_FLAGS} -Wno-register -fno-rtti -fno-exceptions \
  -fno-unwind-tables -fno-asynchronous-unwind-tables \
  -fno-use-cxa-atexit -fno-threadsafe-statics")
set(COMMON_ASM_FLAGS "${OBJECT_GEN_FLAGS} -x assembler-with-cpp")
set(COMMON_LINK_FLAGS "${OBJECT_GEN_FLAGS} -lc -Wl,--gc-sections -static --specs=nano.specs \
  --specs=nosys.specs")

if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
  set(COMMON_C_FLAGS "${COMMON_C_FLAGS} -Og -g3 -DDEBUG -gdwarf-2")
  set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS} -Og -g3 -DDEBUG")
  set(COMMON_ASM_FLAGS "${COMMON_ASM_FLAGS} -g")
  set(COMMON_LINK_FLAGS "${COMMON_LINK_FLAGS} -g2 -u _printf_float") #-u _printf_float
endif()
if(${CMAKE_BUILD_TYPE} STREQUAL Release)
  set(COMMON_C_FLAGS "${COMMON_C_FLAGS} -Os") # -flto
  set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS} -Os") # -flto
  set(COMMON_ASM_FLAGS "${COMMON_ASM_FLAGS}" )
  set(COMMON_LINK_FLAGS "${COMMON_LINK_FLAGS}") # -flto
endif()

# Linker flags
#set(CMAKE_EXE_LINKER_FLAGS " -mthumb \
#  ${COMMON_LINK_FLAGS} \
#  -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mabi=aapcs -T${LINKER_SCRIPT} \
#  -Wl,-Map=${CMAKE_PROJECT_NAME}.map" CACHE INTERNAL "Linker options")

# Linker flags
set(CMAKE_EXE_LINKER_FLAGS " ${COMMON_LINK_FLAGS}  -T${LINKER_SCRIPT} \
  -Wl,-Map=${CMAKE_PROJECT_NAME}.map" CACHE INTERNAL "Linker options")

set(CMAKE_C_FLAGS "${COMMON_C_FLAGS}" CACHE INTERNAL "C Compiler options")
set(CMAKE_CXX_FLAGS "${COMMON_CXX_FLAGS}" CACHE INTERNAL "C++ Compiler options")
set(CMAKE_ASM_FLAGS "${COMMON_ASM_FLAGS}" CACHE INTERNAL "ASM Compiler options")