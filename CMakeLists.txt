cmake_minimum_required(VERSION 3.20)

project(
  niubcc
  LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# add_executable(niubcc
#   src/niub.cxx
# )

add_library(niubcc STATIC
  src/lexer.cc
  # src/niub.cxx
  src/token.cc
  src/buffer.cc
  src/utils.cc
  src/ast.cc
  src/parser.cc
  src/codegen.cc
  src/tacky.cc
  src/symbol_table.cc
)

target_include_directories(niubcc PUBLIC include)

target_compile_features(niubcc PUBLIC cxx_std_17)

enable_testing()
add_subdirectory(tests)