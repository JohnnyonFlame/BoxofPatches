cmake_minimum_required(VERSION 3.13)

file(GLOB FreedomPlanet_src CONFIGURE_DEPENDS "*.h" "*.cpp")

add_library(FreedomPlanet SHARED $<TARGET_OBJECTS:Common> ${FreedomPlanet_src})
target_include_directories(FreedomPlanet PUBLIC ${PROJECT_SOURCE_DIR}/common ${xbyak_SOURCE_DIR}/xbyak)
