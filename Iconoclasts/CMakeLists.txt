cmake_minimum_required(VERSION 3.13)

file(GLOB Iconoclasts_src CONFIGURE_DEPENDS "*.h" "*.cpp")

add_library(Iconoclasts SHARED $<TARGET_OBJECTS:Common> ${Iconoclasts_src})
target_include_directories(Iconoclasts PUBLIC ${PROJECT_SOURCE_DIR}/common ${xbyak_SOURCE_DIR}/xbyak)