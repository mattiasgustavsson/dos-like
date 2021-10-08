cmake_minimum_required(VERSION 3.10)

set(CMAKE_VERBOSE_MAKEFILE on)

project("dos-like" LANGUAGES C)

find_package(GLEW REQUIRED)
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)
find_library(MATH_LIBRARY m) # Required if available, not required if unavailable (like on Windows).

add_library(dos_like STATIC source/dos.c)
target_link_libraries(dos_like PRIVATE
	${CMAKE_DL_LIBS}
	GLEW::glew
	${OPENGL_LIBRARIES}
	Threads::Threads
	${MATH_LIBRARY}
)

find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
	# Works on most Unix-type systems (Linux, macOS)

	function(target_pkg_config_import TARGET DEPENDENCY)
		string(TOUPPER ${DEPENDENCY} DEPENDENCY_VARIABLE)
		pkg_check_modules(${DEPENDENCY_VARIABLE} REQUIRED ${DEPENDENCY})
		target_compile_options(${TARGET} PRIVATE "${${DEPENDENCY_VARIABLE}_CFLAGS}")
		target_compile_options(${TARGET} PRIVATE "${${DEPENDENCY_VARIABLE}_CFLAGS_OTHER}")
		target_include_directories(${TARGET} SYSTEM PRIVATE "${${DEPENDENCY_VARIABLE}_INCLUDE_DIRS}")
		target_link_directories(${TARGET} PRIVATE "${${DEPENDENCY_VARIABLE}_LIBRARY_DIRS}")
		target_link_libraries(${TARGET} PRIVATE "${${DEPENDENCY_VARIABLE}_LIBRARIES}")
		target_link_options(${TARGET} PRIVATE "${${DEPENDENCY_VARIABLE}_LDFLAGS}")
		target_link_options(${TARGET} PRIVATE "${${DEPENDENCY_VARIABLE}_LDFLAGS_OTHER}")
	endfunction()

	set(DEPENDENCIES
		sdl2
	)
	foreach(DEPENDENCY ${DEPENDENCIES})
		target_pkg_config_import(dos_like ${DEPENDENCY})
	endforeach()
else()
	# This *can* work without pkg-config, but only if SDL 2.0 is available with find_package().
	# The pkg-config version above is for systems with older SDL 2.0 versions, that don't come with CMake support.

	find_package(SDL2 REQUIRED)

	target_link_libraries(dos_like PRIVATE
		SDL2::SDL2
	)
	set(SDL2_MAIN SDL2::SDL2main)
endif()

set(EXAMPLES
	burn
	julia
	mandelbrot
	plasma
	raycast
	rotozoom
	stranded
	tracker
	tunnel
	voxel
)

foreach(EXAMPLE ${EXAMPLES})
	add_executable(${EXAMPLE} source/${EXAMPLE}.c)
	target_link_libraries(${EXAMPLE} PRIVATE dos_like ${SDL2_MAIN})
endforeach()
