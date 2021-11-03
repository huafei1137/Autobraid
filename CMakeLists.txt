cmake_minimum_required(VERSION 3.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(autobraid VERSION 0.1)

file(GLOB_RECURSE QASM_FILES src/qasm-tools/*.cpp)
add_library(qasm STATIC ${QASM_FILES})
target_include_directories(qasm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/qasm-tools)

find_library(LIBMETIS metis)
set(METIS_INCLUDE_DIR /usr/local/include)

file(GLOB_RECURSE UTIL_FILES src/utils/*.cpp)
file(GLOB_RECURSE DATA_STRUCTS src/data-structures/*.cpp)

add_executable(autobraid src/main.cpp ${UTIL_FILES} ${DATA_STRUCTS})
target_include_directories(autobraid PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/utils)
target_include_directories(autobraid PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/data-structures)
target_include_directories(autobraid PUBLIC ${METIS_INCLUDE_DIR})
target_link_libraries(autobraid PUBLIC qasm)
target_link_libraries(autobraid PUBLIC ${LIBMETIS})

add_executable(critpath src/test.cpp ${UTIL_FILES} ${DATA_STRUCTS})
target_include_directories(critpath PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/utils)
target_include_directories(critpath PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/data-structures)
target_include_directories(critpath PUBLIC ${METIS_INCLUDE_DIR})
target_link_libraries(critpath PUBLIC qasm)
target_link_libraries(critpath PUBLIC ${LIBMETIS})