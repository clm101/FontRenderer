# minimum cmake version
cmake_minimum_required(VERSION 3.21)

# set project name and version
project(Application VERSION 1.0)

set(MSVC_TOOLSET_VERSION 143)

# C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(Application)

set(EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external")
if(NOT EXISTS ${EXTERNAL_DIR})
	file(MAKE_DIRECTORY ${EXTERNAL_DIR})
endif()

# set source files
target_sources(
	Application
	PRIVATE 
	"EventSystem.cpp"
	"File.cpp"
	"Font.cpp"
	"Keyboard.cpp"
	"KeyboardInfo.cpp"
	"main.cpp"
	"win32.cpp"
	"WindowsMessageMap.cpp"
	"Application.cpp"
	"Console.cpp"
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

include(FetchContent)
find_package(Git)
set(CLM_LIBRARY_DIR "${EXTERNAL_DIR}/clmLibrary")
if(NOT clmLibrary_POPULATED)
	FetchContent_Populate(clmLibrary
							GIT_REPOSITORY "https://github.com/clm101/clmLibrary.git"
							GIT_TAG "origin/master"
							SOURCE_DIR ${CLM_LIBRARY_DIR}
	)
	
	add_subdirectory(${clmlibrary_SOURCE_DIR} ${clmlibrary_BINARY_DIR})
endif()

find_package(Vulkan MODULE REQUIRED)

find_path(
	SHADER_DIR_PATH
	NAMES Shaders
	HINTS ${PROJECT_SOURCE_DIR}
)
configure_file(BuildSystemInput.h.in BuildSystemInput.h)

target_include_directories(
							Application 
							PRIVATE 
							"${Vulkan_INCLUDE_DIR}"
							"${CMAKE_CURRENT_SOURCE_DIR}"
							"${CMAKE_CURRENT_BINARY_DIR}"
)

option(USE_GFX_REFAC "Use the refactored graphics code")
if(USE_GFX_REFAC)
	add_compile_definitions("GFX_REFAC")
endif()

option(DISPLAY_VULKAN_INIT_INFO "Display the layer, instance, and device extensions")
if(DISPLAY_VULKAN_INIT_INFO)
	add_compile_definitions("VULKAN_INIT_INFO")
endif()

target_compile_options(
	Application
	PRIVATE
	"/W4"
)

add_subdirectory(
	"${CMAKE_CURRENT_SOURCE_DIR}/Mesh"
)
add_subdirectory(
	"${CMAKE_CURRENT_SOURCE_DIR}/Delaunay"
)
add_subdirectory(
	"${CMAKE_CURRENT_SOURCE_DIR}/Graphics"
)

target_link_libraries(Application PRIVATE clmLibrary ${Vulkan_LIBRARY})