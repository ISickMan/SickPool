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
CMAKE_BINARY_DIR = /home/sickguy/Documents/Projects/SickPool/server

# Include any dependencies generated for this target.
include src/blocks/CMakeFiles/block_watcher.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/blocks/CMakeFiles/block_watcher.dir/compiler_depend.make

# Include the progress variables for this target.
include src/blocks/CMakeFiles/block_watcher.dir/progress.make

# Include the compile flags for this target's objects.
include src/blocks/CMakeFiles/block_watcher.dir/flags.make

src/blocks/CMakeFiles/block_watcher.dir/main.cpp.o: src/blocks/CMakeFiles/block_watcher.dir/flags.make
src/blocks/CMakeFiles/block_watcher.dir/main.cpp.o: src/blocks/main.cpp
src/blocks/CMakeFiles/block_watcher.dir/main.cpp.o: src/blocks/CMakeFiles/block_watcher.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --progress-dir=/home/sickguy/Documents/Projects/SickPool/server/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/blocks/CMakeFiles/block_watcher.dir/main.cpp.o"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/blocks && /usr/bin/g++-10 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/blocks/CMakeFiles/block_watcher.dir/main.cpp.o -MF CMakeFiles/block_watcher.dir/main.cpp.o.d -o CMakeFiles/block_watcher.dir/main.cpp.o -c /home/sickguy/Documents/Projects/SickPool/server/src/blocks/main.cpp

src/blocks/CMakeFiles/block_watcher.dir/main.cpp.i: cmake_force
	@echo "Preprocessing CXX source to CMakeFiles/block_watcher.dir/main.cpp.i"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/blocks && /usr/bin/g++-10 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sickguy/Documents/Projects/SickPool/server/src/blocks/main.cpp > CMakeFiles/block_watcher.dir/main.cpp.i

src/blocks/CMakeFiles/block_watcher.dir/main.cpp.s: cmake_force
	@echo "Compiling CXX source to assembly CMakeFiles/block_watcher.dir/main.cpp.s"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/blocks && /usr/bin/g++-10 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sickguy/Documents/Projects/SickPool/server/src/blocks/main.cpp -o CMakeFiles/block_watcher.dir/main.cpp.s

# Object files for target block_watcher
block_watcher_OBJECTS = \
"CMakeFiles/block_watcher.dir/main.cpp.o"

# External object files for target block_watcher
block_watcher_EXTERNAL_OBJECTS =

src/blocks/block_watcher: src/blocks/CMakeFiles/block_watcher.dir/main.cpp.o
src/blocks/block_watcher: src/blocks/CMakeFiles/block_watcher.dir/build.make
src/blocks/block_watcher: _deps/simdjson-build/libsimdjson.a
src/blocks/block_watcher: _deps/fmt-build/libfmt.a
src/blocks/block_watcher: src/libSickPool_core.a
src/blocks/block_watcher: _deps/simdjson-build/libsimdjson.a
src/blocks/block_watcher: _deps/fmt-build/libfmt.a
src/blocks/block_watcher: /usr/local/lib/libboost_program_options.a
src/blocks/block_watcher: _deps/hiredis-build/libhiredis.a
src/blocks/block_watcher: src/crypto/cn/libcryptonote_utils.a
src/blocks/block_watcher: src/crypto/verushash/libsickverushash.a
src/blocks/block_watcher: src/blocks/CMakeFiles/block_watcher.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --progress-dir=/home/sickguy/Documents/Projects/SickPool/server/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable block_watcher"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/blocks && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/block_watcher.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/blocks/CMakeFiles/block_watcher.dir/build: src/blocks/block_watcher
.PHONY : src/blocks/CMakeFiles/block_watcher.dir/build

src/blocks/CMakeFiles/block_watcher.dir/clean:
	cd /home/sickguy/Documents/Projects/SickPool/server/src/blocks && $(CMAKE_COMMAND) -P CMakeFiles/block_watcher.dir/cmake_clean.cmake
.PHONY : src/blocks/CMakeFiles/block_watcher.dir/clean

src/blocks/CMakeFiles/block_watcher.dir/depend:
	cd /home/sickguy/Documents/Projects/SickPool/server && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/sickguy/Documents/Projects/SickPool/server /home/sickguy/Documents/Projects/SickPool/server/src/blocks /home/sickguy/Documents/Projects/SickPool/server /home/sickguy/Documents/Projects/SickPool/server/src/blocks /home/sickguy/Documents/Projects/SickPool/server/src/blocks/CMakeFiles/block_watcher.dir/DependInfo.cmake
.PHONY : src/blocks/CMakeFiles/block_watcher.dir/depend

