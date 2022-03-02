# C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

target_sources(
	Application
	PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/Point.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Edge.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Triangle.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Mesh.cpp"
)

target_include_directories(
	Application
	PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}"
)