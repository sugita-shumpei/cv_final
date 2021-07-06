add_executable(test1 ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
target_include_directories(test1 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(test1 cvlib)