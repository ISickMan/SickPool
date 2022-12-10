# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.23

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
CMAKE_COMMAND = /opt/cmake-3.23.2-linux-x86_64/bin/cmake

# The command to remove a file.
RM = /opt/cmake-3.23.2-linux-x86_64/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/sickguy/Documents/Projects/SickPool/server

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/sickguy/Documents/Projects/SickPool/server/bulid

# Include any dependencies generated for this target.
include _deps/simdjson-build/CMakeFiles/simdjson.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include _deps/simdjson-build/CMakeFiles/simdjson.dir/compiler_depend.make

# Include the progress variables for this target.
include _deps/simdjson-build/CMakeFiles/simdjson.dir/progress.make

# Include the compile flags for this target's objects.
include _deps/simdjson-build/CMakeFiles/simdjson.dir/flags.make

_deps/simdjson-build/CMakeFiles/simdjson.dir/src/simdjson.cpp.o: _deps/simdjson-build/CMakeFiles/simdjson.dir/flags.make
_deps/simdjson-build/CMakeFiles/simdjson.dir/src/simdjson.cpp.o: _deps/simdjson-src/src/simdjson.cpp
_deps/simdjson-build/CMakeFiles/simdjson.dir/src/simdjson.cpp.o: _deps/simdjson-build/CMakeFiles/simdjson.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/sickguy/Documents/Projects/SickPool/server/bulid/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object _deps/simdjson-build/CMakeFiles/simdjson.dir/src/simdjson.cpp.o"
	cd /home/sickguy/Documents/Projects/SickPool/server/bulid/_deps/simdjson-build && /usr/bin/g++-12 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT _deps/simdjson-build/CMakeFiles/simdjson.dir/src/simdjson.cpp.o -MF CMakeFiles/simdjson.dir/src/simdjson.cpp.o.d -o CMakeFiles/simdjson.dir/src/simdjson.cpp.o -c /home/sickguy/Documents/Projects/SickPool/server/bulid/_deps/simdjson-src/src/simdjson.cpp

_deps/simdjson-build/CMakeFiles/simdjson.dir/src/simdjson.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/simdjson.dir/src/simdjson.cpp.i"
	cd /home/sickguy/Documents/Projects/SickPool/server/bulid/_deps/simdjson-build && /usr/bin/g++-12 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sickguy/Documents/Projects/SickPool/server/bulid/_deps/simdjson-src/src/simdjson.cpp > CMakeFiles/simdjson.dir/src/simdjson.cpp.i

_deps/simdjson-build/CMakeFiles/simdjson.dir/src/simdjson.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/simdjson.dir/src/simdjson.cpp.s"
	cd /home/sickguy/Documents/Projects/SickPool/server/bulid/_deps/simdjson-build && /usr/bin/g++-12 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sickguy/Documents/Projects/SickPool/server/bulid/_deps/simdjson-src/src/simdjson.cpp -o CMakeFiles/simdjson.dir/src/simdjson.cpp.s

# Object files for target simdjson
simdjson_OBJECTS = \
"CMakeFiles/simdjson.dir/src/simdjson.cpp.o"

# External object files for target simdjson
simdjson_EXTERNAL_OBJECTS =

_deps/simdjson-build/libsimdjson.a: _deps/simdjson-build/CMakeFiles/simdjson.dir/src/simdjson.cpp.o
_deps/simdjson-build/libsimdjson.a: _deps/simdjson-build/CMakeFiles/simdjson.dir/build.make
_deps/simdjson-build/libsimdjson.a: _deps/simdjson-build/CMakeFiles/simdjson.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/sickguy/Documents/Projects/SickPool/server/bulid/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library libsimdjson.a"
	cd /home/sickguy/Documents/Projects/SickPool/server/bulid/_deps/simdjson-build && $(CMAKE_COMMAND) -P CMakeFiles/simdjson.dir/cmake_clean_target.cmake
	cd /home/sickguy/Documents/Projects/SickPool/server/bulid/_deps/simdjson-build && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/simdjson.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
_deps/simdjson-build/CMakeFiles/simdjson.dir/build: _deps/simdjson-build/libsimdjson.a
.PHONY : _deps/simdjson-build/CMakeFiles/simdjson.dir/build

_deps/simdjson-build/CMakeFiles/simdjson.dir/clean:
	cd /home/sickguy/Documents/Projects/SickPool/server/bulid/_deps/simdjson-build && $(CMAKE_COMMAND) -P CMakeFiles/simdjson.dir/cmake_clean.cmake
.PHONY : _deps/simdjson-build/CMakeFiles/simdjson.dir/clean

_deps/simdjson-build/CMakeFiles/simdjson.dir/depend:
	cd /home/sickguy/Documents/Projects/SickPool/server/bulid && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/sickguy/Documents/Projects/SickPool/server /home/sickguy/Documents/Projects/SickPool/server/bulid/_deps/simdjson-src /home/sickguy/Documents/Projects/SickPool/server/bulid /home/sickguy/Documents/Projects/SickPool/server/bulid/_deps/simdjson-build /home/sickguy/Documents/Projects/SickPool/server/bulid/_deps/simdjson-build/CMakeFiles/simdjson.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : _deps/simdjson-build/CMakeFiles/simdjson.dir/depend

