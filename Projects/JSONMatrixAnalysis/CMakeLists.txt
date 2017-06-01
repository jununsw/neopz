file(GLOB headers *.h *.hpp)
file(GLOB sources *.cpp)
file(GLOB data *.json)

set(main_SRCS ${sources} ${headers} ${data})
add_executable(JSONMatrixAnalysis ${main_SRCS})
target_link_libraries(JSONMatrixAnalysis pz)


file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Debug)

configure_file(InputJSON.json Debug COPYONLY)
