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
include src/crypto/verushash/CMakeFiles/sickverushash.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/crypto/verushash/CMakeFiles/sickverushash.dir/compiler_depend.make

# Include the progress variables for this target.
include src/crypto/verushash/CMakeFiles/sickverushash.dir/progress.make

# Include the compile flags for this target's objects.
include src/crypto/verushash/CMakeFiles/sickverushash.dir/flags.make

src/crypto/verushash/CMakeFiles/sickverushash.dir/arith_uint256.cpp.o: src/crypto/verushash/CMakeFiles/sickverushash.dir/flags.make
src/crypto/verushash/CMakeFiles/sickverushash.dir/arith_uint256.cpp.o: src/crypto/verushash/arith_uint256.cpp
src/crypto/verushash/CMakeFiles/sickverushash.dir/arith_uint256.cpp.o: src/crypto/verushash/CMakeFiles/sickverushash.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --progress-dir=/home/sickguy/Documents/Projects/SickPool/server/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/crypto/verushash/CMakeFiles/sickverushash.dir/arith_uint256.cpp.o"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && /usr/bin/g++-10 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/crypto/verushash/CMakeFiles/sickverushash.dir/arith_uint256.cpp.o -MF CMakeFiles/sickverushash.dir/arith_uint256.cpp.o.d -o CMakeFiles/sickverushash.dir/arith_uint256.cpp.o -c /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash/arith_uint256.cpp

src/crypto/verushash/CMakeFiles/sickverushash.dir/arith_uint256.cpp.i: cmake_force
	@echo "Preprocessing CXX source to CMakeFiles/sickverushash.dir/arith_uint256.cpp.i"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && /usr/bin/g++-10 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash/arith_uint256.cpp > CMakeFiles/sickverushash.dir/arith_uint256.cpp.i

src/crypto/verushash/CMakeFiles/sickverushash.dir/arith_uint256.cpp.s: cmake_force
	@echo "Compiling CXX source to assembly CMakeFiles/sickverushash.dir/arith_uint256.cpp.s"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && /usr/bin/g++-10 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash/arith_uint256.cpp -o CMakeFiles/sickverushash.dir/arith_uint256.cpp.s

src/crypto/verushash/CMakeFiles/sickverushash.dir/haraka.c.o: src/crypto/verushash/CMakeFiles/sickverushash.dir/flags.make
src/crypto/verushash/CMakeFiles/sickverushash.dir/haraka.c.o: src/crypto/verushash/haraka.c
src/crypto/verushash/CMakeFiles/sickverushash.dir/haraka.c.o: src/crypto/verushash/CMakeFiles/sickverushash.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --progress-dir=/home/sickguy/Documents/Projects/SickPool/server/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object src/crypto/verushash/CMakeFiles/sickverushash.dir/haraka.c.o"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && /usr/bin/gcc-10 $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS)  -mpclmul -msse4 -msse4.1 -msse4.2 -mssse3 -mavx -maes -g -funroll-loops -fomit-frame-pointer -MD -MT src/crypto/verushash/CMakeFiles/sickverushash.dir/haraka.c.o -MF CMakeFiles/sickverushash.dir/haraka.c.o.d -o CMakeFiles/sickverushash.dir/haraka.c.o -c /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash/haraka.c

src/crypto/verushash/CMakeFiles/sickverushash.dir/haraka.c.i: cmake_force
	@echo "Preprocessing C source to CMakeFiles/sickverushash.dir/haraka.c.i"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && /usr/bin/gcc-10 $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS)  -mpclmul -msse4 -msse4.1 -msse4.2 -mssse3 -mavx -maes -g -funroll-loops -fomit-frame-pointer -E /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash/haraka.c > CMakeFiles/sickverushash.dir/haraka.c.i

src/crypto/verushash/CMakeFiles/sickverushash.dir/haraka.c.s: cmake_force
	@echo "Compiling C source to assembly CMakeFiles/sickverushash.dir/haraka.c.s"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && /usr/bin/gcc-10 $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS)  -mpclmul -msse4 -msse4.1 -msse4.2 -mssse3 -mavx -maes -g -funroll-loops -fomit-frame-pointer -S /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash/haraka.c -o CMakeFiles/sickverushash.dir/haraka.c.s

src/crypto/verushash/CMakeFiles/sickverushash.dir/haraka_portable.c.o: src/crypto/verushash/CMakeFiles/sickverushash.dir/flags.make
src/crypto/verushash/CMakeFiles/sickverushash.dir/haraka_portable.c.o: src/crypto/verushash/haraka_portable.c
src/crypto/verushash/CMakeFiles/sickverushash.dir/haraka_portable.c.o: src/crypto/verushash/CMakeFiles/sickverushash.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --progress-dir=/home/sickguy/Documents/Projects/SickPool/server/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object src/crypto/verushash/CMakeFiles/sickverushash.dir/haraka_portable.c.o"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && /usr/bin/gcc-10 $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT src/crypto/verushash/CMakeFiles/sickverushash.dir/haraka_portable.c.o -MF CMakeFiles/sickverushash.dir/haraka_portable.c.o.d -o CMakeFiles/sickverushash.dir/haraka_portable.c.o -c /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash/haraka_portable.c

src/crypto/verushash/CMakeFiles/sickverushash.dir/haraka_portable.c.i: cmake_force
	@echo "Preprocessing C source to CMakeFiles/sickverushash.dir/haraka_portable.c.i"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && /usr/bin/gcc-10 $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash/haraka_portable.c > CMakeFiles/sickverushash.dir/haraka_portable.c.i

src/crypto/verushash/CMakeFiles/sickverushash.dir/haraka_portable.c.s: cmake_force
	@echo "Compiling C source to assembly CMakeFiles/sickverushash.dir/haraka_portable.c.s"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && /usr/bin/gcc-10 $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash/haraka_portable.c -o CMakeFiles/sickverushash.dir/haraka_portable.c.s

src/crypto/verushash/CMakeFiles/sickverushash.dir/ripemd160.cpp.o: src/crypto/verushash/CMakeFiles/sickverushash.dir/flags.make
src/crypto/verushash/CMakeFiles/sickverushash.dir/ripemd160.cpp.o: src/crypto/verushash/ripemd160.cpp
src/crypto/verushash/CMakeFiles/sickverushash.dir/ripemd160.cpp.o: src/crypto/verushash/CMakeFiles/sickverushash.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --progress-dir=/home/sickguy/Documents/Projects/SickPool/server/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object src/crypto/verushash/CMakeFiles/sickverushash.dir/ripemd160.cpp.o"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && /usr/bin/g++-10 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/crypto/verushash/CMakeFiles/sickverushash.dir/ripemd160.cpp.o -MF CMakeFiles/sickverushash.dir/ripemd160.cpp.o.d -o CMakeFiles/sickverushash.dir/ripemd160.cpp.o -c /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash/ripemd160.cpp

src/crypto/verushash/CMakeFiles/sickverushash.dir/ripemd160.cpp.i: cmake_force
	@echo "Preprocessing CXX source to CMakeFiles/sickverushash.dir/ripemd160.cpp.i"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && /usr/bin/g++-10 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash/ripemd160.cpp > CMakeFiles/sickverushash.dir/ripemd160.cpp.i

src/crypto/verushash/CMakeFiles/sickverushash.dir/ripemd160.cpp.s: cmake_force
	@echo "Compiling CXX source to assembly CMakeFiles/sickverushash.dir/ripemd160.cpp.s"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && /usr/bin/g++-10 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash/ripemd160.cpp -o CMakeFiles/sickverushash.dir/ripemd160.cpp.s

src/crypto/verushash/CMakeFiles/sickverushash.dir/sha256.cpp.o: src/crypto/verushash/CMakeFiles/sickverushash.dir/flags.make
src/crypto/verushash/CMakeFiles/sickverushash.dir/sha256.cpp.o: src/crypto/verushash/sha256.cpp
src/crypto/verushash/CMakeFiles/sickverushash.dir/sha256.cpp.o: src/crypto/verushash/CMakeFiles/sickverushash.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --progress-dir=/home/sickguy/Documents/Projects/SickPool/server/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object src/crypto/verushash/CMakeFiles/sickverushash.dir/sha256.cpp.o"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && /usr/bin/g++-10 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/crypto/verushash/CMakeFiles/sickverushash.dir/sha256.cpp.o -MF CMakeFiles/sickverushash.dir/sha256.cpp.o.d -o CMakeFiles/sickverushash.dir/sha256.cpp.o -c /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash/sha256.cpp

src/crypto/verushash/CMakeFiles/sickverushash.dir/sha256.cpp.i: cmake_force
	@echo "Preprocessing CXX source to CMakeFiles/sickverushash.dir/sha256.cpp.i"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && /usr/bin/g++-10 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash/sha256.cpp > CMakeFiles/sickverushash.dir/sha256.cpp.i

src/crypto/verushash/CMakeFiles/sickverushash.dir/sha256.cpp.s: cmake_force
	@echo "Compiling CXX source to assembly CMakeFiles/sickverushash.dir/sha256.cpp.s"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && /usr/bin/g++-10 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash/sha256.cpp -o CMakeFiles/sickverushash.dir/sha256.cpp.s

src/crypto/verushash/CMakeFiles/sickverushash.dir/uint256.cpp.o: src/crypto/verushash/CMakeFiles/sickverushash.dir/flags.make
src/crypto/verushash/CMakeFiles/sickverushash.dir/uint256.cpp.o: src/crypto/verushash/uint256.cpp
src/crypto/verushash/CMakeFiles/sickverushash.dir/uint256.cpp.o: src/crypto/verushash/CMakeFiles/sickverushash.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --progress-dir=/home/sickguy/Documents/Projects/SickPool/server/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object src/crypto/verushash/CMakeFiles/sickverushash.dir/uint256.cpp.o"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && /usr/bin/g++-10 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/crypto/verushash/CMakeFiles/sickverushash.dir/uint256.cpp.o -MF CMakeFiles/sickverushash.dir/uint256.cpp.o.d -o CMakeFiles/sickverushash.dir/uint256.cpp.o -c /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash/uint256.cpp

src/crypto/verushash/CMakeFiles/sickverushash.dir/uint256.cpp.i: cmake_force
	@echo "Preprocessing CXX source to CMakeFiles/sickverushash.dir/uint256.cpp.i"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && /usr/bin/g++-10 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash/uint256.cpp > CMakeFiles/sickverushash.dir/uint256.cpp.i

src/crypto/verushash/CMakeFiles/sickverushash.dir/uint256.cpp.s: cmake_force
	@echo "Compiling CXX source to assembly CMakeFiles/sickverushash.dir/uint256.cpp.s"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && /usr/bin/g++-10 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash/uint256.cpp -o CMakeFiles/sickverushash.dir/uint256.cpp.s

src/crypto/verushash/CMakeFiles/sickverushash.dir/util/strencodings.cpp.o: src/crypto/verushash/CMakeFiles/sickverushash.dir/flags.make
src/crypto/verushash/CMakeFiles/sickverushash.dir/util/strencodings.cpp.o: src/crypto/verushash/util/strencodings.cpp
src/crypto/verushash/CMakeFiles/sickverushash.dir/util/strencodings.cpp.o: src/crypto/verushash/CMakeFiles/sickverushash.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --progress-dir=/home/sickguy/Documents/Projects/SickPool/server/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object src/crypto/verushash/CMakeFiles/sickverushash.dir/util/strencodings.cpp.o"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && /usr/bin/g++-10 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/crypto/verushash/CMakeFiles/sickverushash.dir/util/strencodings.cpp.o -MF CMakeFiles/sickverushash.dir/util/strencodings.cpp.o.d -o CMakeFiles/sickverushash.dir/util/strencodings.cpp.o -c /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash/util/strencodings.cpp

src/crypto/verushash/CMakeFiles/sickverushash.dir/util/strencodings.cpp.i: cmake_force
	@echo "Preprocessing CXX source to CMakeFiles/sickverushash.dir/util/strencodings.cpp.i"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && /usr/bin/g++-10 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash/util/strencodings.cpp > CMakeFiles/sickverushash.dir/util/strencodings.cpp.i

src/crypto/verushash/CMakeFiles/sickverushash.dir/util/strencodings.cpp.s: cmake_force
	@echo "Compiling CXX source to assembly CMakeFiles/sickverushash.dir/util/strencodings.cpp.s"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && /usr/bin/g++-10 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash/util/strencodings.cpp -o CMakeFiles/sickverushash.dir/util/strencodings.cpp.s

src/crypto/verushash/CMakeFiles/sickverushash.dir/utilstrencodings.cpp.o: src/crypto/verushash/CMakeFiles/sickverushash.dir/flags.make
src/crypto/verushash/CMakeFiles/sickverushash.dir/utilstrencodings.cpp.o: src/crypto/verushash/utilstrencodings.cpp
src/crypto/verushash/CMakeFiles/sickverushash.dir/utilstrencodings.cpp.o: src/crypto/verushash/CMakeFiles/sickverushash.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --progress-dir=/home/sickguy/Documents/Projects/SickPool/server/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object src/crypto/verushash/CMakeFiles/sickverushash.dir/utilstrencodings.cpp.o"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && /usr/bin/g++-10 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/crypto/verushash/CMakeFiles/sickverushash.dir/utilstrencodings.cpp.o -MF CMakeFiles/sickverushash.dir/utilstrencodings.cpp.o.d -o CMakeFiles/sickverushash.dir/utilstrencodings.cpp.o -c /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash/utilstrencodings.cpp

src/crypto/verushash/CMakeFiles/sickverushash.dir/utilstrencodings.cpp.i: cmake_force
	@echo "Preprocessing CXX source to CMakeFiles/sickverushash.dir/utilstrencodings.cpp.i"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && /usr/bin/g++-10 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash/utilstrencodings.cpp > CMakeFiles/sickverushash.dir/utilstrencodings.cpp.i

src/crypto/verushash/CMakeFiles/sickverushash.dir/utilstrencodings.cpp.s: cmake_force
	@echo "Compiling CXX source to assembly CMakeFiles/sickverushash.dir/utilstrencodings.cpp.s"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && /usr/bin/g++-10 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash/utilstrencodings.cpp -o CMakeFiles/sickverushash.dir/utilstrencodings.cpp.s

src/crypto/verushash/CMakeFiles/sickverushash.dir/verus_clhash.cpp.o: src/crypto/verushash/CMakeFiles/sickverushash.dir/flags.make
src/crypto/verushash/CMakeFiles/sickverushash.dir/verus_clhash.cpp.o: src/crypto/verushash/verus_clhash.cpp
src/crypto/verushash/CMakeFiles/sickverushash.dir/verus_clhash.cpp.o: src/crypto/verushash/CMakeFiles/sickverushash.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --progress-dir=/home/sickguy/Documents/Projects/SickPool/server/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building CXX object src/crypto/verushash/CMakeFiles/sickverushash.dir/verus_clhash.cpp.o"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && /usr/bin/g++-10 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS)  -mpclmul -msse4 -msse4.1 -msse4.2 -mssse3 -mavx -maes -g -funroll-loops -fomit-frame-pointer -MD -MT src/crypto/verushash/CMakeFiles/sickverushash.dir/verus_clhash.cpp.o -MF CMakeFiles/sickverushash.dir/verus_clhash.cpp.o.d -o CMakeFiles/sickverushash.dir/verus_clhash.cpp.o -c /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash/verus_clhash.cpp

src/crypto/verushash/CMakeFiles/sickverushash.dir/verus_clhash.cpp.i: cmake_force
	@echo "Preprocessing CXX source to CMakeFiles/sickverushash.dir/verus_clhash.cpp.i"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && /usr/bin/g++-10 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS)  -mpclmul -msse4 -msse4.1 -msse4.2 -mssse3 -mavx -maes -g -funroll-loops -fomit-frame-pointer -E /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash/verus_clhash.cpp > CMakeFiles/sickverushash.dir/verus_clhash.cpp.i

src/crypto/verushash/CMakeFiles/sickverushash.dir/verus_clhash.cpp.s: cmake_force
	@echo "Compiling CXX source to assembly CMakeFiles/sickverushash.dir/verus_clhash.cpp.s"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && /usr/bin/g++-10 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS)  -mpclmul -msse4 -msse4.1 -msse4.2 -mssse3 -mavx -maes -g -funroll-loops -fomit-frame-pointer -S /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash/verus_clhash.cpp -o CMakeFiles/sickverushash.dir/verus_clhash.cpp.s

src/crypto/verushash/CMakeFiles/sickverushash.dir/verus_clhash_portable.cpp.o: src/crypto/verushash/CMakeFiles/sickverushash.dir/flags.make
src/crypto/verushash/CMakeFiles/sickverushash.dir/verus_clhash_portable.cpp.o: src/crypto/verushash/verus_clhash_portable.cpp
src/crypto/verushash/CMakeFiles/sickverushash.dir/verus_clhash_portable.cpp.o: src/crypto/verushash/CMakeFiles/sickverushash.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --progress-dir=/home/sickguy/Documents/Projects/SickPool/server/CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Building CXX object src/crypto/verushash/CMakeFiles/sickverushash.dir/verus_clhash_portable.cpp.o"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && /usr/bin/g++-10 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/crypto/verushash/CMakeFiles/sickverushash.dir/verus_clhash_portable.cpp.o -MF CMakeFiles/sickverushash.dir/verus_clhash_portable.cpp.o.d -o CMakeFiles/sickverushash.dir/verus_clhash_portable.cpp.o -c /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash/verus_clhash_portable.cpp

src/crypto/verushash/CMakeFiles/sickverushash.dir/verus_clhash_portable.cpp.i: cmake_force
	@echo "Preprocessing CXX source to CMakeFiles/sickverushash.dir/verus_clhash_portable.cpp.i"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && /usr/bin/g++-10 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash/verus_clhash_portable.cpp > CMakeFiles/sickverushash.dir/verus_clhash_portable.cpp.i

src/crypto/verushash/CMakeFiles/sickverushash.dir/verus_clhash_portable.cpp.s: cmake_force
	@echo "Compiling CXX source to assembly CMakeFiles/sickverushash.dir/verus_clhash_portable.cpp.s"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && /usr/bin/g++-10 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash/verus_clhash_portable.cpp -o CMakeFiles/sickverushash.dir/verus_clhash_portable.cpp.s

src/crypto/verushash/CMakeFiles/sickverushash.dir/verus_hash.cpp.o: src/crypto/verushash/CMakeFiles/sickverushash.dir/flags.make
src/crypto/verushash/CMakeFiles/sickverushash.dir/verus_hash.cpp.o: src/crypto/verushash/verus_hash.cpp
src/crypto/verushash/CMakeFiles/sickverushash.dir/verus_hash.cpp.o: src/crypto/verushash/CMakeFiles/sickverushash.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --progress-dir=/home/sickguy/Documents/Projects/SickPool/server/CMakeFiles --progress-num=$(CMAKE_PROGRESS_11) "Building CXX object src/crypto/verushash/CMakeFiles/sickverushash.dir/verus_hash.cpp.o"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && /usr/bin/g++-10 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS)  -mpclmul -msse4 -msse4.1 -msse4.2 -mssse3 -mavx -maes -g -funroll-loops -fomit-frame-pointer -MD -MT src/crypto/verushash/CMakeFiles/sickverushash.dir/verus_hash.cpp.o -MF CMakeFiles/sickverushash.dir/verus_hash.cpp.o.d -o CMakeFiles/sickverushash.dir/verus_hash.cpp.o -c /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash/verus_hash.cpp

src/crypto/verushash/CMakeFiles/sickverushash.dir/verus_hash.cpp.i: cmake_force
	@echo "Preprocessing CXX source to CMakeFiles/sickverushash.dir/verus_hash.cpp.i"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && /usr/bin/g++-10 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS)  -mpclmul -msse4 -msse4.1 -msse4.2 -mssse3 -mavx -maes -g -funroll-loops -fomit-frame-pointer -E /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash/verus_hash.cpp > CMakeFiles/sickverushash.dir/verus_hash.cpp.i

src/crypto/verushash/CMakeFiles/sickverushash.dir/verus_hash.cpp.s: cmake_force
	@echo "Compiling CXX source to assembly CMakeFiles/sickverushash.dir/verus_hash.cpp.s"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && /usr/bin/g++-10 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS)  -mpclmul -msse4 -msse4.1 -msse4.2 -mssse3 -mavx -maes -g -funroll-loops -fomit-frame-pointer -S /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash/verus_hash.cpp -o CMakeFiles/sickverushash.dir/verus_hash.cpp.s

# Object files for target sickverushash
sickverushash_OBJECTS = \
"CMakeFiles/sickverushash.dir/arith_uint256.cpp.o" \
"CMakeFiles/sickverushash.dir/haraka.c.o" \
"CMakeFiles/sickverushash.dir/haraka_portable.c.o" \
"CMakeFiles/sickverushash.dir/ripemd160.cpp.o" \
"CMakeFiles/sickverushash.dir/sha256.cpp.o" \
"CMakeFiles/sickverushash.dir/uint256.cpp.o" \
"CMakeFiles/sickverushash.dir/util/strencodings.cpp.o" \
"CMakeFiles/sickverushash.dir/utilstrencodings.cpp.o" \
"CMakeFiles/sickverushash.dir/verus_clhash.cpp.o" \
"CMakeFiles/sickverushash.dir/verus_clhash_portable.cpp.o" \
"CMakeFiles/sickverushash.dir/verus_hash.cpp.o"

# External object files for target sickverushash
sickverushash_EXTERNAL_OBJECTS =

src/crypto/verushash/libsickverushash.a: src/crypto/verushash/CMakeFiles/sickverushash.dir/arith_uint256.cpp.o
src/crypto/verushash/libsickverushash.a: src/crypto/verushash/CMakeFiles/sickverushash.dir/haraka.c.o
src/crypto/verushash/libsickverushash.a: src/crypto/verushash/CMakeFiles/sickverushash.dir/haraka_portable.c.o
src/crypto/verushash/libsickverushash.a: src/crypto/verushash/CMakeFiles/sickverushash.dir/ripemd160.cpp.o
src/crypto/verushash/libsickverushash.a: src/crypto/verushash/CMakeFiles/sickverushash.dir/sha256.cpp.o
src/crypto/verushash/libsickverushash.a: src/crypto/verushash/CMakeFiles/sickverushash.dir/uint256.cpp.o
src/crypto/verushash/libsickverushash.a: src/crypto/verushash/CMakeFiles/sickverushash.dir/util/strencodings.cpp.o
src/crypto/verushash/libsickverushash.a: src/crypto/verushash/CMakeFiles/sickverushash.dir/utilstrencodings.cpp.o
src/crypto/verushash/libsickverushash.a: src/crypto/verushash/CMakeFiles/sickverushash.dir/verus_clhash.cpp.o
src/crypto/verushash/libsickverushash.a: src/crypto/verushash/CMakeFiles/sickverushash.dir/verus_clhash_portable.cpp.o
src/crypto/verushash/libsickverushash.a: src/crypto/verushash/CMakeFiles/sickverushash.dir/verus_hash.cpp.o
src/crypto/verushash/libsickverushash.a: src/crypto/verushash/CMakeFiles/sickverushash.dir/build.make
src/crypto/verushash/libsickverushash.a: src/crypto/verushash/CMakeFiles/sickverushash.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --progress-dir=/home/sickguy/Documents/Projects/SickPool/server/CMakeFiles --progress-num=$(CMAKE_PROGRESS_12) "Linking CXX static library libsickverushash.a"
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && $(CMAKE_COMMAND) -P CMakeFiles/sickverushash.dir/cmake_clean_target.cmake
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/sickverushash.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/crypto/verushash/CMakeFiles/sickverushash.dir/build: src/crypto/verushash/libsickverushash.a
.PHONY : src/crypto/verushash/CMakeFiles/sickverushash.dir/build

src/crypto/verushash/CMakeFiles/sickverushash.dir/clean:
	cd /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash && $(CMAKE_COMMAND) -P CMakeFiles/sickverushash.dir/cmake_clean.cmake
.PHONY : src/crypto/verushash/CMakeFiles/sickverushash.dir/clean

src/crypto/verushash/CMakeFiles/sickverushash.dir/depend:
	cd /home/sickguy/Documents/Projects/SickPool/server && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/sickguy/Documents/Projects/SickPool/server /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash /home/sickguy/Documents/Projects/SickPool/server /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash /home/sickguy/Documents/Projects/SickPool/server/src/crypto/verushash/CMakeFiles/sickverushash.dir/DependInfo.cmake
.PHONY : src/crypto/verushash/CMakeFiles/sickverushash.dir/depend

