include_directories($ENV{EPICS_BASE}/include)
#include($ENV{EPICS_BASE}/include/alarm.h)
include_directories($ENV{EPICS_BASE}/include/os/Linux)
include_directories($ENV{EPICS_BASE}/include/compiler/gcc)
include_directories($ENV{EPICS_BASE}/include/lib/linux-x86_64/)
include_directories($ENV{XERCES_C_INC})
cet_make(LIBRARY_NAME SlowControlsDashboard
		 LIBRARIES
		 ${EPICS_CA}
		 ${EPICS_COM}
		 ${XDAQ_BASIC_LIB_LIST}
		 ConfigurationInterface
		 ConfigurationDataFormats
		 SupervisorDescriptorInfo
		 CgiDataUtilities
		 WebUsersUtilities
		 SOAPUtilities
		 XmlUtilities
         $ENV{XERCES_C_LIB}/libxerces-c.so
         MessageFacility
	 ${MF_MESSAGELOGGER}
	 ${MF_UTILITIES}
)

install_headers()
install_source()

