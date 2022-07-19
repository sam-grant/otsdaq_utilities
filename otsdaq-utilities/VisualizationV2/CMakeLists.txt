
cet_make_library(LIBRARY_NAME VisualizationV2
SOURCE
version.cc
VisualDataManagerV2.cc
VisualSupervisorV2.cc
		 LIBRARIES PRIVATE
		 otsdaq::CoreSupervisors
		 ROOT::RIO
		 ROOT::Gui
		 otsdaq::RootUtilities
		 Boost::regex
		 otsdaq::RawDataVisualizerConsumer
)


install_headers()
install_source()
