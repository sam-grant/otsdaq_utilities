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
        $ENV{ARTDAQ_UTILITIES_DIR}/Modules
        ${CMAKE_MODULE_PATH})
message(STATUS "CMAKE_MODULE_PATH is ${CMAKE_MODULE_PATH}")

include(CetCMakeEnv)
cet_cmake_env()
include(FindUpsXdaq)

cet_set_compiler_flags(
 DIAGS 
 VIGILANT
 WERROR
 NO_UNDEFINED
 EXTRA_FLAGS -pedantic -Wno-unused-parameter -Wno-unused-but-set-variable -Wno-maybe-uninitialized -Wno-ignored-qualifiers -Wno-unused-variable
 )

cet_remove_compiler_flags(-std=c++98)

cet_report_compiler_flags()

# these are minimum required versions, not the actual product versions
#find_ups_product( epics v3_15_4)
find_ups_product( cetbuildtools v3_03_00 )
find_ups_product( otsdaq v1_00_00 )
find_ups_product( artdaq v0_04_02 )
find_ups_product( artdaq_core v1_00_00 )
find_ups_product( artdaq_utilities v1_00_00 )
find_ups_product( art v1_06_00 )
find_ups_product( fhiclcpp v2_16_00 )
find_ups_product( messagefacility v1_10_00 )
find_ups_product( cetlib v1_03_00 )
find_ups_product(smc_compiler v6_0_0)
find_ups_xdaq( v11_2_7 )
find_ups_root( v5_34_00 )
find_ups_boost( v1_50_0 filesystem
                        program_options
                        unit_test_framework
 )


##find_package(MPI QUIET)

check_ups_version(xdaq "$ENV{XDAQ_VERSION}" v14_0_0 PRODUCT_OLDER_VAR slf6_xdaq)
if(slf6_xdaq)
	message("Building on SLF6, defining NOCONST")
  add_definitions(-DXDAQ_NOCONST)
endif()

string(REGEX MATCHALL "[0-9][0-9]?[0-9]?" MESSAGEFACILITY_VERSION_LIST "$ENV{MESSAGEFACILITY_VERSION}")
list(GET MESSAGEFACILITY_VERSION_LIST 0 MESSAGEFACILITY_MAJOR_VERSION)
list(GET MESSAGEFACILITY_VERSION_LIST 1 MESSAGEFACILITY_MINOR_VERSION)
list(GET MESSAGEFACILITY_VERSION_LIST 2 MESSAGEFACILITY_PATCH_VERSION)
message("-DMESSAGEFACILITY_HEX_VERSION=0x${MESSAGEFACILITY_MAJOR_VERSION}${MESSAGEFACILITY_MINOR_VERSION}${MESSAGEFACILITY_PATCH_VERSION}")
add_definitions(-DMESSAGEFACILITY_HEX_VERSION=0x${MESSAGEFACILITY_MAJOR_VERSION}${MESSAGEFACILITY_MINOR_VERSION}${MESSAGEFACILITY_PATCH_VERSION})

# macros for art_dictionary and simple_plugin

include(ArtMake)
include(BuildPlugins)
include(CetTest)

# Extra parts for the XDAQ GUI
add_subdirectory(otsdaq-utilities)

# Scripts
add_subdirectory(tools)

# Documentation directory
add_subdirectory(doc)
add_subdirectory(onlineDoc)

# UPS Version files
add_subdirectory(ups)

# WebGUI
FILE(COPY WebGUI DESTINATION .)
INSTALL(DIRECTORY WebGUI DESTINATION ${product}/${version})

# tools
FILE(COPY tools DESTINATION .)
INSTALL(DIRECTORY tools DESTINATION ${product}/${version})

# packaging utility
include(UseCPack)
