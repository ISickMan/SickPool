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
CMAKE_SOURCE_DIR = /home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-subbuild

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-subbuild

# Utility rule file for hiredis-populate.

# Include any custom commands dependencies for this target.
include CMakeFiles/hiredis-populate.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/hiredis-populate.dir/progress.make

CMakeFiles/hiredis-populate: CMakeFiles/hiredis-populate-complete

CMakeFiles/hiredis-populate-complete: hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-install
CMakeFiles/hiredis-populate-complete: hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-mkdir
CMakeFiles/hiredis-populate-complete: hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-download
CMakeFiles/hiredis-populate-complete: hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-update
CMakeFiles/hiredis-populate-complete: hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-patch
CMakeFiles/hiredis-populate-complete: hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-configure
CMakeFiles/hiredis-populate-complete: hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-build
CMakeFiles/hiredis-populate-complete: hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-install
CMakeFiles/hiredis-populate-complete: hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-test
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-subbuild/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Completed 'hiredis-populate'"
	/opt/cmake-3.23.2-linux-x86_64/bin/cmake -E make_directory /home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-subbuild/CMakeFiles
	/opt/cmake-3.23.2-linux-x86_64/bin/cmake -E touch /home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-subbuild/CMakeFiles/hiredis-populate-complete
	/opt/cmake-3.23.2-linux-x86_64/bin/cmake -E touch /home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-subbuild/hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-done

hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-update:
.PHONY : hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-update

hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-build: hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-configure
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-subbuild/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "No build step for 'hiredis-populate'"
	cd /home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-build && /opt/cmake-3.23.2-linux-x86_64/bin/cmake -E echo_append
	cd /home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-build && /opt/cmake-3.23.2-linux-x86_64/bin/cmake -E touch /home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-subbuild/hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-build

hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-configure: hiredis-populate-prefix/tmp/hiredis-populate-cfgcmd.txt
hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-configure: hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-patch
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-subbuild/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "No configure step for 'hiredis-populate'"
	cd /home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-build && /opt/cmake-3.23.2-linux-x86_64/bin/cmake -E echo_append
	cd /home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-build && /opt/cmake-3.23.2-linux-x86_64/bin/cmake -E touch /home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-subbuild/hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-configure

hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-download: hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-gitinfo.txt
hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-download: hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-mkdir
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-subbuild/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Performing download step (git clone) for 'hiredis-populate'"
	cd /home/sickguy/Documents/Projects/SickPool/server/_deps && /opt/cmake-3.23.2-linux-x86_64/bin/cmake -P /home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-subbuild/hiredis-populate-prefix/tmp/hiredis-populate-gitclone.cmake
	cd /home/sickguy/Documents/Projects/SickPool/server/_deps && /opt/cmake-3.23.2-linux-x86_64/bin/cmake -E touch /home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-subbuild/hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-download

hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-install: hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-build
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-subbuild/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "No install step for 'hiredis-populate'"
	cd /home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-build && /opt/cmake-3.23.2-linux-x86_64/bin/cmake -E echo_append
	cd /home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-build && /opt/cmake-3.23.2-linux-x86_64/bin/cmake -E touch /home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-subbuild/hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-install

hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-mkdir:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-subbuild/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Creating directories for 'hiredis-populate'"
	/opt/cmake-3.23.2-linux-x86_64/bin/cmake -P /home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-subbuild/hiredis-populate-prefix/tmp/hiredis-populate-mkdirs.cmake
	/opt/cmake-3.23.2-linux-x86_64/bin/cmake -E touch /home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-subbuild/hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-mkdir

hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-patch: hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-update
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-subbuild/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "No patch step for 'hiredis-populate'"
	/opt/cmake-3.23.2-linux-x86_64/bin/cmake -E echo_append
	/opt/cmake-3.23.2-linux-x86_64/bin/cmake -E touch /home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-subbuild/hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-patch

hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-update:
.PHONY : hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-update

hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-test: hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-install
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-subbuild/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "No test step for 'hiredis-populate'"
	cd /home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-build && /opt/cmake-3.23.2-linux-x86_64/bin/cmake -E echo_append
	cd /home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-build && /opt/cmake-3.23.2-linux-x86_64/bin/cmake -E touch /home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-subbuild/hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-test

hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-update: hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-download
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-subbuild/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Performing update step for 'hiredis-populate'"
	cd /home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-src && /opt/cmake-3.23.2-linux-x86_64/bin/cmake -P /home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-subbuild/hiredis-populate-prefix/tmp/hiredis-populate-gitupdate.cmake

hiredis-populate: CMakeFiles/hiredis-populate
hiredis-populate: CMakeFiles/hiredis-populate-complete
hiredis-populate: hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-build
hiredis-populate: hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-configure
hiredis-populate: hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-download
hiredis-populate: hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-install
hiredis-populate: hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-mkdir
hiredis-populate: hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-patch
hiredis-populate: hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-test
hiredis-populate: hiredis-populate-prefix/src/hiredis-populate-stamp/hiredis-populate-update
hiredis-populate: CMakeFiles/hiredis-populate.dir/build.make
.PHONY : hiredis-populate

# Rule to build all files generated by this target.
CMakeFiles/hiredis-populate.dir/build: hiredis-populate
.PHONY : CMakeFiles/hiredis-populate.dir/build

CMakeFiles/hiredis-populate.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/hiredis-populate.dir/cmake_clean.cmake
.PHONY : CMakeFiles/hiredis-populate.dir/clean

CMakeFiles/hiredis-populate.dir/depend:
	cd /home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-subbuild && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-subbuild /home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-subbuild /home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-subbuild /home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-subbuild /home/sickguy/Documents/Projects/SickPool/server/_deps/hiredis-subbuild/CMakeFiles/hiredis-populate.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/hiredis-populate.dir/depend

