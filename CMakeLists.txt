cmake_minimum_required(VERSION 2.8)
project(MiniJS)

set(MINIJS_SOURCES
	src/ast.cc
	src/evaluate.cc
	src/execute.cc
	src/miniscript.cc
	src/runtime.cc
)

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

BISON_TARGET(PARSER src/parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp)
FLEX_TARGET(SCANNER src/lexer.l  ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)
ADD_FLEX_BISON_DEPENDENCY(SCANNER PARSER)

add_executable(minijs
	${MINIJS_SOURCES}
	${BISON_PARSER_OUTPUTS}
	${FLEX_SCANNER_OUTPUTS}
)

target_compile_options(minijs PRIVATE -Wall;-std=c++11;-g)
target_include_directories(minijs PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(minijs ${FLEX_LIBRARIES} ${BISON_LIBRARIES})

install(TARGETS minijs RUNTIME DESTINATION bin)
