cmake_minimum_required(VERSION 3.13)

include(FetchContent)
FetchContent_Declare(
  xbyak
  GIT_REPOSITORY https://github.com/herumi/xbyak.git
  GIT_TAG        47cf85fdba463135a18c4924f5d82b14cb4e0068
)

FetchContent_MakeAvailable(xbyak)