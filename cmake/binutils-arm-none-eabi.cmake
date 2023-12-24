#---------------------------------------------------------------------------------------
# Set tools
#---------------------------------------------------------------------------------------
set(CMAKE_OBJCOPY ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN}-objcopy${TOOLCHAIN_EXT})
set(CMAKE_OBJDUMP ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN}-objdump${TOOLCHAIN_EXT})
set(CMAKE_SIZE ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN}-size${TOOLCHAIN_EXT})

#---------------------------------------------------------------------------------------
# Prints the section sizes
#---------------------------------------------------------------------------------------
function(print_section_sizes TARGET)
    add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_SIZE} ${TARGET})
endfunction()

# Функция генерирует hex и bin файлы, плюс показывает размеры прошивки
function(fn_add_generation_hex_bin TARGET FILE_NAME FILE_PATH)
    add_custom_command(TARGET ${TARGET} POST_BUILD
        COMMAND echo Size of hex
        COMMAND ${CMAKE_SIZE} "$<TARGET_FILE:${TARGET}>"
        COMMAND echo Create HEX BIN files to ${FILE_PATH}/${FILE_NAME}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${FILE_PATH}
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${TARGET}> ${FILE_PATH}/${FILE_NAME}.hex
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${TARGET}> ${FILE_PATH}/${FILE_NAME}.bin
        )
endfunction()