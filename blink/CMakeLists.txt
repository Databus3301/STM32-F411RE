set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

cmake_minimum_required(VERSION 3.15)
project(stm32_blink C)

set(CMAKE_C_STANDARD 11)

set(OBJECT_GEN_FLAGS "-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OBJECT_GEN_FLAGS} -O0 -g3 -Wall")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OBJECT_GEN_FLAGS} -nostartfiles -nodefaultlibs -nostdlib -T ${CMAKE_SOURCE_DIR}/linker.ld")

add_executable(blink external.c)

target_compile_definitions(blink PRIVATE STM32F411xE)
target_include_directories(blink PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
