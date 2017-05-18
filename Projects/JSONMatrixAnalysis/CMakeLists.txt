file(GLOB headers *.h *.hpp)
file(GLOB sources *.cpp)
file(GLOB data *.json)

set(main_SRCS ${sources} ${headers} ${data})
set(CMAKE_CXX_FLAGS "-std=c++11")

add_executable(JSONMatrixAnalysis ${main_SRCS})

target_link_libraries(JSONMatrixAnalysis pz)


add_custom_command(TARGET Debug POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
"${PROJECT_SOURCE_DIR}/InputJSON.json" $<TARGET_FILE_DIR:Debug>)