cmake_minimum_required(VERSION 3.20)

project(
  niubcc
  LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(niubcc
  src/niub.cxx
)

target_sources(niubcc PRIVATE
  src/token.cxx
  src/lexer.cxx
  src/token.cxx
)

target_include_directories(niubcc PRIVATE include)

target_compile_features(niubcc PRIVATE cxx_std_17)