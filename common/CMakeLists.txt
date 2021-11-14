cmake_minimum_required(VERSION 3.13)

file(GLOB Common_src CONFIGURE_DEPENDS "*.h" "*.cpp")
add_library(Common OBJECT ${Common_src})
set_property(TARGET Common PROPERTY POSITION_INDEPENDENT_CODE ON)