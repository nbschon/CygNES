file(
    RELATIVE_PATH relative_path
    "/${CygNES_INSTALL_CMAKEDIR}"
    "/${CMAKE_INSTALL_BINDIR}/${CygNES_NAME}"
)

get_filename_component(prefix "${CMAKE_INSTALL_PREFIX}" ABSOLUTE)
set(config_dir "${prefix}/${CygNES_INSTALL_CMAKEDIR}")
set(config_file "${config_dir}/CygNESConfig.cmake")

message(STATUS "Installing: ${config_file}")
file(WRITE "${config_file}" "\
get_filename_component(
    _CygNES_executable
    \"\${CMAKE_CURRENT_LIST_DIR}/${relative_path}\"
    ABSOLUTE
)
set(
    CYGNES_EXECUTABLE \"\${_CygNES_executable}\"
    CACHE FILEPATH \"Path to the CygNES executable\"
)
")
