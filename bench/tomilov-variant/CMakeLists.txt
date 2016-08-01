project("versatile")

cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_COLOR_MAKEFILE ON)

if(NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    message(FATAL_ERROR "only clang supported currently")
endif()

include(CheckCXXCompilerFlag)

CHECK_CXX_COMPILER_FLAG("-stdlib=libc++" COMPILER_SUPPORTS_LIBCXX)
if(NOT COMPILER_SUPPORTS_LIBCXX)
    message(FATAL_ERROR "libc++ not installed or libc++ not supported by clang++")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")

CHECK_CXX_COMPILER_FLAG("-std=gnu++1z" COMPILER_SUPPORTS_CXX1Z)
if(NOT COMPILER_SUPPORTS_CXX1Z)
    message(FATAL_ERROR "Compiler does not support C++1z standard")
endif()
add_compile_options("-std=gnu++1z")

#set(CXX_EXTENSIONS ON)
#set_property(TARGET ${TARGETS} PROPERTY CXX_STANDARD 17)
#set_property(TARGET ${TARGETS} PROPERTY CXX_STANDARD_REQUIRED ON)
add_compile_options(-fconstexpr-steps=50000000 -ftemplate-depth=150)
add_compile_options(-ftemplate-backtrace-limit=0 -fconstexpr-backtrace-limit=0 -fdiagnostics-show-template-tree)
add_compile_options(-W -Weverything -Wmissing-include-dirs -Wconversion -Wmismatched-tags -Wuninitialized -Wfatal-errors -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded)
add_compile_options(-Wno-gnu-anonymous-struct -Wno-nested-anon-types)
#add_compile_options(-ftime-report)

set(CMAKE_CXX_FLAGS_DEBUG "-ggdb -fno-inline -DDEBUG=1 -D_DEBUG=1 -D_GLIBCXX_DEBUG=1 -march=x86-64 -mtune=generic")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG=1 -march=native")
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -gline-tables-only -DNDEBUG=1 -march=native")
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG=1 -march=native")
else()
    message(STATUS "Wrong build type selected, default to Debug.")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")

include_directories("include/")
include_directories("test/include/")

set(HEADERS
    "include/versatile/type_traits.hpp"
    "include/versatile/in_place.hpp"
    "include/versatile/recursive_wrapper.hpp"
    "include/versatile/aggregate_wrapper.hpp"
    "include/versatile/visit.hpp"
    "include/versatile/versatile.hpp"
    "include/versatile/variant.hpp"
    "include/versatile/utility.hpp"
    "include/versatile/compare.hpp"
    "include/versatile/io.hpp"
    "include/versatile.hpp"

    "test/include/test/prologue.hpp"
)

add_executable("test_traits"        "test/src/traits.cpp"        ${HEADERS} "test/include/test/traits.hpp")
add_executable("test_versatile"     "test/src/versatile.cpp"     ${HEADERS} "test/include/test/common.hpp" "test/include/test/visit.hpp" "test/include/test/wrappers.hpp" "test/include/test/versatile.hpp")
add_executable("test_variant"       "test/src/variant.cpp"       ${HEADERS} "test/include/test/common.hpp" "test/include/test/visit.hpp" "test/include/test/wrappers.hpp" "test/include/test/variant.hpp")
add_executable("test_boost_variant" "test/src/boost_variant.cpp" ${HEADERS} "test/include/test/common.hpp" "test/include/test/visit.hpp" "test/include/test/boost_variant.hpp")
add_executable("test_eggs_variant"  "test/src/eggs_variant.cpp"  ${HEADERS} "test/include/test/common.hpp" "test/include/test/visit.hpp" "test/include/test/eggs_variant.hpp")
add_executable("test_multivisit"    "test/src/multivisit.cpp"    ${HEADERS} "test/include/test/common.hpp" "test/include/test/visit.hpp" "test/include/test/versatile.hpp" "test/include/test/variant.hpp" "test/include/test/boost_variant.hpp" "test/include/test/eggs_variant.hpp")

set(TESTING_TARGETS
    "test_traits"
    "test_versatile"
    "test_variant"
    "test_boost_variant"
    "test_eggs_variant"
    "test_multivisit"
)

foreach(t ${TESTING_TARGETS})
    set_target_properties(${t} PROPERTIES DEBUG_POSTFIX "d")
endforeach()

add_executable("test_ct_versatile"     "test/src/test_ct_versatile.cpp"     ${HEADERS} "test/include/test/deep_and_hard.hpp")
add_executable("test_ct_variant"       "test/src/test_ct_variant.cpp"       ${HEADERS} "test/include/test/deep_and_hard.hpp")
add_executable("test_ct_eggs_variant"  "test/src/test_ct_eggs_variant.cpp"  ${HEADERS} "test/include/test/deep_and_hard.hpp")
add_executable("test_ct_boost_variant" "test/src/test_ct_boost_variant.cpp" ${HEADERS} "test/include/test/deep_and_hard.hpp")

set(TESTING_CT_TARGETS
    "test_ct_versatile"
    "test_ct_variant"
    "test_ct_eggs_variant"
    "test_ct_boost_variant"
)

foreach(t ${TESTING_CT_TARGETS})
    target_compile_definitions(${t} PUBLIC "-DCOLS=4" "-DROWS=4")
    target_compile_options(${t} PUBLIC "-ftime-report")
    set_target_properties(${t} PROPERTIES DEBUG_POSTFIX "d")
endforeach()

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})
install(TARGETS ${TESTING_TARGETS} ${TESTING_CT_TARGETS}
    RUNTIME DESTINATION "bin/"
    )

# for f in `ls bin/test_*` ; do du -sh $f ; $f && echo -e "\e[1;32mSuccess! \e[0m$f" || echo -e "\e[1;31mFailure! \e[0m$f" ; done ;

#[[
message("Your C++ compiler supports these C++ features:")
foreach(i ${CMAKE_CXX_COMPILE_FEATURES})
  message("${i}")
endforeach()
target_compile_features("test_versatile" PUBLIC ${CMAKE_CXX_COMPILE_FEATURES})
]]

