cmake_minimum_required(VERSION 3.0.0)

# version number is taken from SCIP
project(SCIPOptSuite)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-psabi")
   # require at least gcc 5
   if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5)
     message(WARNING "GCC version not supported, should be at least 5!")
   endif()
endif()

option(NO_EXTERNAL_CODE "should everything be disabled except the bare minimum necessary for SCIP and SoPLEX" OFF)
option(SOPLEX "should SOPLEX be included" ON)
option(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS "Export all symbols into the DLL" OFF)

if(NOT NO_EXTERNAL_CODE)
   option(PAPILO "should papilo library be linked" ON)
   option(ZIMPL "should zimpl be linked" ON)
   option(GMP "should GMP be linked" ON)
   option(GCG "should GCG be included" ON)
   option(UG "should ug be included" ON)
else()
   option(PAPILO "should papilo library be linked" OFF)
   option(ZIMPL "should zimpl be linked" OFF)
   option(GMP "should GMP be linked" OFF)
   option(GCG "should GCG be included" OFF)
   option(UG "should ug be included" OFF)
   option(ZLIB "should zlib be linked" OFF)
   option(READLINE "should readline be linked" OFF)
   option(IPOPT "should ipopt be linked" OFF)
   option(WORHP "should worhp be linked" OFF)
   option(BOOST "Use Boost (required to build the binary). Disable if you only want to build libsoplex." OFF)
   option(QUADMATH "should quadmath library be used" OFF)
   option(MPFR "Use MPFR" OFF)
   set(SYM none CACHE STRING "options for symmetry computation")
endif()

include_directories(
   soplex/src
   scip/src
   zimpl/src
   papilo/src
   ug/src
   gcg/src
   papilo/external
   )

# make 'Release' the default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# path to e.g. findGMP module
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/Modules;${PROJECT_SOURCE_DIR}/scip/cmake/Modules")

if(ZIMPL)
  find_package(BISON QUIET)
  find_package(FLEX QUIET)
  find_package(GMP QUIET)
  # only add ZIMPL subdirectory if required packages could be found
  if(${BISON_FOUND} AND ${FLEX_FOUND} AND ${GMP_FOUND})
    add_subdirectory(zimpl)
    set(ZIMPL_DIR ${CMAKE_BINARY_DIR})
  endif()
endif()

if(SOPLEX)
  add_subdirectory(soplex)
  set(SOPLEX_DIR ${CMAKE_BINARY_DIR})
else()
  set(LPS "none" CACHE STRING "SCIP option for LP solver")
endif()

if(PAPILO)
  find_package(Boost QUIET)
  if( Boost_FOUND )
    set(PAPILO_NO_BINARIES on)
    add_subdirectory(papilo)
    set(PAPILO_DIR ${CMAKE_BINARY_DIR})
  endif()
endif()

add_subdirectory(scip)

if(GCG AND EXISTS ${PROJECT_SOURCE_DIR}/gcg/CMakeLists.txt)
  add_subdirectory(gcg)
  set(GCG_DIR ${CMAKE_BINARY_DIR})
endif()
if(UG AND EXISTS ${PROJECT_SOURCE_DIR}/ug/CMakeLists.txt)
  add_subdirectory(ug)
endif()

if(PAPILO_NO_BINARIES)
   add_subdirectory(papilo/binaries)
endif()

set(CPACK_PACKAGE_VERSION_MAJOR "${SCIPOptSuite_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${SCIPOptSuite_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${SCIPOptSuite_VERSION_PATCH}")
set(CPACK_PACKAGE_VENDOR "Zuse Institute Berlin")
set(CPACK_NSIS_MODIFY_PATH ON)
set(CPACK_PACKAGE_EXECUTABLES scip;SCIP soplex;SoPlex)
set(CPACK_PACKAGE_CONTACT "SCIP <scip@zib.de>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Toolbox for generating and solving mixed integer linear (MIP) and nonlinear programs (MINLP) and constraint integer programs (CIP)")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "http://scip.zib.de")
# autogenerate dependency information
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_GENERATE_SHLIBS ON)
set(CPACK_DEBIAN_ENABLE_COMPONENT_DEPENDS ON)
include(CPack)
enable_testing()

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  include(FeatureSummary)
  feature_summary(WHAT ALL)
endif()

