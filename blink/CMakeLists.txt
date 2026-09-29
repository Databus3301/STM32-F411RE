cmake_minimum_required(VERSION 3.15)

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

project(stm32_blink C)

set(CMAKE_C_STANDARD 11)

set(OBJECT_GEN_FLAGS "-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OBJECT_GEN_FLAGS} -O0 -g3 -Wall")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OBJECT_GEN_FLAGS} -nostartfiles -nodefaultlibs -nostdlib -T ${CMAKE_SOURCE_DIR}/linker.ld")

add_executable(blink external.c)
