cet_make(LIBRARY_NAME UtilitiesPluginMakers
		 LIBRARIES
         $ENV{XERCES_C_LIB}/libxerces-c.so
         ${CETLIB}
)

install_headers()
install_source()
