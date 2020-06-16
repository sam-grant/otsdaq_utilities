#!/bin/bash
#
# This script is expected to be in otsdaq utilities repository in a specific directory
# but it can be executed from any path (do not source it, execute with ./ )
#
# ./path/to/script/OnlineDocPushUpdate.sh <do NOT do mrb z> <only transfer main page> <transfer to dev area>
#
#	For example:  ./srcs/otsdaq_utilities/tools/OnlineDocPushUpdate.sh 1 1
#
# Note: people keep commenting out CMakeLists requirements when doxygen causes issues,
#	so remember to have 'add_subdirectory(doc)'  in repo/CMakeLists.txt 
#	and ...				'include(artdaq_doxygen) \n create_doxygen_documentation()' in repo/doc/CMakeLists.txt
#   NOW -- export OTS_DOXY=DOIT  #to enable doxygen doc creation
#

echo 
echo
echo -e "OnlineDoc [${LINENO}]  \t =================="
echo -e "OnlineDoc [${LINENO}]  \t Starting online doc push..."

if ! [ -e setup_ots.sh ]; then
	echo -e "OnlineDoc [${LINENO}]  \t You must run this script from an OTSDAQ installation directory!"
  exit 1
fi

CURRENT_AWESOME_BASE=$PWD
CHECKIN_LOG_PATH=$CURRENT_AWESOME_BASE/.checkinAll.log
UPDATE_LOG_PATH=$CURRENT_AWESOME_BASE/.updateAll.log

echo 
echo
echo -e "OnlineDoc [${LINENO}]  \t Note: Your shell must be bash. If you received 'Expression Syntax' errors, please type 'bash' to switch."
echo -e "OnlineDoc [${LINENO}]  \t You are using $0"
echo
echo


SCRIPT_DIR="$( 
  cd "$(dirname "$(readlink "$0" || printf %s "$0")")"
  pwd -P 
)"
		
echo -e "OnlineDoc [${LINENO}]  \t Script directory found as: $SCRIPT_DIR"


#######################################################################################################################
# regenerate documentation

echo
echo -e "OnlineDoc [${LINENO}]  \t =================="
DO_MRBZ=0
if [ "x$1" == "x" ]; then    
	DO_MRBZ=1
else
	echo -e "OnlineDoc [${LINENO}]  \t Skipping mrb z and regeneration of documentation."
fi
ONLY_MAIN=1
if [ "x$2" == "x" ]; then    
	ONLY_MAIN=0
else
	echo -e "OnlineDoc [${LINENO}]  \t Only regenerating and updating main.html"
fi
SCP_LOC="/docs/dev"
if [ "x$3" == "x" ]; then    
	SCP_LOC="/docs/code"
fi
echo -e "OnlineDoc [${LINENO}]  \t Transferring to location otsdaq.fnal.gov${SCP_LOC}"

if [ $DO_MRBZ == 1 ]; then
	echo -e "OnlineDoc [${LINENO}]  \t Cleaning all so that doxygen will run... mrb z..."
	export OTS_DOXY="DOIT" #enable doxygen in CMakelists
	source setup_ots.sh
	mrb z
	source mrbSetEnv
	mrb b
	unset OTS_DOXY #enable doxygen for future builds
	source mrbSetEnv
fi

#exit #for debugging

#######################################################################################################################
# transfer documentation

echo
echo -e "OnlineDoc [${LINENO}]  \t =================="


if [ $DO_MRBZ == 1 ]; then #should be careful to not delete /artdaq folder.. target only otsdaq*
	echo -e "OnlineDoc [${LINENO}]  \t Deleting current web documentation..."
	ssh web-otsdaq@otsdaq.fnal.gov /web/sites/otsdaq.fnal.gov/data/deleteCodeNav.sh
fi


echo -e "OnlineDoc [${LINENO}]  \t Finding paths and transferring doxygen html..."


REPO_DIR="$(find $SCRIPT_DIR/../../../build_* -maxdepth 1 -iname 'otsdaq*')"


for p in ${REPO_DIR[@]}; do
	if [ -d $p ]; then    
	if [ -d $p/doc/html ]; then
		echo -e "OnlineDoc [${LINENO}]  \t Doc directory found as: $(basename $p)"
	fi
	fi
done
				
for p in ${REPO_DIR[@]}; do
	if [ -d $p ]; then    
	if [ -d $p/doc/html ]; then
		echo -e "OnlineDoc [${LINENO}]  \t Handling directory: $(basename $p)"

		echo -e "OnlineDoc [${LINENO}]  \t Injecting main html..."

		if [ $DO_MRBZ == 1 ]; then #only backup when generated
			#cp $p/doc/html/main.html $p/doc/html/main.html.bk
			cp $p/doc/html/index.html $p/doc/html/index.html.bk
		fi

		echo
		echo -e "OnlineDoc [${LINENO}]  \t =================="
		echo -e "OnlineDoc [${LINENO}]  \t Refining content..."
		echo
		echo
		
		#doxygen_main_editor $p/doc/html/main.html $SCRIPT_DIR/../../../srcs/otsdaq_utilities/onlineDoc/inject_$(basename $p).html $SCRIPT_DIR/../../../srcs/otsdaq_utilities/onlineDoc/inject_otsdaq_head.html
		doxygen_main_editor $p/doc/html/index.html $SCRIPT_DIR/../../../srcs/otsdaq_utilities/onlineDoc/inject_$(basename $p).html $SCRIPT_DIR/../../../srcs/otsdaq_utilities/onlineDoc/inject_otsdaq_head.html

		if [ $ONLY_MAIN == 1 ]; then
			echo -e "OnlineDoc [${LINENO}]  \t scp -r $p/doc/html/index.html web-otsdaq@otsdaq.fnal.gov:/web/sites/otsdaq.fnal.gov/htdocs${SCP_LOC}/$(basename $p)/"
			#scp -r $p/doc/html/main.html web-otsdaq@otsdaq.fnal.gov:/web/sites/otsdaq.fnal.gov/htdocs/${SCP_LOC}/$(basename $p)/
			scp -r $p/doc/html/index.html web-otsdaq@otsdaq.fnal.gov:/web/sites/otsdaq.fnal.gov/htdocs${SCP_LOC}/$(basename $p)/
			continue
		fi

		echo
		echo -e "OnlineDoc [${LINENO}]  \t =================="
		echo -e "OnlineDoc [${LINENO}]  \t Transferring content..."
		echo
		echo
		
		echo -e "OnlineDoc [${LINENO}]  \t scp -r $p/doc/html web-otsdaq@otsdaq.fnal.gov:/web/sites/otsdaq.fnal.gov/htdocs${SCP_LOC}/$(basename $p)"
		scp -r $p/doc/html web-otsdaq@otsdaq.fnal.gov:/web/sites/otsdaq.fnal.gov/htdocs${SCP_LOC}/$(basename $p)
		echo -e "OnlineDoc [${LINENO}]  \t Done with .... scp -r $p/doc/html web-otsdaq@otsdaq.fnal.gov:/web/sites/otsdaq.fnal.gov/htdocs${SCP_LOC}/$(basename $p)"
	fi
    fi	   
done


echo
echo -e "OnlineDoc [${LINENO}]  \t =================="
echo -e "OnlineDoc [${LINENO}]  \t Transferring shared content..."
echo
echo
echo -e "OnlineDoc [${LINENO}]  \t scp -r ${SCRIPT_DIR}/../onlineDoc/otsdaq_doc_library.js web-otsdaq@otsdaq.fnal.gov:/web/sites/otsdaq.fnal.gov/htdocs${SCP_LOC}/"
scp -r ${SCRIPT_DIR}/../onlineDoc/otsdaq_doc_library.js web-otsdaq@otsdaq.fnal.gov:/web/sites/otsdaq.fnal.gov/htdocs${SCP_LOC}/
echo -e "OnlineDoc [${LINENO}]  \t scp -r ${SCRIPT_DIR}/../onlineDoc/contentData web-otsdaq@otsdaq.fnal.gov:/web/sites/otsdaq.fnal.gov/htdocs${SCP_LOC}/"
scp -r ${SCRIPT_DIR}/../onlineDoc/contentData web-otsdaq@otsdaq.fnal.gov:/web/sites/otsdaq.fnal.gov/htdocs${SCP_LOC}/



echo
echo -e "OnlineDoc [${LINENO}]  \t =================="
echo
echo -e "OnlineDoc [${LINENO}]  \t =================="
echo -e "OnlineDoc [${LINENO}]  \t Online documentation update done"
echo -e "OnlineDoc [${LINENO}]  \t *******************************"
echo -e "OnlineDoc [${LINENO}]  \t *******************************"





















