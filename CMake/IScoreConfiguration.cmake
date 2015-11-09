# Options

option(ENABLE_LTO "Enable link-time optimization. Won't work on Travis." OFF)
option(ENABLE_OPTIMIZE_CUSTOM "Enable -march=native." OFF)

option(OSSIA_NO_EXAMPLES "Don't build OSSIA examples" True)
option(OSSIA_NO_TESTS "Don't build OSSIA tests" True)

option(ISCORE_COTIRE "Use cotire. Will make the build faster." OFF)
option(ISCORE_COTIRE_ALL_HEADERS "All headers will be put in prefix headers. Faster for CI but slower for development" OFF)

option(ISCORE_USE_DEV_PLUGINS "Build the prototypal plugins" OFF)
option(INTEGRATION_TESTING "Run integration tests" OFF)

option(ISCORE_BUILD_FOR_PACKAGE_MANAGER "Set FHS-friendly install paths" OFF)

option(ISCORE_OPENGL "Use OpenGL for rendering" OFF)
option(ISCORE_IEEE "Use a graphical skin adapted to publication" OFF)
option(ISCORE_WEBSOCKETS "Run a websocket server in the scenario" OFF)
if(ISCORE_OPENGL)
        add_definitions(-DISCORE_OPENGL)
endif()

if(ISCORE_WEBSOCKETS)
  add_definitions(-DISCORE_WEBSOCKETS)
endif()

if(DEPLOYMENT_BUILD)
  add_definitions(-DISCORE_DEPLOYMENT_BUILD)
endif()

if(ISCORE_IEEE)
  add_definitions(-DISCORE_IEEE_SKIN)
endif()

# Note : if building with a Qt installed in e.g. /home/myuser/Qt/ or /Users/Qt or c:\Qt\
# keep in mind that you have to call CMake with :
# $ cmake -DCMAKE_MODULE_PATH={path/to/qt/5.3}/{gcc64,clang,msvc2013...}/lib/cmake/Qt5

# Settings
cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0042 NEW)

set(ISCORE_ROOT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(CTEST_OUTPUT_ON_FAILURE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_ANDROID_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Android")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/CMake;${CMAKE_CURRENT_SOURCE_DIR}/CMake/modules")


if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Za /wd4180 /wd4224")
else()
  # Linux flags
  if(NOT APPLE AND NOT WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}  -Wl,-z,defs -fvisibility=hidden")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}  -Wl,-z,defs -fvisibility=hidden")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS}  -Wl,-z,defs -fvisibility=hidden")
  endif()


  # Common Unix flags
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wabi -Wctor-dtor-privacy -Wnon-virtual-dtor -Wreorder -Wstrict-null-sentinel -Wno-non-template-friend -Woverloaded-virtual -Wno-pmf-conversions -Wsign-promo -Wextra -Wall -Waddress -Waggregate-return -Warray-bounds -Wno-attributes -Wno-builtin-macro-redefined")
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wc++0x-compat -Wcast-align -Wcast-qual -Wchar-subscripts -Wclobbered -Wcomment -Wconversion -Wcoverage-mismatch -Wno-deprecated -Wno-deprecated-declarations -Wdisabled-optimization -Wno-div-by-zero -Wempty-body -Wenum-compare")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -pipe -Wall -Wextra -Wno-unused-parameter -Wno-unknown-pragmas")

  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast")
  if(ENABLE_OPTIMIZE_CUSTOM)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native")
  endif()

  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion
                    OUTPUT_VARIABLE GCC_VERSION)

    if (GCC_VERSION VERSION_LESS 4.9)
      message(FATAL_ERROR "i-score requires at least g++-4.9 to build")
    endif()
    if (GCC_VERSION VERSION_GREATER 5.2 OR GCC_VERSION VERSION_EQUAL 5.2)
      # -Wcast-qual is nice but requires more work...
      # -Wzero-as-null-pointer-constant  is garbage
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wswitch-enum -Wno-div-by-zero -Wsuggest-final-types -Wsuggest-final-methods -Wsuggest-override -Wpointer-arith -Wsuggest-attribute=noreturn -Wno-missing-braces -Wformat=2 -Wno-format-nonliteral -Wpedantic")
      # Too much clutter :set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wsuggest-attribute=const  -Wsuggest-attribute=pure ")
    endif()
  endif()
endif()


if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-DISCORE_DEBUG)
  add_definitions(-DBOOST_MULTI_INDEX_ENABLE_SAFE_MODE)
  add_definitions(-DBOOST_MULTI_INDEX_ENABLE_INVARIANT_CHECKING)
endif()

#if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
#	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything -Wno-c++98-compat -Wno-exit-time-destructors -Wno-padded")
#endif()

# Useful header files
include(WriteCompilerDetectionHeader)
write_compiler_detection_header(
  FILE iscore_compiler_detection.hpp
  PREFIX ISCORE
  COMPILERS GNU Clang AppleClang MSVC
  FEATURES cxx_relaxed_constexpr
  VERSION 3.1
)

include(GetGitRevisionDescription)
get_git_head_revision(GIT_COMMIT_REFSPEC GIT_COMMIT_HASH)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/iscore_git_info.hpp" "
#pragma once
#define GIT_COMMIT ${GIT_COMMIT_HASH}
#define ISCORE_VERSION_MAJOR ${ISCORE_VERSION_MAJOR}
#define ISCORE_VERSION_MINOR ${ISCORE_VERSION_MINOR}
#define ISCORE_VERSION_PATCH ${ISCORE_VERSION_PATCH}
#define ISCORE_VERSION_EXTRA \"${ISCORE_VERSION_EXTRA}\"
")

include(cotire)
