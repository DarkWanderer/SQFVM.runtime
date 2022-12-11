cmake_minimum_required(VERSION 2.8.12)
project(sqfvm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MACOSX_RPATH 1)

# Enable multithreaded compilation in MSVC
if (MSVC)
  add_definitions(/MP /W4 /wd4100)
else()
  add_definitions(-Wall -Wno-unknown-pragmas)

  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_link_options(-static-libstdc++ -static-libgcc)
  endif()
endif()

# Add the filesystem library if we are building on Clang or GCC
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(ST_CXXFS_LIBS stdc++fs)
endif()

set(ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/output)
set(LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/output)
set(RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/output)

include_directories("${PROJECT_SOURCE_DIR}/include/tclap-1.2.2/include")

option(SQFVM_ENABLE_SQC_SUPPORT "ENABLE SQC SUPPORT IN BUILD" OFF)
option(SQFVM_ENABLE_FULL_DIAGNOSE "ENABLE FULL DIAGNOSTICS" OFF)
option(SQFVM_ENABLE_ARMA2_LOCALKEYWORD "ENABLE ARMA2 LOCALKEYWORD" OFF)

#####################
##                 ##
##  FILES SECTION  ##
##                 ##
#####################


file(GLOB_RECURSE sqfvm_SOURCES CONFIGURE_DEPENDS src/*.cpp src/*.hpp src/*.h src/*.hh src/*.c src/*.cc)
list(FILTER sqfvm_SOURCES EXCLUDE REGEX ".*src/unused/.*")
if (NOT SQFVM_ENABLE_SQC_SUPPORT)
    list(FILTER sqfvm_SOURCES EXCLUDE REGEX ".*src/sqc/.*")
endif()

# Get the local git revision hash and put it into a header we can use
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

configure_file("${PROJECT_SOURCE_DIR}/cmake/git_sha1.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/git_sha1.cpp" @ONLY)
list(APPEND sqfvm_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/git_sha1.cpp")

############################
##                        ##
##  DEPENDENCIES SECTION  ##
##                        ##
############################

find_package(Threads)

add_library(sqfvm-object OBJECT ${sqfvm_SOURCES})
set_property(TARGET sqfvm-object PROPERTY POSITION_INDEPENDENT_CODE 1)
target_include_directories(sqfvm-object PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_compile_definitions(sqfvm-object PRIVATE SQFVM_BUILD)

if (SQFVM_ENABLE_SQC_SUPPORT)
    target_compile_definitions(sqfvm-object PUBLIC SQF_SQC_SUPPORT)
endif()

if (SQFVM_ENABLE_FULL_DIAGNOSE)
    target_compile_definitions(sqfvm-object PUBLIC 
        DF__SQF_RUNTIME__ASSEMBLY_DEBUG_ON_EXECUTE
        DF__SQF_FILEIO__ALL_FILES
        DF__SQF_FILEIO__TRACE_REESOLVE
        DF__SQF_PREPROC__TRACE_MACRO_RESOLVE
        DF__SQF_PREPROC__TRACE_MACRO_PARSE
        DF__SQF_CONFIG__REPORT_PROGRESS_BY_LINE
        DF__SQF_RUNTIME__VALUE_SCOPE_DEBUG
        DF__CLI_PRINT_INPUT_ARGS
        DF__CLI_PRINT_TCLAP_ARGUMENTS
    )
endif ()


if (SQFVM_ENABLE_ARMA2_LOCALKEYWORD)
    target_compile_definitions(sqfvm-object PUBLIC SQFVM_ARMA2_SYNTAX)
endif ()

add_executable(sqfvm $<TARGET_OBJECTS:sqfvm-object>)
target_link_libraries(sqfvm ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ${ST_CXXFS_LIBS})

add_library(libsqfvm SHARED $<TARGET_OBJECTS:sqfvm-object>)
set_target_properties(libsqfvm PROPERTIES PREFIX "")

enable_testing()
add_subdirectory(tests)
