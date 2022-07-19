
cet_make_library(LIBRARY_NAME Visualization
SOURCE version.cc VisualSupervisor.cc
		 LIBRARIES PRIVATE
		 otsdaq::CoreSupervisors
		 ROOT::RIO
		 ROOT::Gui
		 otsdaq::RootUtilities
		 Boost::regex
)


install_headers()
install_source()
