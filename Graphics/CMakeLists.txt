# C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

target_sources(
	Application
	PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/GraphicsDevice.cpp"
)
target_include_directories(
	Application
	PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}"
)