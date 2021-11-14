cmake_minimum_required(VERSION 3.13)

file(GLOB ShovelKnight_src CONFIGURE_DEPENDS "*.h" "*.cpp")

add_library(ShovelKnight SHARED $<TARGET_OBJECTS:Common> ${ShovelKnight_src})
target_include_directories(ShovelKnight PUBLIC ${PROJECT_SOURCE_DIR}/common ${xbyak_SOURCE_DIR}/xbyak)