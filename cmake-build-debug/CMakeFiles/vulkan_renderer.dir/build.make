# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.17

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
CMAKE_COMMAND = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake

# The command to remove a file.
RM = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/fanqilei/github/vulkan-renderer

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/fanqilei/github/vulkan-renderer/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/vulkan_renderer.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/vulkan_renderer.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/vulkan_renderer.dir/flags.make

CMakeFiles/vulkan_renderer.dir/vulkan-renderer/main.cpp.o: CMakeFiles/vulkan_renderer.dir/flags.make
CMakeFiles/vulkan_renderer.dir/vulkan-renderer/main.cpp.o: ../vulkan-renderer/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/fanqilei/github/vulkan-renderer/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/vulkan_renderer.dir/vulkan-renderer/main.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/vulkan_renderer.dir/vulkan-renderer/main.cpp.o -c /Users/fanqilei/github/vulkan-renderer/vulkan-renderer/main.cpp

CMakeFiles/vulkan_renderer.dir/vulkan-renderer/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/vulkan_renderer.dir/vulkan-renderer/main.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/fanqilei/github/vulkan-renderer/vulkan-renderer/main.cpp > CMakeFiles/vulkan_renderer.dir/vulkan-renderer/main.cpp.i

CMakeFiles/vulkan_renderer.dir/vulkan-renderer/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/vulkan_renderer.dir/vulkan-renderer/main.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/fanqilei/github/vulkan-renderer/vulkan-renderer/main.cpp -o CMakeFiles/vulkan_renderer.dir/vulkan-renderer/main.cpp.s

# Object files for target vulkan_renderer
vulkan_renderer_OBJECTS = \
"CMakeFiles/vulkan_renderer.dir/vulkan-renderer/main.cpp.o"

# External object files for target vulkan_renderer
vulkan_renderer_EXTERNAL_OBJECTS =

vulkan_renderer: CMakeFiles/vulkan_renderer.dir/vulkan-renderer/main.cpp.o
vulkan_renderer: CMakeFiles/vulkan_renderer.dir/build.make
vulkan_renderer: CMakeFiles/vulkan_renderer.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/fanqilei/github/vulkan-renderer/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable vulkan_renderer"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/vulkan_renderer.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/vulkan_renderer.dir/build: vulkan_renderer

.PHONY : CMakeFiles/vulkan_renderer.dir/build

CMakeFiles/vulkan_renderer.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/vulkan_renderer.dir/cmake_clean.cmake
.PHONY : CMakeFiles/vulkan_renderer.dir/clean

CMakeFiles/vulkan_renderer.dir/depend:
	cd /Users/fanqilei/github/vulkan-renderer/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/fanqilei/github/vulkan-renderer /Users/fanqilei/github/vulkan-renderer /Users/fanqilei/github/vulkan-renderer/cmake-build-debug /Users/fanqilei/github/vulkan-renderer/cmake-build-debug /Users/fanqilei/github/vulkan-renderer/cmake-build-debug/CMakeFiles/vulkan_renderer.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/vulkan_renderer.dir/depend

