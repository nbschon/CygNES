# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.22.3/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.22.3/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/noahschonhorn/Documents/CygNES

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/noahschonhorn/Documents/CygNES/cmake

# Include any dependencies generated for this target.
include CMakeFiles/CygNES_exe.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/CygNES_exe.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/CygNES_exe.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/CygNES_exe.dir/flags.make

CMakeFiles/CygNES_exe.dir/source/main.cpp.o: CMakeFiles/CygNES_exe.dir/flags.make
CMakeFiles/CygNES_exe.dir/source/main.cpp.o: ../source/main.cpp
CMakeFiles/CygNES_exe.dir/source/main.cpp.o: CMakeFiles/CygNES_exe.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/noahschonhorn/Documents/CygNES/cmake/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/CygNES_exe.dir/source/main.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/CygNES_exe.dir/source/main.cpp.o -MF CMakeFiles/CygNES_exe.dir/source/main.cpp.o.d -o CMakeFiles/CygNES_exe.dir/source/main.cpp.o -c /Users/noahschonhorn/Documents/CygNES/source/main.cpp

CMakeFiles/CygNES_exe.dir/source/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/CygNES_exe.dir/source/main.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/noahschonhorn/Documents/CygNES/source/main.cpp > CMakeFiles/CygNES_exe.dir/source/main.cpp.i

CMakeFiles/CygNES_exe.dir/source/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/CygNES_exe.dir/source/main.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/noahschonhorn/Documents/CygNES/source/main.cpp -o CMakeFiles/CygNES_exe.dir/source/main.cpp.s

# Object files for target CygNES_exe
CygNES_exe_OBJECTS = \
"CMakeFiles/CygNES_exe.dir/source/main.cpp.o"

# External object files for target CygNES_exe
CygNES_exe_EXTERNAL_OBJECTS = \
"/Users/noahschonhorn/Documents/CygNES/cmake/CMakeFiles/CygNES_lib.dir/source/lib.cpp.o"

CygNES: CMakeFiles/CygNES_exe.dir/source/main.cpp.o
CygNES: CMakeFiles/CygNES_lib.dir/source/lib.cpp.o
CygNES: CMakeFiles/CygNES_exe.dir/build.make
CygNES: CMakeFiles/CygNES_exe.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/noahschonhorn/Documents/CygNES/cmake/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable CygNES"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/CygNES_exe.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/CygNES_exe.dir/build: CygNES
.PHONY : CMakeFiles/CygNES_exe.dir/build

CMakeFiles/CygNES_exe.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/CygNES_exe.dir/cmake_clean.cmake
.PHONY : CMakeFiles/CygNES_exe.dir/clean

CMakeFiles/CygNES_exe.dir/depend:
	cd /Users/noahschonhorn/Documents/CygNES/cmake && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/noahschonhorn/Documents/CygNES /Users/noahschonhorn/Documents/CygNES /Users/noahschonhorn/Documents/CygNES/cmake /Users/noahschonhorn/Documents/CygNES/cmake /Users/noahschonhorn/Documents/CygNES/cmake/CMakeFiles/CygNES_exe.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/CygNES_exe.dir/depend

