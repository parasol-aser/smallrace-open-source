# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/thirdparty/utfcpp

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/utfcpp-prefix/src/utfcpp-build

# Include any dependencies generated for this target.
include tests/CMakeFiles/apitests.dir/depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/apitests.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/apitests.dir/flags.make

tests/CMakeFiles/apitests.dir/test_checked_api.cpp.o: tests/CMakeFiles/apitests.dir/flags.make
tests/CMakeFiles/apitests.dir/test_checked_api.cpp.o: /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/thirdparty/utfcpp/tests/test_checked_api.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/utfcpp-prefix/src/utfcpp-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object tests/CMakeFiles/apitests.dir/test_checked_api.cpp.o"
	cd /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/utfcpp-prefix/src/utfcpp-build/tests && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/apitests.dir/test_checked_api.cpp.o -c /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/thirdparty/utfcpp/tests/test_checked_api.cpp

tests/CMakeFiles/apitests.dir/test_checked_api.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/apitests.dir/test_checked_api.cpp.i"
	cd /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/utfcpp-prefix/src/utfcpp-build/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/thirdparty/utfcpp/tests/test_checked_api.cpp > CMakeFiles/apitests.dir/test_checked_api.cpp.i

tests/CMakeFiles/apitests.dir/test_checked_api.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/apitests.dir/test_checked_api.cpp.s"
	cd /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/utfcpp-prefix/src/utfcpp-build/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/thirdparty/utfcpp/tests/test_checked_api.cpp -o CMakeFiles/apitests.dir/test_checked_api.cpp.s

tests/CMakeFiles/apitests.dir/test_unchecked_api.cpp.o: tests/CMakeFiles/apitests.dir/flags.make
tests/CMakeFiles/apitests.dir/test_unchecked_api.cpp.o: /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/thirdparty/utfcpp/tests/test_unchecked_api.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/utfcpp-prefix/src/utfcpp-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object tests/CMakeFiles/apitests.dir/test_unchecked_api.cpp.o"
	cd /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/utfcpp-prefix/src/utfcpp-build/tests && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/apitests.dir/test_unchecked_api.cpp.o -c /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/thirdparty/utfcpp/tests/test_unchecked_api.cpp

tests/CMakeFiles/apitests.dir/test_unchecked_api.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/apitests.dir/test_unchecked_api.cpp.i"
	cd /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/utfcpp-prefix/src/utfcpp-build/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/thirdparty/utfcpp/tests/test_unchecked_api.cpp > CMakeFiles/apitests.dir/test_unchecked_api.cpp.i

tests/CMakeFiles/apitests.dir/test_unchecked_api.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/apitests.dir/test_unchecked_api.cpp.s"
	cd /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/utfcpp-prefix/src/utfcpp-build/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/thirdparty/utfcpp/tests/test_unchecked_api.cpp -o CMakeFiles/apitests.dir/test_unchecked_api.cpp.s

tests/CMakeFiles/apitests.dir/test_checked_iterator.cpp.o: tests/CMakeFiles/apitests.dir/flags.make
tests/CMakeFiles/apitests.dir/test_checked_iterator.cpp.o: /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/thirdparty/utfcpp/tests/test_checked_iterator.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/utfcpp-prefix/src/utfcpp-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object tests/CMakeFiles/apitests.dir/test_checked_iterator.cpp.o"
	cd /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/utfcpp-prefix/src/utfcpp-build/tests && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/apitests.dir/test_checked_iterator.cpp.o -c /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/thirdparty/utfcpp/tests/test_checked_iterator.cpp

tests/CMakeFiles/apitests.dir/test_checked_iterator.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/apitests.dir/test_checked_iterator.cpp.i"
	cd /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/utfcpp-prefix/src/utfcpp-build/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/thirdparty/utfcpp/tests/test_checked_iterator.cpp > CMakeFiles/apitests.dir/test_checked_iterator.cpp.i

tests/CMakeFiles/apitests.dir/test_checked_iterator.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/apitests.dir/test_checked_iterator.cpp.s"
	cd /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/utfcpp-prefix/src/utfcpp-build/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/thirdparty/utfcpp/tests/test_checked_iterator.cpp -o CMakeFiles/apitests.dir/test_checked_iterator.cpp.s

tests/CMakeFiles/apitests.dir/test_unchecked_iterator.cpp.o: tests/CMakeFiles/apitests.dir/flags.make
tests/CMakeFiles/apitests.dir/test_unchecked_iterator.cpp.o: /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/thirdparty/utfcpp/tests/test_unchecked_iterator.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/utfcpp-prefix/src/utfcpp-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object tests/CMakeFiles/apitests.dir/test_unchecked_iterator.cpp.o"
	cd /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/utfcpp-prefix/src/utfcpp-build/tests && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/apitests.dir/test_unchecked_iterator.cpp.o -c /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/thirdparty/utfcpp/tests/test_unchecked_iterator.cpp

tests/CMakeFiles/apitests.dir/test_unchecked_iterator.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/apitests.dir/test_unchecked_iterator.cpp.i"
	cd /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/utfcpp-prefix/src/utfcpp-build/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/thirdparty/utfcpp/tests/test_unchecked_iterator.cpp > CMakeFiles/apitests.dir/test_unchecked_iterator.cpp.i

tests/CMakeFiles/apitests.dir/test_unchecked_iterator.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/apitests.dir/test_unchecked_iterator.cpp.s"
	cd /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/utfcpp-prefix/src/utfcpp-build/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/thirdparty/utfcpp/tests/test_unchecked_iterator.cpp -o CMakeFiles/apitests.dir/test_unchecked_iterator.cpp.s

# Object files for target apitests
apitests_OBJECTS = \
"CMakeFiles/apitests.dir/test_checked_api.cpp.o" \
"CMakeFiles/apitests.dir/test_unchecked_api.cpp.o" \
"CMakeFiles/apitests.dir/test_checked_iterator.cpp.o" \
"CMakeFiles/apitests.dir/test_unchecked_iterator.cpp.o"

# External object files for target apitests
apitests_EXTERNAL_OBJECTS =

tests/apitests: tests/CMakeFiles/apitests.dir/test_checked_api.cpp.o
tests/apitests: tests/CMakeFiles/apitests.dir/test_unchecked_api.cpp.o
tests/apitests: tests/CMakeFiles/apitests.dir/test_checked_iterator.cpp.o
tests/apitests: tests/CMakeFiles/apitests.dir/test_unchecked_iterator.cpp.o
tests/apitests: tests/CMakeFiles/apitests.dir/build.make
tests/apitests: extern/gtest/googlemock/gtest/libgtest_main.a
tests/apitests: extern/gtest/googlemock/gtest/libgtest.a
tests/apitests: tests/CMakeFiles/apitests.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/utfcpp-prefix/src/utfcpp-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Linking CXX executable apitests"
	cd /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/utfcpp-prefix/src/utfcpp-build/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/apitests.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tests/CMakeFiles/apitests.dir/build: tests/apitests

.PHONY : tests/CMakeFiles/apitests.dir/build

tests/CMakeFiles/apitests.dir/clean:
	cd /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/utfcpp-prefix/src/utfcpp-build/tests && $(CMAKE_COMMAND) -P CMakeFiles/apitests.dir/cmake_clean.cmake
.PHONY : tests/CMakeFiles/apitests.dir/clean

tests/CMakeFiles/apitests.dir/depend:
	cd /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/utfcpp-prefix/src/utfcpp-build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/thirdparty/utfcpp /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/thirdparty/utfcpp/tests /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/utfcpp-prefix/src/utfcpp-build /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/utfcpp-prefix/src/utfcpp-build/tests /git/LAM/Smalltalk/build/locals/antlr4cpp/src/antlr4cpp-build/runtime/utfcpp-prefix/src/utfcpp-build/tests/CMakeFiles/apitests.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tests/CMakeFiles/apitests.dir/depend

