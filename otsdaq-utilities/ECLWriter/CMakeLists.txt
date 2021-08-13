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
#endif(XSD_FOUND)


find_package(CURL)

if(CURL_FOUND)
  include_directories(${CURL_INCLUDE_DIRS})
#endif(CURL_FOUND)
cet_make( LIBRARY_NAME ECLWriterCore EXCLUDE ECLSupervisor.cc ECLTest.cc
	LIBRARIES
	artdaq_core::artdaq-core_Utilities
	otsdaq::Macros
        ${XDAQ_XERCES-C}
        pthread
    crypto
    ${CURL_LIBRARIES}
    MF_MessageLogger
	)
cet_make( LIBRARY_NAME ECLWriter EXCLUDE ECLConnection.cpp  ECL.cxx ECLTest.cc
	LIBRARIES
	ECLWriterCore
	artdaq_core::artdaq-core_Utilities
	${XDAQ_BASIC_LIB_LIST}
	#XmlUtilities
	otsdaq::Macros
	otsdaq::CoreSupervisors
		 otsdaq::SupervisorInfo
		 otsdaq::WebUsersUtilities
		 #CgiDataUtilities
		 otsdaq::SOAPUtilities
		 otsdaq::FiniteStateMachine
		 otsdaq::ConfigurationInterface
		 otsdaq::TableCore
		 otsdaq_TablePlugins_XDAQContextTable_table
		 MF_MessageLogger
    crypto
    ${CURL_LIBRARIES}
	)

	cet_make_exec(ECLTest LIBRARIES ECLWriterCore ${Boost_PROGRAM_OPTIONS_LIBRARY})
endif(CURL_FOUND)

endif(XSD_FOUND)


install_headers(EXTRAS ECL.hxx)
install_source(EXTRAS ECL.hxx)
