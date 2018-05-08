include_directories($ENV{XERCES_C_INC})


# Look for the header file.
find_path(XSD_INCLUDE_DIR NAMES xsd/cxx/config.hxx)
mark_as_advanced(XSD_INCLUDE_DIR)


if(XSD_INCLUDE_DIR)
  foreach(_XSD_version_header version.hxx)
    if(EXISTS "${XSD_INCLUDE_DIR}/xsd/cxx/${_XSD_version_header}")
      file(STRINGS "${XSD_INCLUDE_DIR}/xsd/cxx/${_XSD_version_header}" XSD_version_str REGEX "^#define[\t ]+XSD_STR_VERSION[\t ]+\".*\"")

      string(REGEX REPLACE "^#define[\t ]+XSD_STR_VERSION[\t ]+\"([^\"]*)\".*" "\\1" XSD_VERSION_STRING "${XSD_version_str}")
      unset(XSD_version_str)
      break()
    endif()
  endforeach()
endif()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(XSD
                                  REQUIRED_VARS XSD_INCLUDE_DIR
                                  VERSION_VAR XSD_VERSION_STRING)

if(XSD_FOUND)
  set(XSD_INCLUDE_DIRS ${XSD_INCLUDE_DIR})


find_package(CURL)

if(CURL_FOUND)
  include_directories(${CURL_INCLUDE_DIRS})

cet_make( LIBRARY_NAME ECLWriter
	LIBRARIES
	artdaq-core_Utilities
	${XDAQ_BASIC_LIB_LIST}
	#XmlUtilities
	CoreSupervisors
	#	 SupervisorInfo
#		 WebUsersUtilities
		 #CgiDataUtilities
		 #SOAPUtilities
		 FiniteStateMachine
		 ConfigurationInterface
		 ConfigurationDataFormats
		 otsdaq-core_ConfigurationPluginDataFormats_XDAQContextConfiguration_configuration
    $ENV{XERCES_C_LIB}/libxerces-c.so
    crypto
    ${CURL_LIBRARIES}
	)

endif(CURL_FOUND)

endif(XSD_FOUND)


install_headers()
install_source()
