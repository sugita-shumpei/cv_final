add_executable(test0 ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
target_include_directories(test0 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(test0 cvlib)