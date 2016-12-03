
#get_cmake_property(_variableNames VARIABLES)
#foreach(_variableName ${_variableNames})
#	message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()



include_directories($ENV{XERCES_C_INC})

simple_plugin(SlowControlsOtsInterface "monitor"	
	NetworkUtilities
	WorkLoopManager
    $ENV{XERCES_C_LIB}/libxerces-c.so
	${CETLIB}
  )


#include_directories($ENV{EPICS_BASE}/include)
##include($ENV{EPICS_BASE}/include/alarm.h)
#include_directories($ENV{EPICS_BASE}/include/os/Linux)
#include_directories($ENV{EPICS_BASE}/include/compiler/gcc)
#include_directories($ENV{EPICS_BASE}/include/lib/linux-x86_64/)
#include_directories($ENV{XERCES_C_INC})
#simple_plugin(EpicsInterface "monitor"	
#	 ${EPICS_CA}
#	 ${EPICS_COM}
#	 ${XDAQ_BASIC_LIB_LIST}
#	 ConfigurationInterface
#	 ConfigurationDataFormats
#	 SupervisorDescriptorInfo
#	 CgiDataUtilities
#	 WebUsersUtilities
#	 SOAPUtilities
#	 XmlUtilities
#	 $ENV{XERCES_C_LIB}/libxerces-c.so
#  )

install_headers()
install_source()
