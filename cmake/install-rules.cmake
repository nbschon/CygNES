include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
set(package CygNES)

install(
    TARGETS CygNES_exe
    RUNTIME COMPONENT CygNES_Runtime
)

write_basic_package_version_file(
    "${package}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)

# Allow package maintainers to freely override the path for the configs
set(
    CygNES_INSTALL_CMAKEDIR "${CMAKE_INSTALL_DATADIR}/${package}"
    CACHE PATH "CMake package config location relative to the install prefix"
)
mark_as_advanced(CygNES_INSTALL_CMAKEDIR)

install(
    FILES "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
    DESTINATION "${CygNES_INSTALL_CMAKEDIR}"
    COMPONENT CygNES_Development
)

# Export variables for the install script to use
install(CODE "
set(CygNES_NAME [[$<TARGET_FILE_NAME:CygNES_exe>]])
set(CygNES_INSTALL_CMAKEDIR [[${CygNES_INSTALL_CMAKEDIR}]])
set(CMAKE_INSTALL_BINDIR [[${CMAKE_INSTALL_BINDIR}]])
" COMPONENT CygNES_Development)

install(
    SCRIPT cmake/install-script.cmake
    COMPONENT CygNES_Development
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
