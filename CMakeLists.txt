# ======================================================================
#  otsdaq main build file
#
#  cd .../path/to/build/directory
#  source .../path/to/otsdaq-utilities/ups/setup_for_development
#  buildtool
# ======================================================================


# use cmake 2.8 or later
cmake_minimum_required (VERSION 2.8)


project(otsdaq-utilities)
# cetbuildtools contains our cmake modules
SET ( CETBUILDTOOLS_VERSION $ENV{CETBUILDTOOLS_VERSION} )
IF (NOT CETBUILDTOOLS_VERSION)
    MESSAGE (FATAL_ERROR "ERROR: setup cetbuildtools to get the cmake modules")
ENDIF()

set(CMAKE_MODULE_PATH 
        $ENV{ART_DIR}/Modules
        $ENV{CETBUILDTOOLS_DIR}/Modules
        $ENV{XDAQ_DIR}/Modules
        ${CMAKE_MODULE_PATH})
message(STATUS "CMAKE_MODULE_PATH is ${CMAKE_MODULE_PATH}")

include(CetCMakeEnv)
cet_cmake_env()
include(FindUpsXdaq)

cet_set_compiler_flags(DIAGS VIGILANT
 WERROR
 NO_UNDEFINED
 EXTRA_FLAGS -pedantic -Wno-unused-parameter -Wno-unused-but-set-variable -Wno-maybe-uninitialized -Wno-ignored-qualifiers -Wno-unused-variable
 )

cet_report_compiler_flags()

# these are minimum required versions, not the actual product versions
find_ups_product( cetbuildtools v3_03_00 )
find_ups_product( otsdaq v1_00_00 )
find_ups_product( artdaq v0_04_02 )
find_ups_product( artdaq_core v1_00_00 )
find_ups_product( art v1_06_00 )
find_ups_product( fhiclcpp v2_16_00 )
find_ups_product( messagefacility v1_10_00 )
find_ups_product( cpp0x v1_03_00 )
find_ups_product( cetlib v1_03_00 )
find_ups_xdaq( v11_2_7 )
find_ups_root( v5_34_00 )
find_ups_boost( v1_50_0 filesystem
                        program_options
                        unit_test_framework
 )

# Include at the top level to make sure include directory is visible
# everywhere.
#include(ProcessSmc)

# Needed if using old art with new cetbuildtools / CMake.
check_ups_version(art "$ENV{ART_VERSION}" v1_11_00 PRODUCT_OLDER_VAR have_old_art)
if (have_old_art)
  cmake_policy(SET CMP0046 OLD)
endif()
unset(have_old_art)

find_package(MPI QUIET)

# macros for art_dictionary and simple_plugin
include(ArtDictionary)
include(ArtMake)
include(BuildPlugins)
include(CetTest)

# Extra parts for the XDAQ GUI
add_subdirectory(otsdaq-utilities)

# UPS Version files
add_subdirectory(ups)

# WebGUI
FILE(COPY WebGUI DESTINATION Data)
INSTALL(DIRECTORY WebGUI DESTINATION ${product}/${version}/Data)
