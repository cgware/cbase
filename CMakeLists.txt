cmake_minimum_required(VERSION 3.10)

project(cbase LANGUAGES C)

enable_testing()

find_program(LCOV lcov)
find_program(GENHTML genhtml)
add_custom_target(coverage
	COMMAND ${LCOV} -q -c -d ${CMAKE_BINARY_DIR} -o ${CMAKE_BINARY_DIR}/lcov.info
	COMMAND ${GENHTML} -q ${CMAKE_BINARY_DIR}/lcov.info -o ${CMAKE_SOURCE_DIR}/report/coverage
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

if(NOT MSVC)
	if(ARCH STREQUAL "x64")
		set(CMAKE_C_FLAGS "-m64")
	elseif(ARCH STREQUAL "x86")
		set(CMAKE_C_FLAGS "-m32")
	endif()
endif()

file(GLOB_RECURSE cbase_src src/*.h src/*.c)
add_library(cbase ${cbase_src})
target_include_directories(cbase PUBLIC include)
if(NOT MSVC)
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		target_compile_options(cbase PRIVATE -Wall -Wextra -Werror -pedantic -coverage -O0 -ggdb)
		target_link_options(cbase PRIVATE -coverage)
	else()
		target_compile_options(cbase PRIVATE -Wall -Wextra -Werror -pedantic)
	endif()
endif()
set_target_properties(cbase PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin/cbase/${ARCH}-Debug/
	ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin/cbase/${ARCH}-Release/
	PREFIX ""
)

file(GLOB_RECURSE cbase_test_src test/*.h test/*.c)
add_executable(cbase_test ${cbase_test_src})
target_link_libraries(cbase_test cbase)
if(NOT MSVC)
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		target_compile_options(cbase_test PRIVATE -Wall -Wextra -Werror -pedantic -coverage -O0 -ggdb)
		target_link_options(cbase_test PRIVATE -coverage)
	else()
		target_compile_options(cbase_test PRIVATE -Wall -Wextra -Werror -pedantic)
	endif()
endif()
set_target_properties(cbase_test PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin/cbase_test/${ARCH}-Debug/
	RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin/cbase_test/${ARCH}-Release/
)
add_test(cbase_build "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --config ${CMAKE_BUILD_TYPE} --target cbase_test)
add_test(cbase ${CMAKE_SOURCE_DIR}/bin/cbase_test/${ARCH}-${CMAKE_BUILD_TYPE}/cbase_test)
set_tests_properties(cbase PROPERTIES DEPENDS cbase_build)
