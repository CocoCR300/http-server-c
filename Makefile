# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.30

# Default target executed when no arguments are given to make.
default_target: all
.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/coco/projects/http-server-c

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/coco/projects/http-server-c

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --cyan "Running CMake cache editor..."
	/usr/bin/ccmake -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache
.PHONY : edit_cache/fast

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --cyan "Running CMake to regenerate build system..."
	/usr/bin/cmake --regenerate-during-build -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache
.PHONY : rebuild_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /home/coco/projects/http-server-c/CMakeFiles /home/coco/projects/http-server-c//CMakeFiles/progress.marks
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /home/coco/projects/http-server-c/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean
.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -P /home/coco/projects/http-server-c/CMakeFiles/VerifyGlobs.cmake
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named http_server_c

# Build rule for target.
http_server_c: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 http_server_c
.PHONY : http_server_c

# fast build rule for target.
http_server_c/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/http_server_c.dir/build.make CMakeFiles/http_server_c.dir/build
.PHONY : http_server_c/fast

src/app_string.o: src/app_string.c.o
.PHONY : src/app_string.o

# target to build an object file
src/app_string.c.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/http_server_c.dir/build.make CMakeFiles/http_server_c.dir/src/app_string.c.o
.PHONY : src/app_string.c.o

src/app_string.i: src/app_string.c.i
.PHONY : src/app_string.i

# target to preprocess a source file
src/app_string.c.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/http_server_c.dir/build.make CMakeFiles/http_server_c.dir/src/app_string.c.i
.PHONY : src/app_string.c.i

src/app_string.s: src/app_string.c.s
.PHONY : src/app_string.s

# target to generate assembly for a file
src/app_string.c.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/http_server_c.dir/build.make CMakeFiles/http_server_c.dir/src/app_string.c.s
.PHONY : src/app_string.c.s

src/buffer.o: src/buffer.c.o
.PHONY : src/buffer.o

# target to build an object file
src/buffer.c.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/http_server_c.dir/build.make CMakeFiles/http_server_c.dir/src/buffer.c.o
.PHONY : src/buffer.c.o

src/buffer.i: src/buffer.c.i
.PHONY : src/buffer.i

# target to preprocess a source file
src/buffer.c.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/http_server_c.dir/build.make CMakeFiles/http_server_c.dir/src/buffer.c.i
.PHONY : src/buffer.c.i

src/buffer.s: src/buffer.c.s
.PHONY : src/buffer.s

# target to generate assembly for a file
src/buffer.c.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/http_server_c.dir/build.make CMakeFiles/http_server_c.dir/src/buffer.c.s
.PHONY : src/buffer.c.s

src/main.o: src/main.c.o
.PHONY : src/main.o

# target to build an object file
src/main.c.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/http_server_c.dir/build.make CMakeFiles/http_server_c.dir/src/main.c.o
.PHONY : src/main.c.o

src/main.i: src/main.c.i
.PHONY : src/main.i

# target to preprocess a source file
src/main.c.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/http_server_c.dir/build.make CMakeFiles/http_server_c.dir/src/main.c.i
.PHONY : src/main.c.i

src/main.s: src/main.c.s
.PHONY : src/main.s

# target to generate assembly for a file
src/main.c.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/http_server_c.dir/build.make CMakeFiles/http_server_c.dir/src/main.c.s
.PHONY : src/main.c.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... edit_cache"
	@echo "... rebuild_cache"
	@echo "... http_server_c"
	@echo "... src/app_string.o"
	@echo "... src/app_string.i"
	@echo "... src/app_string.s"
	@echo "... src/buffer.o"
	@echo "... src/buffer.i"
	@echo "... src/buffer.s"
	@echo "... src/main.o"
	@echo "... src/main.i"
	@echo "... src/main.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -P /home/coco/projects/http-server-c/CMakeFiles/VerifyGlobs.cmake
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

