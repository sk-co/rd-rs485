project(system)
cmake_minimum_required(VERSION 3.16)

#---------------------------------------------------------------------------------------
# Target
#---------------------------------------------------------------------------------------

file(GLOB ${PROJECT_NAME}_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(${PROJECT_NAME} OBJECT EXCLUDE_FROM_ALL ${${PROJECT_NAME}_SOURCES})

target_include_directories( ${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
#    $<TARGET_PROPERTY:secure_element,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:board,INTERFACE_INCLUDE_DIRECTORIES>
    "${CMAKE_CURRENT_SOURCE_DIR}/../common"
    "${CMAKE_CURRENT_SOURCE_DIR}/../common/crypto"
    "${CMAKE_CURRENT_SOURCE_DIR}/../utils"
)
