cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    CygNES
    VERSION 0.1.0
    DESCRIPTION "NES Emulator in C++"
    LANGUAGES CXX
)

find_package(SDL2 REQUIRED)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(
    CygNES_lib OBJECT
    source/lib.cpp
    source/utils.cpp
    source/utils.hpp
    source/cpu.cpp
    source/cpu.hpp
    source/cartridge.cpp
    source/cartridge.hpp
    source/mapper.cpp
    source/mapper.hpp
    source/ppu.cpp
    source/ppu.hpp
    source/mapper000.cpp
    source/mapper000.hpp source/controller.cpp source/controller.hpp)

target_include_directories(
    CygNES_lib ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
    ${SDL2_INCLUDE_DIRS}
)

#target_compile_options(CygNES_lib PRIVATE "-O3")

target_compile_features(CygNES_lib PUBLIC cxx_std_17)

# ---- Declare executable ----

add_executable(CygNES_exe source/main.cpp)
add_executable(CygNES::exe ALIAS CygNES_exe)

set_target_properties(
    CygNES_exe PROPERTIES
    OUTPUT_NAME CygNES
    EXPORT_NAME exe
)

target_compile_features(CygNES_exe PRIVATE cxx_std_17)

#target_compile_options(CygNES_exe PRIVATE "-O3")

target_link_libraries(CygNES_exe PRIVATE CygNES_lib ${SDL2_LIBRARIES})

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT CygNES_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of CygNES"
  )
endif()

include(cmake/dev-mode.cmake)
